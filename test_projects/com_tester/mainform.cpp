#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lstatic.h"
#include "comobj.h"
#include "fileworker.h"
#include "comparams_struct.h"

#include <QDebug>
#include <QSerialPort>
#include <QByteArray>


MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    m_comObj(NULL),
    f_worker(NULL)
{
    setObjectName("main_form_testcom");

    m_comObj = new ComObj(this);
    connect(m_comObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_comObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

    f_worker = new FileWorker(this);
    connect(f_worker, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(f_worker, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

}
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
    combo_list << "Only read" << "Only write" << "Read/Write";
    
    QString key = QString("direction");
    lCommonSettings.addParam(QString("Direction"), LSimpleDialog::sdtStringCombo, key);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, combo_list.last());
    
    key = QString("portname");
    lCommonSettings.addParam(QString("Port name"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, "/dev/ttyS3");

    key = QString("infile");
    lCommonSettings.addParam(QString("Input file"), LSimpleDialog::sdtFilePath, key);
    lCommonSettings.setDefValue(key, "");

    key = QString("showbuff");
    lCommonSettings.addParam(QString("Show buffers"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, false);

    key = QString("savebuff");
    lCommonSettings.addParam(QString("Save received buffer to file"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, false);

    key = QString("addcrc");
    lCommonSettings.addParam(QString("Add CRC to sending buffer"), LSimpleDialog::sdtBool, key);
    lCommonSettings.setDefValue(key, false);

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
quint16 MainForm::calcCRC (const QByteArray &ba) const
{
    quint32 reg = 0xffff;
    quint32 lsb = 0;

    for (int i=0; i<ba.size(); i++)
    {
        reg ^= ba.at(i);
        for (int j=0; j<8; j++)
        {
            lsb = reg & 0x0001;
            reg = reg >> 1;
            if (lsb) reg ^= 0xa001;
        }
    }
    quint8 hi = reg >> 8;
    quint8 lo = reg;
    quint16 res = 0;
    res = hi;
    res = lsb << 8;
    res += lo;

    return lsb;
}
quint16 MainForm::MB_CRC16_cs(const unsigned char * puchMsg, quint16 usDataLen) const
{
     unsigned char uchCRCHi = 0xFF; // high byte of CRC initialized
     unsigned char uchCRCLo = 0xFF; // low byte of CRC initialized
     unsigned uIndex;               // will index into CRC lookup table

     while (usDataLen--)
     {
         uIndex = uchCRCLo ^ *puchMsg++; // calculate the CRC
         uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
         uchCRCHi = auchCRCLo[uIndex];
     }

     return (uchCRCHi << 8 | uchCRCLo);
}
void MainForm::tryAddCRCBuff(QByteArray &ba)
{
    QString key = "addcrc";
    if (lCommonSettings.paramValue(key).toBool())
    {
        //QByteArray ba1(ba);
        //quint16 crc = qChecksum(ba.data(), ba.count(), Qt::ChecksumItuV41);
        //quint16 crc = calcCheckSum<quint16>(ba.data(), ba.count());
        //quint16 crc = calcCRC(ba);
        quint16 crc = MB_CRC16_cs((const unsigned char*)(ba.data()), ba.count());
        QDataStream stream(&ba, QIODevice::Append);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream << crc;

        //memccpy(crc, )

        //qDebug()<<QString("ba size %1/%2").arg(ba.size()).arg(ba1.size());
    }

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
void MainForm::slotMessage(const QString &text)
{
    m_protocol->addText(text);
    if (text.toLower().contains("received data"))
        parseReceivedData(m_comObj->receivedBuffer());
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


