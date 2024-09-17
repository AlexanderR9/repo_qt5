#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lstatic.h"
#include "mbtcpobj.h"
#include "mbtcpcentralwidget.h"

#include <QDebug>
//#include <QSerialPort>
#include <QByteArray>
#include <QSplitter>


MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_centralWidget(NULL)

    //m_comObj(NULL),
    //f_worker(NULL)
{
    setObjectName("main_form_mbtcptest");

    /*
    m_comObj = new ComObj(this);
    connect(m_comObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_comObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

    f_worker = new FileWorker(this);
    connect(f_worker, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(f_worker, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));
    */

}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart); //open port
    addAction(LMainWidget::atStop); //close port
    addAction(LMainWidget::atSendMsg); //write data to port
    addAction(LMainWidget::atClear); //clear protocol
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {startExchange(); break;}
        case LMainWidget::atStop: {stopExchange(); break;}
        case LMainWidget::atSendMsg: {sendReq(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        default: break;
    }
}
void MainForm::initWidgets()
{
//    m_protocol = new LProtocolBox(false, this);
  //  addWidget(m_protocol, 0, 0);

    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    m_protocol = new LProtocolBox(false, this);
    m_centralWidget = new MBTcpCentralWidget(this);
    v_splitter->addWidget(m_centralWidget);
    v_splitter->addWidget(m_protocol);

    connect(m_centralWidget, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_centralWidget, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));

}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    combo_list << "Master" << "Slave";

    QString key = QString("mode");
    lCommonSettings.addParam(QString("Mode"), LSimpleDialog::sdtStringCombo, key);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.last());

    key = QString("port");
    lCommonSettings.addParam(QString("TCP port"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, "1502");

    key = QString("ip");
    lCommonSettings.addParam(QString("IP address"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, "127.0.0.1");

    key = QString("do_count");
    lCommonSettings.addParam(QString("Registers count (DO)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, 0);
    key = QString("di_count");
    lCommonSettings.addParam(QString("Registers count (DI)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, 0);
    key = QString("ao_count");
    lCommonSettings.addParam(QString("Registers count (AO)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, 0);
    key = QString("ai_count");
    lCommonSettings.addParam(QString("Registers count (AI)"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, 0);

}
void MainForm::startExchange()
{

}
void MainForm::stopExchange()
{

}
void MainForm::sendReq()
{

}

/*
void MainForm::save()
{
    LMainWidget::save();
    if (m_comObj->portOpened()) m_comObj->tryClose();
}
void MainForm::load()
{
    LMainWidget::load();

    QStringList keys;
    keys.append("infile");
    slotAppSettingsChanged(keys);

}
MainForm::~MainForm()
{
//    if (m_req) {delete m_req; m_req = NULL;}
}

void MainForm::slotAppSettingsChanged(QStringList keys)
{
    LMainWidget::slotAppSettingsChanged(keys);

    QString key = "infile";
    if (keys.contains(key))
    {
        m_protocol->addSpace();
        f_worker->setInputFile(lCommonSettings.paramValue(key).toString());

        key = "showbuff";
        if (lCommonSettings.paramValue(key).toBool())
        {
            QString s_ba = LStatic::baToStr(f_worker->sendingBuffer());
            m_protocol->addText(s_ba, LProtocolBox::ttData);
        }
    }
}
void MainForm::updatePortParams()
{
    if (m_comObj->portOpened()) return;

    ComParams p;
    p.port_name = lCommonSettings.paramValue("portname").toString().trimmed();
    QString drct = lCommonSettings.paramValue("direction").toString().trimmed();
    if (drct == "Only read") p.direction = QSerialPort::Input;
    if (drct == "Only write") p.direction = QSerialPort::Output;
    m_comObj->setPortParams(p);
}
void MainForm::openPort()
{
    updatePortParams();

    m_protocol->addSpace();
    m_protocol->addText(QString("Try open COM port (%1), direction (%2) .........").arg(m_comObj->portName()).arg(m_comObj->strDirection()), LProtocolBox::ttOk);

    bool ok;
    m_comObj->tryOpen(ok);
    m_protocol->addText(QString("done!"));
}
void MainForm::closePort()
{
    m_protocol->addSpace();
    m_protocol->addText(QString("Try close COM port (%1) .........").arg(m_comObj->portName()), LProtocolBox::ttOk);

    m_comObj->tryClose();
    m_protocol->addText(QString("done!"));
}
void MainForm::writeToPort()
{
    m_protocol->addSpace();
    m_protocol->addText(QString("Try write test data to COM port (%1) .........").arg(m_comObj->portName()), LProtocolBox::ttOk);

    QByteArray ba(f_worker->sendingBuffer());
    tryAddCRCBuff(ba);


    //getTestBA(ba);
    if (ba.isEmpty()) m_protocol->addText(QString("buffer is empty"), LProtocolBox::ttWarning);
    else m_comObj->tryWrite(ba);

    m_protocol->addText(QString("done!"));
}

void MainForm::parseReceivedData(const QByteArray &ba)
{
    QString key = "showbuff";
    if (lCommonSettings.paramValue(key).toBool())
    {
        m_protocol->addSpace();
        QString s_ba = LStatic::baToStr(ba);
        m_protocol->addText(s_ba, LProtocolBox::ttData);
        m_protocol->addSpace();
    }

    key = "savebuff";
    if (lCommonSettings.paramValue(key).toBool())
    {
        f_worker->saveBuffer(ba);
    }
}
*/
void MainForm::slotMessage(const QString &text)
{
    m_protocol->addText(text);
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
/*
void MainForm::getTestBA(QByteArray &ba)
{
    ba.clear();

    char c = 0xaa;
    ba.push_back(c);
    c = 0xbb;
    ba.push_back(c);
    c = 0xdd;
    ba.push_back(c);
}


*/
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());

    m_centralWidget->save(settings);
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    m_centralWidget->load(settings);

}

//private
int MainForm::mode() const
{
    QString s = lCommonSettings.paramValue("mode").toString().trimmed().toLower();
    return ((s == "master") ? 0 : 1);
}
quint16 MainForm::port() const
{
    return lCommonSettings.paramValue("port").toUInt();
}
QString MainForm::host() const
{
    return lCommonSettings.paramValue("ip").toString();
}
quint32 MainForm::do_count() const
{
    return lCommonSettings.paramValue("do_count").toUInt();
}
quint32 MainForm::di_count() const
{
    return lCommonSettings.paramValue("di_count").toUInt();
}
quint32 MainForm::ao_count() const
{
    return lCommonSettings.paramValue("ao_count").toUInt();
}
quint32 MainForm::ai_count() const
{
    return lCommonSettings.paramValue("ai_count").toUInt();
}


