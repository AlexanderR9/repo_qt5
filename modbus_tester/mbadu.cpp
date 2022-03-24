#include "mbadu.h"
#include "comparams_struct.h"
#include "lstatic.h"

#include <QModbusPdu>
#include <QDebug>

MBAdu::MBAdu(const QByteArray &raw_data)
    :m_data(raw_data),
    is_valid(false),
    must_crc(9999),
    m_err(-99)
{
    checkData();
}
qint8 MBAdu::serverAddress() const
{
    if (m_data.isEmpty()) return -1;
    return uchar(m_data.at(0));
}
qint8 MBAdu::cmdCode() const
{
    if (m_data.isEmpty()) return -1;
    return uchar(m_data.at(1));
}
quint16 MBAdu::rawDataCRC() const
{
    int n = rawSize();
    if (n < 4) return 9999;
    return quint16((quint8(m_data.at(n-2)) << 8) | quint8(m_data.at(n-1)));
}
quint16 MBAdu::startPosReg() const
{
    if (rawSize() < 4) return 9998;
    return quint16((quint8(m_data.at(2)) << 8) | quint8(m_data.at(3)));
}
QString MBAdu::rawDataToStr() const
{
    QString s1 = QString("/////////////ADU: Received raw data, size %1 /////////////////").arg(rawSize());
    QString s2;
    if (rawSize() == 0) s2 = "is empty!";
    else
    {
        //s += m_data.toHex();
        s2 = LStatic::baToStr(m_data, 24);
    }
    return QString("%1 \n\r %2").arg(s1).arg(s2);
}
void MBAdu::getPduData(QModbusPdu &pdu)
{
    if (invalid())
    {
        pdu.setData(QByteArray());
        pdu.setFunctionCode(QModbusPdu::Invalid);
    }
    else
    {
        pdu.setFunctionCode(QModbusPdu::FunctionCode(cmdCode()));
        pdu.setData(m_data.mid(2, m_data.size()-4));
    }
}
void MBAdu::checkData()
{
    if (m_data.size() < 4) {m_err = -1; return;}

    //qDebug()<<m_data.toHex();


    must_crc = convertToLittleEndian(MB_CRC16(m_data, rawSize()-2));
    //QByteArray ba(m_data.left(rawSize()-2));
    //must_crc = MB_CRC16(ba);

    //must_crc = MB_CRC16_cs((const unsigned char*)(ba.data()), ba.count());


    if (!checkSumOk()) {m_err = -2; return;}
    if (!reqCodeOk()) {m_err = -3; return;}

    m_err = 0;
    is_valid = true;
}
QString MBAdu::stringErr() const
{
    switch (m_err)
    {
        case -1: return QString("Raw data size(%1) < 4 bytes.").arg(rawSize());
        case -2: return QString("Check sum fault,   raw_data_crc=%1  validity_crc=%2").arg(rawDataCRC()).arg(must_crc);
        case -3: return QString("Request command fault: %1.").arg(cmdCode());
        case -99: return QString("ADU is empty.");
        default: break;
    }
    return QString("no error");
}
bool MBAdu::reqCodeOk() const
{
    qint8 cmd = cmdCode();
    switch (cmd)
    {
        case QModbusPdu::EncapsulatedInterfaceTransport:        //0x2B
        case QModbusPdu::WriteMultipleRegisters:                //0x10
        case QModbusPdu::WriteSingleRegister:                   //0x06
        case QModbusPdu::ReadHoldingRegisters: return true;     //0x03
        default: break;
    }
    return false;
}
quint16 MBAdu::convertToLittleEndian(const quint16 v16)
{
    return ((v16 << 8) | (v16 >> 8));
}
bool MBAdu::checkSumOk() const
{
    //if (invalid()) return false;
    //quint16 must_crc = MB_CRC16_cs(m_data, rawSize()-2);
    return (must_crc == rawDataCRC());
    //return (MB_CRC16_cs(m_data) == 0);
}
quint16 MBAdu::MB_CRC16(const QByteArray &ba, int len)
{
//    qDebug()<<QString("MB_CRC16: len=%1").arg(len);
    const unsigned char *puchMsg = (const unsigned char*)(ba.data());
    quint16 usDataLen = (len < 0 || len >= ba.size()) ? ba.size() : quint16(len);
    //qDebug()<<QString("MB_CRC16: len=%1").arg(usDataLen);

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
/*
quint16 MBAdu::MB_CRC16_cs(const unsigned char * puchMsg, quint16 usDataLen) const
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
*/



