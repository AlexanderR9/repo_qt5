#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lstatic.h"
#include "modbusobj.h"

#include <QDebug>
#include <QByteArray>

MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    m_modbusObj(NULL)
    //f_worker(NULL)
{
    setObjectName("main_form_testmodbus");

    m_modbusObj = new ModBusObj(this);
    connect(m_modbusObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_modbusObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

}
void MainForm::save()
{
    LMainWidget::save();
    if (m_modbusObj->isConnected()) m_modbusObj->tryDisconnect();
}
void MainForm::load()
{
    LMainWidget::load();

    //QStringList keys;
    //keys.append("infile");
    //slotAppSettingsChanged(keys);
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
void MainForm::initWidgets()
{
    m_protocol = new LProtocolBox(false, this);
    addWidget(m_protocol, 0, 0);
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("devicetype");
    lCommonSettings.addParam(QString("Device type"), LSimpleDialog::sdtStringCombo, key);
    combo_list << "MASTER" << "SLAVE";
    lCommonSettings.setComboList(key, combo_list);
    
    key = QString("portname");
    lCommonSettings.addParam(QString("Port name"), LSimpleDialog::sdtFilePath, key);
    lCommonSettings.setDefValue(key, "/dev/ttyS3");

    key = QString("cmd");
    lCommonSettings.addParam(QString("Packet command"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "0x03" << "0x06" << "0x10";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("address");
    lCommonSettings.addParam(QString("Server device address"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    for (int i=1; i<=30; i++) combo_list.append(QString::number(i));
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, 7);

    key = QString("startreg");
    lCommonSettings.addParam(QString("Start position register"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, 88);

    key = QString("nreg");
    lCommonSettings.addParam(QString("Registers number"), LSimpleDialog::sdtIntCombo, key);
    combo_list.append(QString::number(100));
    combo_list.append(QString::number(500));
    combo_list.append(QString::number(1000));
    combo_list.append(QString::number(1500));
    combo_list.append(QString::number(2000));
    combo_list.append(QString::number(5000));
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, 5);

    key = QString("retries");
    lCommonSettings.addParam(QString("Retries number (master requests)"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    for (int i=0; i<=5; i++) combo_list.append(QString::number(i));
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, 1);


    //key = QString("infile");
    //lCommonSettings.addParam(QString("Input file"), LSimpleDialog::sdtFilePath, key);
    //lCommonSettings.setDefValue(key, "");

    //key = QString("showbuff");
    //lCommonSettings.addParam(QString("Show buffers"), LSimpleDialog::sdtBool, key);
    //lCommonSettings.setDefValue(key, false);

    //key = QString("savebuff");
    //lCommonSettings.addParam(QString("Save received buffer to file"), LSimpleDialog::sdtBool, key);
    //lCommonSettings.setDefValue(key, false);
}
void MainForm::slotAppSettingsChanged(QStringList keys)
{
    LMainWidget::slotAppSettingsChanged(keys);

    ModbusPacketParams p_pack;
    p_pack.setData(m_modbusObj->packetParams());
    bool need_update = false;

    QString key = "address";
    if (keys.contains(key))
    {
        p_pack.address = lCommonSettings.paramValue(key).toInt();
        need_update = true;
    }
    key = "cmd";
    if (keys.contains(key))
    {
        bool ok;
        p_pack.cmd = lCommonSettings.paramValue(key).toString().toInt(&ok, 16);
        need_update = true;
    }
    key = "startreg";
    if (keys.contains(key))
    {
        p_pack.start_pos = lCommonSettings.paramValue(key).toInt();
        need_update = true;
    }
    key = "nreg";
    if (keys.contains(key))
    {
        p_pack.n_regs = lCommonSettings.paramValue(key).toInt();
        need_update = true;
    }
    key = "retries";
    if (keys.contains(key))
    {
        p_pack.retries = lCommonSettings.paramValue(key).toInt();
        need_update = true;
    }

    if (need_update)
        m_modbusObj->setPacketParams(p_pack);


    /*
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
    */

}
void MainForm::updatePortParams()
{
    if (m_modbusObj->isConnected()) return;

    ComParams p_com;
    p_com.port_name = lCommonSettings.paramValue("portname").toString().trimmed();
    QString type = lCommonSettings.paramValue("devicetype").toString().trimmed();
    if (type == "MASTER") p_com.device_type = 0;
    if (type == "SLAVE") p_com.device_type = 1;
    m_modbusObj->setPortParams(p_com);


    bool ok;
    ModbusPacketParams p_pack;
    p_pack.address = lCommonSettings.paramValue("address").toInt();
    p_pack.cmd = lCommonSettings.paramValue("cmd").toString().toInt(&ok, 16);
    //qDebug()<<QString("MainForm::updatePortParams()  cmd=%1/%2").arg(p_pack.cmd).arg(lCommonSettings.paramValue("cmd").toString());
    p_pack.start_pos = lCommonSettings.paramValue("startreg").toInt();
    p_pack.n_regs = lCommonSettings.paramValue("nreg").toInt();
    p_pack.retries = lCommonSettings.paramValue("retries").toInt();
    m_modbusObj->setPacketParams(p_pack);

}
void MainForm::openPort()
{
    updatePortParams();
    m_protocol->addSpace();

    bool ok;
    m_modbusObj->tryConnect(ok);
    m_protocol->addText(QString("done!"));
}
void MainForm::closePort()
{
    m_protocol->addSpace();
    m_modbusObj->tryDisconnect();
    m_protocol->addText(QString("done!"));
}
void MainForm::writeToPort()
{
    m_protocol->addSpace();
    m_modbusObj->sendData();
    m_protocol->addText(QString("done!"));
}
void MainForm::slotMessage(const QString &text)
{
    m_protocol->addText(text);
    //if (text.toLower().contains("received data"))
      //  parseReceivedData(m_comObj->receivedBuffer());
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {openPort(); break;}
        case LMainWidget::atStop: {closePort(); break;}
        case LMainWidget::atSendMsg: {writeToPort(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        default: break;
    }
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
/*
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

