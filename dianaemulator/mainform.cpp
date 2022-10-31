#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstatic.h"
#include "lfile.h"
#include "dianaviewwidget.h"
#include "mqgeneralpage.h"
#include "mqworker.h"


#include <QDebug>
#include <QDir>
#include <QSplitter>
#include <QSettings>
#include <QApplication>
#include <QTimer>
#include <QProgressBar>
#include <QGroupBox>
#include <QTabWidget>
#include <QMessageBox>
#include <QSysInfo>



// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
      m_mode(emClient),
      v_splitter(NULL),
      m_tab(NULL),
      m_protocol(NULL),
      m_exchangeTimer(NULL),
      m_stateTimer(NULL),
      m_generalPage(NULL)
{
    setObjectName("main_form_dianaemulator");

}
void MainForm::initTimers()
{
    m_exchangeTimer = new QTimer(this);
    m_exchangeTimer->stop();
    m_stateTimer = new QTimer(this);
    m_stateTimer->stop();

    connect(m_exchangeTimer, SIGNAL(timeout()), this, SLOT(slotMQExchangeTimer()));
    connect(m_stateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateMQStateTimer()));
}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atStop);
    //addAction(LMainWidget::atSendMsg);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {start(); break;}
        case LMainWidget::atStop: {stop(); break;}
        //case LMainWidget::atSendMsg: {sendPack(); break;}
        default: break;
    }
}
void MainForm::initWidgets()
{
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    m_protocol = new LProtocolBox(false, this);
    m_tab = new QTabWidget(this);
    m_generalPage = new MQGeneralPage(this);
    m_tab->addTab(m_generalPage, "General");
    //m_tab->clear();

    v_splitter->addWidget(m_tab);
    v_splitter->addWidget(m_protocol);


    initTimers();
    updateButtonsState();

    slotMsg("************ APP STARTED! ***************");
    slotMsg(LStatic::systemInfo());
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=0; i<=5; i++)
        combo_list.append(QString::number(i));

    QString key = QString("config_dir");
    lCommonSettings.addParam(QString("Packets folder"), LSimpleDialog::sdtDirPath, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("mode");
    lCommonSettings.addParam(QString("Emulator mode"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "Server" << "Client";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("byteorder");
    lCommonSettings.addParam(QString("DataStream bytes order"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "BigEndian" << "LitleEndian";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, QString("LitleEndian"));

    key = QString("precision");
    lCommonSettings.addParam(QString("View double values precision"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "1" << "2" << "3" << "4" << "5" << "6";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("view_expand");
    lCommonSettings.addParam(QString("Expand packets level"), LSimpleDialog::sdtIntCombo, key);
    lCommonSettings.setComboList(key, combo_list);


    //timers settings
    key = QString("mq_state_interval");
    lCommonSettings.addParam(QString("Update queues state timer interval, ms"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "100" << "200" << "300" << "500" << "1000" << "2000" << "5000";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.at(3));

    key = QString("mq_exchange_interval");
    lCommonSettings.addParam(QString("Exchange queues timer interval, ms"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "500" << "1000" << "2000" << "5000" << "10000";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.at(1));


    key = QString("auto_update_pack_values");
    lCommonSettings.addParam(QString("Auto update values of sending packets"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, true);

}
void MainForm::start()
{
     restartExchangeTimer();
     updateButtonsState();

     m_protocol->addSpace();
     m_protocol->addText(QString("Data exchange started, interval %1 ms.").arg(mqExchangeInterval()));
}
void MainForm::stop()
{
    m_exchangeTimer->stop();
    updateButtonsState();
    m_protocol->addSpace();
    m_protocol->addText(QString("Data exchange stopped."));
}
void MainForm::slotUpdateMQStateTimer()
{
    foreach (DianaViewWidget *page, m_pages)
        page->updateMQState();

    m_generalPage->updateMQState();
}
void MainForm::slotMQExchangeTimer()
{
    foreach (DianaViewWidget *page, m_pages)
        page->sendMsgToQueue();

}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    if (v_splitter)
        settings.setValue(QString("%1/v_spltitter_state").arg(objectName()), v_splitter->saveState());

    settings.setValue(QString("%1/tab_index").arg(objectName()), m_tab->currentIndex());

    for (int i=0; i<m_tab->count(); i++)
    {
        LSimpleWidget *sw = qobject_cast<LSimpleWidget*>(m_tab->widget(i));
        if (sw) sw->save(settings);
    }
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba = settings.value(QString("%1/v_spltitter_state").arg(objectName()), QByteArray()).toByteArray();
    if (v_splitter && !ba.isEmpty()) v_splitter->restoreState(ba);

    bool ok;
    checkMQLinuxDir(ok);
    if (!ok) return;

    m_mode = modeSettings();
    loadMQConfig();

    for (int i=0; i<m_tab->count(); i++)
    {
        LSimpleWidget *sw = qobject_cast<LSimpleWidget*>(m_tab->widget(i));
        if (sw) sw->load(settings);
    }

    m_tab->setCurrentIndex(settings.value(QString("%1/tab_index").arg(objectName()), 0).toInt());
    restartStateTimer();

    foreach (DianaViewWidget *page, m_pages)
        page->setAutoRecalcPackValues(autoUpdatePackets());

}
void MainForm::updateButtonsState()
{
    bool started = m_exchangeTimer->isActive();
    getAction(atStart)->setEnabled(!started);
    getAction(atStop)->setEnabled(started);
    //getAction(atSendMsg)->setEnabled(started);
    getAction(atSettings)->setEnabled(!started);
    getAction(atExit)->setEnabled(!started);

}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}
void MainForm::slotAppSettingsChanged(QStringList keys)
{
    LMainWidget::slotAppSettingsChanged(keys);

    foreach (QString key, keys)
    {
        if (key == "mode" || key == "config_dir")
        {
            stop();
            QMessageBox::information(this, "Need restart!", "Main app settings chainged, \n you must restart program.");
            break;
        }
        else if (key == "precision")
        {
            foreach (DianaViewWidget *page, m_pages)
                page->setDoublePrecision(doublePrecision());
        }
        else if (key == "auto_update_pack_values")
        {
            foreach (DianaViewWidget *page, m_pages)
                page->setAutoRecalcPackValues(autoUpdatePackets());
        }
        else if (key == "mq_state_interval") restartStateTimer();
        else if (key == "mq_exchange_interval" && m_exchangeTimer->isActive()) restartExchangeTimer();


    }
}
void MainForm::checkMQLinuxDir(bool &ok)
{

    QDir dir(MQWorker::mqLinuxDir());
    if (!dir.exists())
    {
        ok = false;
        slotError(QString("You must mount mqueue dir: %1").arg(dir.path()));
        slotMsg("Run next commands by root:");
        slotMsg(QString("1. [mkdir %1]").arg(dir.path()));
        slotMsg(QString("2. [mount -t mqueue none %1]").arg(dir.path()));
        m_protocol->addSpace();
        slotMsg("And restart program!!!");
    }
    else ok = true;
}
void MainForm::loadMQConfig()
{
    m_protocol->addSpace();
    m_protocol->addText("Try loading DIANA structs ........", LProtocolBox::ttData);

    QString dir_name(configDir().trimmed());
    if (dir_name.isEmpty())
    {
        slotError(QString("packets folder name is empty"));
        return;
    }
    if (!LFile::dirExists(dir_name))
    {
        slotError(QString("packets folder [%1] not found").arg(dir_name));
        return;
    }

    slotMsg(QString("CONFIG_DIR = [%1]").arg(dir_name));
    QStringList list;
    QString err = LFile::dirFiles(dir_name, list, "xml");
    if (!err.isEmpty())
    {
        slotError(QString("%1").arg(err));
        return;
    }
    if (list.isEmpty())
    {
        slotError(QString("packet files list is empty"));
        return;
    }

    //try load packets for DIANA
    QPair<QString, QString> pair;
    for (int i=0; i<list.count(); i++)
    {
        parseConfigName(list.at(i), pair);
        if (pair.first.isEmpty()) continue;

        slotMsg(QString("loading pack - %1 : MODULE=[%2]  TYPE=[%3]").arg(list.at(i)).arg(pair.first).arg(pair.second));
        tryAddPage(pair.first);
        loadMQPacket(list.at(i), pair.first);
    }
}
void MainForm::loadMQPacket(const QString &fname, const QString &diana_name)
{
    //qDebug()<<QString("loadMQPacket: %1").arg(fname);
    DianaViewWidget *page = m_pages.value(diana_name);
    if (page)
    {
        page->loadMQPacket(fname);
        page->setExpandLevel(viewExpandLevel());
        page->setDoublePrecision(doublePrecision());
    }
}
void MainForm::tryAddPage(const QString &diana_name)
{
    if (m_pages.contains(diana_name))
    {
        //qWarning()<<QString("Diana page (%1) allready exist");
        return;
    }

    DianaViewWidget *dw = new DianaViewWidget(diana_name, this);
    m_pages.insert(diana_name, dw);
    m_tab->addTab(dw, diana_name);

    connect(dw, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(dw, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));
    connect(dw, SIGNAL(signalMQCreated(const QString&, quint32, const MQ*)), m_generalPage, SLOT(slotAppendMQ(const QString&, quint32, const MQ*)));
    connect(dw, SIGNAL(signalSendMsgOk(const QString&)), m_generalPage, SLOT(slotSendMsgOk(const QString&)));
    connect(dw, SIGNAL(signalReceiveMsgOk(const QString&)), m_generalPage, SLOT(slotReceiveMsgOk(const QString&)));
    connect(dw, SIGNAL(signalSendMsgErr(const QString&)), m_generalPage, SLOT(slotSendMsgErr(const QString&)));
    connect(dw, SIGNAL(signalReceiveMsgErr(const QString&)), m_generalPage, SLOT(slotReceiveMsgErr(const QString&)));

}
void MainForm::restartStateTimer()
{
    m_stateTimer->stop();
    m_stateTimer->setInterval(mqStateInterval());
    m_stateTimer->start();
}
void MainForm::restartExchangeTimer()
{
    m_exchangeTimer->stop();
    m_exchangeTimer->setInterval(mqExchangeInterval());
    m_exchangeTimer->start();
}



//private funcs
QString MainForm::configDir() const
{
    return lCommonSettings.paramValue("config_dir").toString();
}
quint8 MainForm::doublePrecision() const
{
    return lCommonSettings.paramValue("precision").toUInt();
}
int MainForm::mqStateInterval()
{
    return lCommonSettings.paramValue("mq_state_interval").toInt();
}
int MainForm::mqExchangeInterval()
{
    return lCommonSettings.paramValue("mq_exchange_interval").toInt();
}
int MainForm::byteOrder() const
{
    if (lCommonSettings.paramValue("byteorder").toString().toLower().contains("big")) return QDataStream::BigEndian;
    return QDataStream::LittleEndian;
}
bool MainForm::autoUpdatePackets() const
{
    return lCommonSettings.paramValue("auto_update_pack_values").toBool();
}
int MainForm::viewExpandLevel() const
{
    return lCommonSettings.paramValue("view_expand").toInt();
}
int MainForm::modeSettings() const
{
    QString value = lCommonSettings.paramValue("mode").toString().trimmed();
    return ((value.toLower() == "client") ? emClient : emServer);
}
void MainForm::parseConfigName(const QString &fname, QPair<QString, QString> &pair)
{
    pair.first = QString();
    pair.second = QString();
    QString sname = LFile::shortFileName(fname);

    int pos = -1;
    if (sname.contains("_input.xml"))
    {
        pos = sname.indexOf("_input.xml");
        if (pos > 0)
        {
            pair.first = sname.left(pos).trimmed();
            if (!pair.first.isEmpty()) pair.second = "input";
        }
    }
    else if (sname.contains("_output.xml"))
    {
        pos = sname.indexOf("_output.xml");
        if (pos > 0)
        {
            pair.first = sname.left(pos).trimmed();
            if (!pair.first.isEmpty()) pair.second = "output";
        }
    }
}


