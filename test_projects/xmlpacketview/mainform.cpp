#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "viewwidget.h"
#include "lfile.h"
#include "lstatic.h"


#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QSplitter>
#include <QMessageBox>


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_centralWidget(NULL)
{
    setObjectName("packetview_mainwindow");

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
        //case LMainWidget::atStart: {actStart(); break;}
        case LMainWidget::atLoadData: {reloadPackets(); break;}
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        default: break;
    }
}
void MainForm::initWidgets()
{
    m_centralWidget = new ViewWidget(this);
    addWidget(m_centralWidget, 0, 0);
    connect(this, SIGNAL(signalError(const QString&)), m_centralWidget, SLOT(slotError(const QString&)));
    connect(this, SIGNAL(signalMsg(const QString&)), m_centralWidget, SLOT(slotMessage(const QString&)));

}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("data_dir");
    lCommonSettings.addParam(QString("Packets dir"), LSimpleDialog::sdtDirPath, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("chart_width");
    lCommonSettings.addParam(QString("Chart line width"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "1" << "2" << "3" << "4" << "5";
    lCommonSettings.setComboList(key, combo_list);
}
QString MainForm::packDir() const
{
    return lCommonSettings.paramValue("data_dir").toString();
}
void MainForm::reloadPackets()
{
    qDebug("reload packets");
    emit signalMsg("");
    emit signalMsg(QString("Try reload packets ..........."));
    QString dir_name(packDir().trimmed());
    if (dir_name.isEmpty())
    {
        emit signalError(QString("MainForm: packets dir name is empty"));
        return;
    }
    if (!LFile::dirExists(dir_name))
    {
        emit signalError(QString("MainForm: packets dir [%1] not found").arg(dir_name));
        return;
    }

    QStringList list;
    QString err = LFile::dirFiles(dir_name, list, "xml");
    if (!err.isEmpty())
    {
        emit signalError(QString("MainForm: %1").arg(err));
        return;
    }
    if (list.isEmpty())
    {
        emit signalError(QString("MainForm: packet files list is empty"));
        return;
    }

    m_centralWidget->loadInPack(list.at(0));
    if (list.count() > 1)
        m_centralWidget->loadInPack(list.at(1));

}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    if (m_centralWidget) m_centralWidget->save(settings);
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    if (m_centralWidget) m_centralWidget->load(settings);
}
void MainForm::slotAppSettingsChanged(QStringList keys)
{
    //qDebug("MainForm::slotAppSettingsChanged");
    LMainWidget::slotAppSettingsChanged(keys);

//    if (keys.contains("chart_width") || keys.contains("chart_precision"))
  //      updateChartSettings();


}

