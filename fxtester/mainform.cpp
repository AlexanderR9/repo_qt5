#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstatic.h"
#include "fxcentralwidget.h"
#include "fxdataloader.h"
#include "fxdataloaderwidget.h"
#include "fxbarcontainer.h"
#include "fxchartwidget.h"

#include <QDebug>
#include <QDir>
#include <QTest>
#include <QTimer>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QProgressBar>
#include <QGroupBox>
#include <QMessageBox>


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_centralWidget(NULL),
    m_protocol(NULL),
    v_splitter(NULL),
    m_dataLoader(NULL)
{
    setObjectName("fxtester_mainwindow");

    initDataLoader();
}
void MainForm::slotSetLoadedDataByReq(const QList<FXCoupleDataParams> &req, QList<const FXBarContainer*> &list)
{
    list.clear();
    foreach (const FXCoupleDataParams &v, req)
    {
        qDebug() << v.toStr();
        const FXBarContainer *couple_data = m_dataLoader->container(v.couple, v.timeframe);
        if (couple_data) list.append(couple_data);
        else qWarning()<<QString("MainForm::slotSetLoadedDataByReq WARNING: m_dataLoader->container return NULL");
    }
}
void MainForm::initDataLoader()
{
    m_dataLoader = new FXDataLoader(this);
    connect(m_dataLoader, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_dataLoader, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));


}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atLoadData);
    addToolBarSeparator();
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atStart: {actStart(); break;}
        case LMainWidget::atLoadData: {reloadData(); break;}
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        default: break;
    }
}
void MainForm::actStart()
{
    switch (m_centralWidget->currentPageType())
    {
        case FXCentralWidget::fxptQualData:
        {
            m_centralWidget->checkQualData();
            break;
        }
        case FXCentralWidget::fxptTester:
        {
            m_centralWidget->startTesting();
            break;
        }
        default:
        {
            qWarning()<<QString("MainForm::actStart() - WARNING: invalid page type %1").arg(m_centralWidget->currentPageType());
            break;
        }
    }
}
void MainForm::initWidgets()
{
    m_centralWidget = new FXCentralWidget(this);
    m_protocol = new LProtocolBox(false, this);
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    v_splitter->addWidget(m_centralWidget);
    v_splitter->addWidget(m_protocol);

    connect(m_centralWidget, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_centralWidget, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));
    connect(m_centralWidget, SIGNAL(signalGetLoadedDataByReq(const QList<FXCoupleDataParams>&, QList<const FXBarContainer*>&)),
                       this, SLOT(slotSetLoadedDataByReq(const QList<FXCoupleDataParams>&, QList<const FXBarContainer*>&)));

}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("data_dir");
    lCommonSettings.addParam(QString("FX data dir"), LSimpleDialog::sdtDirPath, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("chart_width");
    lCommonSettings.addParam(QString("Chart line width"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "1" << "2" << "3" << "4" << "5";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("chart_precision");
    lCommonSettings.addParam(QString("Chart axis precision"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "0" << "1" << "2" << "3" << "4" << "5";
    lCommonSettings.setComboList(key, combo_list);
}
void MainForm::reloadData()
{
    m_protocol->addSpace();
    m_protocol->addText("Try load data .......", LProtocolBox::ttFile);
    m_protocol->addText(QString("DATA_DIR: [%1]").arg(dataDir()));
    m_dataLoader->setDataDir(dataDir());
    m_dataLoader->reloadData();
    m_protocol->addText("Finished!");

    m_centralWidget->loaderDataUpdate(m_dataLoader);
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMessage(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());

    if (m_centralWidget)
        m_centralWidget->save(settings);

}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    if (m_centralWidget)
        m_centralWidget->load(settings);

    updateChartSettings();
}
QString MainForm::dataDir() const
{
    return lCommonSettings.paramValue("data_dir").toString();
}
void MainForm::updateChartSettings()
{
    quint8 w = lCommonSettings.paramValue("chart_width").toUInt();
    quint8 p = lCommonSettings.paramValue("chart_precision").toUInt();
    FXChartSettings chs;
    chs.line_width = w;
    chs.axis_presision = p;

    m_centralWidget->setChartSettings(chs);
}
void MainForm::slotAppSettingsChanged(QStringList keys)
{
    //qDebug("MainForm::slotAppSettingsChanged");
    LMainWidget::slotAppSettingsChanged(keys);

    if (keys.contains("chart_width") || keys.contains("chart_precision"))
        updateChartSettings();


}

