#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstring.h"
#include "lstatic.h"
#include "lsimplewidget.h"
#include "lsearch.h"
#include "ltable.h"
#include "bb_centralwidget.h"
#include "apiconfig.h"
#include "bb_apistruct.h"

#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QProgressBar>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QTableWidget>
#include <QClipboard>
#include <QGuiApplication>



// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_centralWidget(NULL)
{
    setObjectName("bybit_mainwindow");
    api_config.loadTickers();
}
void MainForm::initActions()
{
    addAction(LMainWidget::atRefresh);
    //addAction(LMainWidget::atLoadData);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atAdd);
    addAction(LMainWidget::atRemove);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atLoadData);
    addAction(LMainWidget::atChart);

    addToolBarSeparator();
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);

    //this->setActionTooltip(atRedo, "remove carriage return symbols and rewrite file1");
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atRefresh: {actStart(); break;}
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        case LMainWidget::atAdd: {m_centralWidget->actAdd(); break;}
        case LMainWidget::atRemove: {m_centralWidget->actRemove(); break;}
        case LMainWidget::atStop: {m_centralWidget->actStop(); break;}
        case LMainWidget::atLoadData: {m_centralWidget->actLoadData(); break;}
        case LMainWidget::atChart: {m_centralWidget->actChart(); break;}

        default: break;
    }
}
void MainForm::slotSetShadowLimitSize(float &a)
{
    a = shadowSize();
}
void MainForm::slotSetStopLoss(float &a)
{
    a = stopLoss();
}
void MainForm::slotActionsApdate(int p_type)
{
    switch(p_type)
    {
        case rtCandles:
        {
            getAction(atAdd)->setVisible(true);
            getAction(atRemove)->setVisible(true);
            getAction(atStop)->setVisible(false);
            getAction(atLoadData)->setVisible(false);
            getAction(atChart)->setVisible(false);
            break;
        }
        case rtShadows:
        {
            getAction(atStop)->setVisible(true);
            getAction(atAdd)->setVisible(false);
            getAction(atRemove)->setVisible(false);
            getAction(atLoadData)->setVisible(true);
            getAction(atChart)->setVisible(true);
            break;
        }
        case rtPositions:
        case rtOrders:
        case rtHistory:
        case rtSpotHistory:
        case rtBag:
        case rtFundRate:
        case rtJsonView:
        case rtPrices:
        {
            getAction(atAdd)->setVisible(false);
            getAction(atRemove)->setVisible(false);
            getAction(atStop)->setVisible(false);
            getAction(atLoadData)->setVisible(false);
            getAction(atChart)->setVisible(false);
            break;
        }
        default: break;
    }
}
void MainForm::actStart()
{
    m_protocol->addSpace();
    m_protocol->addText("Run updatating data of page .......", LProtocolBox::ttFile);

    if (m_centralWidget) m_centralWidget->updateDataPage();
}
void MainForm::initWidgets()
{
    qDebug("MainForm::initWidgets()");
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    m_protocol = new LProtocolBox(false, this);
    m_centralWidget = new BB_CentralWidget(this);
    v_splitter->addWidget(m_centralWidget);
    v_splitter->addWidget(m_protocol);

    connect(m_centralWidget, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_centralWidget, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
    connect(m_centralWidget, SIGNAL(signalPageChanged(int)), this, SLOT(slotActionsApdate(int)));
    connect(m_centralWidget, SIGNAL(signalGetShadowLimitSize(float&)), this, SLOT(slotSetShadowLimitSize(float&)));
    connect(m_centralWidget, SIGNAL(signalGetStopLoss(float&)), this, SLOT(slotSetStopLoss(float&)));

}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("candle_size");
    lCommonSettings.addParam(QString("Candle size"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "1" << "5" << "15" << "60" << "240" << "720" << "D" << "W" << "M";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("candle_count");
    lCommonSettings.addParam(QString("Candles count"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "25" << "50" << "100" << "200" << "300" << "500";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("apikey");
    lCommonSettings.addParam(QString("API key"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("apikey2");
    lCommonSettings.addParam(QString("API key (private)"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("req_delay");
    lCommonSettings.addParam(QString("Request delay"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "500" << "1000" << "2000" << "3000" << "5000" << "10000";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("view_expand");
    lCommonSettings.addParam(QString("View expand level"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "1" << "2" << "3" << "4" << "5";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("limit_pos");
    lCommonSettings.addParam(QString("Limit request positions"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "25" << "50" << "100" << "200" << "300" << "500";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("page_updating_interval");
    lCommonSettings.addParam(QString("Min page updating interval"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "15" << "30" << "60" << "120" << "300" << "600" << "1800";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.at(2));

    key = QString("shadow_size");
    lCommonSettings.addParam(QString("Limit shadow size, %"), LSimpleDialog::sdtDoubleCombo, key, 1);
    combo_list.clear();
    for (int i=0; i<20; i++) combo_list << QString::number(float(i+1)*0.2, 'f', 1);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.at(5));

    key = QString("stop_loss");
    lCommonSettings.addParam(QString("Stoploss size, %"), LSimpleDialog::sdtDoubleCombo, key, 1);
    combo_list.clear();
    combo_list << QString::number(-1);
    for (int i=0; i<30; i++) combo_list << QString::number(2+float(i+1)*0.5, 'f', 1);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.at(10));

}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());

    m_centralWidget->save(settings);
    api_config.saveFavorTickers();
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    m_centralWidget->load(settings);
    m_centralWidget->setExpandLevel(expandLevel());
    m_centralWidget->setUpdatingInterval(pageUpdatingInterval());

    //restore apiconfig
    QString apikey1(lCommonSettings.paramValue("apikey").toString());
    QString apikey2(lCommonSettings.paramValue("apikey2").toString());
    api_config.setApiKeys(apikey1, apikey2);
    api_config.req_delay = lCommonSettings.paramValue("req_delay").toInt();
    api_config.candle.size = lCommonSettings.paramValue("candle_size").toString();
    api_config.candle.number = lCommonSettings.paramValue("candle_count").toInt();
}
void MainForm::slotAppSettingsChanged(QStringList list)
{
    LMainWidget::slotAppSettingsChanged(list);
    if (list.contains("candle_size")) api_config.candle.size = lCommonSettings.paramValue("candle_size").toString();
    if (list.contains("candle_count")) api_config.candle.number = lCommonSettings.paramValue("candle_count").toInt();
    if (list.contains("req_delay")) api_config.req_delay = lCommonSettings.paramValue("req_delay").toInt();
    if (list.contains("view_expand")) m_centralWidget->setExpandLevel(expandLevel());
    if (list.contains("limit_pos")) api_config.req_limit_pos = lCommonSettings.paramValue("limit_pos").toInt();
    if (list.contains("page_updating_interval")) m_centralWidget->setUpdatingInterval(pageUpdatingInterval());
}

//private
int MainForm::expandLevel() const
{
    return lCommonSettings.paramValue("view_expand").toInt();
}
quint16 MainForm::pageUpdatingInterval() const
{
    return lCommonSettings.paramValue("page_updating_interval").toInt();
}
float MainForm::shadowSize() const
{
    return lCommonSettings.paramValue("shadow_size").toFloat();
}
float MainForm::stopLoss() const
{
    return lCommonSettings.paramValue("stop_loss").toFloat();
}




