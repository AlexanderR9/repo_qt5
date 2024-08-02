#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "paramswidget.h"
//#include "tickobj.h"
#include "calc_v3.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QApplication>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QGroupBox>
#include <QListWidget>
#include <QTreeWidget>
#include <QColor>



// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
      m_protocol(NULL),
    m_paramsWidget(NULL),
    v_splitter(NULL),
    m_calcObj(NULL)
{
    setObjectName("main_form_v3");
    qDebug("MainForm::MainForm 1");

    m_calcObj = new LPoolCalcObj(this);
    connect(m_calcObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_calcObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));

    qDebug("MainForm::MainForm");
}

void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=0; i<=10; i++)
        combo_list.append(QString::number(i));
    combo_list.append(QString::number(-1));

    QString key = QString("server_api");
    lCommonSettings.addParam(QString("Server API"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString("invest-public-api.tinkoff.ru"));
}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atLoadData);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atRefresh);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);

    this->getAction(atClear)->setToolTip("Clear protocol");
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {actStart(); break;}
        //case LMainWidget::atLoadData: {loadData(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
//        case LMainWidget::atRefresh: {updateOrdersList(); break;}
        default: break;
    }
}
void MainForm::actStart()
{
    InputPoolParams pp;
    m_paramsWidget->getParams(pp);
    m_paramsWidget->resetCalcParams();
    qDebug()<<pp.toStr();
    if (!pp.validity) return;

    m_calcObj->setPoolTokens("OP", "USDC", pp.fee_type);
    m_calcObj->setPricesRange(pp.range.first, pp.range.second);
    m_calcObj->setCurPrice(pp.cur_price);
    m_calcObj->setTokenSize(pp.input_size, pp.input_token);

    m_calcObj->recalc();
    m_paramsWidget->updateParams(m_calcObj);
}
void MainForm::initWidgets()
{
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    m_paramsWidget = new ParamsWidget(this);
    m_protocol = new LProtocolBox(false, this);
    v_splitter->addWidget(m_paramsWidget);
    v_splitter->addWidget(m_protocol);

    connect(m_paramsWidget, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_paramsWidget, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
    //connect(m_tickObj, SIGNAL(signalSendCalcResult(const PoolParamsCalculated&)), m_paramsWidget, SLOT(slotCalcResult(const PoolParamsCalculated&)));
    //connect(m_paramsWidget, SIGNAL(signalPriceChanged(float)), m_tickObj, SLOT(slotPriceChanged(float)));
    //connect(m_tickObj, SIGNAL(signalChangePriceResult(float, float)), m_paramsWidget, SLOT(slotChangePriceResult(float, float)));

}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}

void MainForm::slotAppSettingsChanged(QStringList list)
{
    LMainWidget::slotAppSettingsChanged(list);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());

    if (m_paramsWidget) m_paramsWidget->save(settings);
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    if (m_paramsWidget) m_paramsWidget->load(settings);
}


