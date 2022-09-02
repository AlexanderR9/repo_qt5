#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstatic.h"
#include "fxcentralwidget.h"
#include "fxdataloader.h"

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
        case LMainWidget::atStart: {break;}
        case LMainWidget::atLoadData: {reloadData(); break;}
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        default: break;
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
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("data_dir");
    lCommonSettings.addParam(QString("FX data dir"), LSimpleDialog::sdtDirPath, key);
    lCommonSettings.setDefValue(key, QString(""));



    /*
    QString key = QString("source");
    lCommonSettings.addParam(QString("Source dir"), LSimpleDialog::sdtDirPath, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("speed");
    lCommonSettings.addParam(QString("Write speed"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "none" << "4x" << "8x" << "12x" << "16x" << "24x" << "36x" << "48x";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("cd_dev");
    lCommonSettings.addParam(QString("CDRW device"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString("/dev/cdrom"));
*/

}
void MainForm::reloadData()
{
    m_protocol->addSpace();
    m_protocol->addText("Try load data .......", LProtocolBox::ttFile);
    m_protocol->addText(QString("DATA_DIR: [%1]").arg(dataDir()));
    m_dataLoader->setDataDir(dataDir());
    m_dataLoader->reloadData();
    m_protocol->addText("Finished!");

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
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);
}
QString MainForm::dataDir() const
{
    return lCommonSettings.paramValue("data_dir").toString();
}


/*
void MainForm::updateActionsEnable(bool stoped)
{
    getAction(LMainWidget::atStop)->setEnabled(!stoped);
    getAction(LMainWidget::atISO)->setEnabled(stoped);
    getAction(LMainWidget::atBurn)->setEnabled(stoped);
    getAction(LMainWidget::atEject)->setEnabled(stoped);
    getAction(LMainWidget::atRemove)->setEnabled(stoped);
    getAction(LMainWidget::atCDErase)->setEnabled(stoped);
    //getAction(LMainWidget::atCalcCRC)->setEnabled(false);
    getAction(LMainWidget::atCalcCRC)->setEnabled(stoped);
    getAction(LMainWidget::atSettings)->setEnabled(stoped);
    getAction(LMainWidget::atExit)->setEnabled(stoped);
}




void MainForm::slotAppSettingsChanged(QStringList keys)
{
    qDebug("MainForm::slotAppSettingsChanged");
    LMainWidget::slotAppSettingsChanged(keys);

    if (keys.contains("source"))
    {
        m_protocol->addSpace();
        m_protocol->addText(QString("Changed source dir: %1").arg(sourcePath()));
        m_protocol->addText(QString("reloading ISO file list ...."));
        m_paramsPage->reloadISOList(sourcePath());
        m_protocol->addText(QString("done!"));
    }
}
*/

