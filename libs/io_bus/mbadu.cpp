#include "mbadu.h"
#include "mbcrckeytable.h"
//#include "lstatic.h"

#include <QModbusPdu>
#include <QDebug>

#define MIN_RAWDATA_SIZE    4

LMBAdu::LMBAdu(const QByteArray &input_data)
    :m_data(input_data),
    is_valid(false),
    must_crc(9999),
    m_err(-99)
{
    checkData();
}
qint8 LMBAdu::serverAddress() const
{
    if (m_data.isEmpty()) return -1;
    return uchar(m_data.at(0));
}
qint8 LMBAdu::cmdCode() const
{
    if (m_data.isEmpty()) return -1;
    return uchar(m_data.at(1));
}
quint16 LMBAdu::rawDataCRC() const
{
    int n = rawSize();
    if (n < MIN_RAWDATA_SIZE) return 9999;
    return quint16((quint8(m_data.at(n-2)) << 8) | quint8(m_data.at(n-1)));
}
quint16 LMBAdu::startPosReg() const
{
    if (rawSize() < MIN_RAWDATA_SIZE) return 9998;
    return quint16((quint8(m_data.at(2)) << 8) | quint8(m_data.at(3)));
}
QString LMBAdu::rawDataToStr() const
{
    QString s1 = QString("ADU: Received raw data, size %1").arg(rawSize());
    QString s2;
    if (rawSize() == 0) s2 = "data is empty";
    else s2 = m_data.toHex(':');
    return QString("%1    [%2]").arg(s1).arg(s2);
}
void LMBAdu::getPduData(QModbusPdu &pdu) const
{
    if (invalid())
    {
        pdu.setData(QByteArray());
        pdu.setFunctionCode(QModbusPdu::Invalid);
    }
    else
    {
        pdu.setFunctionCode(QModbusPdu::FunctionCode(cmdCode()));
        pdu.setData(m_data.mid(2, m_data.size()-4)); // берем весь массив ADU и отрываем по 2 байта с начала и с конца
    }
}
void LMBAdu::checkData()
{
    m_err = 0;
    if (m_data.size() < MIN_RAWDATA_SIZE) {m_err = -1; return;}

    checkSumOk();
    if (m_err < 0) return;

    if (!reqCodeOk()) {m_err = -3; return;}

    is_valid = true;
}
void LMBAdu::checkSumOk()
{
    must_crc = convertToLittleEndian(MB_CRC16(m_data, rawSize()-2));
    if (must_crc != rawDataCRC())
    {
        QByteArray save_raw_data(m_data);
        int successed_packet_size = -1;
        while (rawSize() > MIN_RAWDATA_SIZE)
        {
            m_data.remove(rawSize()-1, 1);
            must_crc = convertToLittleEndian(MB_CRC16(m_data, rawSize()-2));
            if (must_crc == rawDataCRC())
            {
                successed_packet_size = rawSize();
                break;
            }
        }

        if (successed_packet_size < 0)
        {
            m_err = -2;
            m_data.clear();
            m_data.append(save_raw_data);
        }
    }
}
QString LMBAdu::stringErr() const
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
bool LMBAdu::reqCodeOk() const
{
    qint8 cmd = cmdCode();
    switch (cmd)
    {
        case QModbusPdu::EncapsulatedInterfaceTransport:        //0x2B
            return true;

        //for Holding Registers (16 bit)
        case QModbusPdu::WriteMultipleRegisters:                //0x10
        case QModbusPdu::WriteSingleRegister:                   //0x06
        case QModbusPdu::ReadHoldingRegisters: return true;     //0x03

        //for Coils inputs (1 bit)
        case QModbusPdu::WriteSingleCoil:                       //0x05
            return true;

        //for discrete inputs (1 bit)
        case QModbusPdu::ReadDiscreteInputs:                    //0x02
            return true;

        default: break;
    }
    return false;
}
quint16 LMBAdu::convertToLittleEndian(const quint16 v16)
{
    return ((v16 << 8) | (v16 >> 8));
}
quint16 LMBAdu::MB_CRC16(const QByteArray &ba, int len)
{
    const unsigned char *puchMsg = (const unsigned char*)(ba.data());
    quint16 usDataLen = (len < 0 || len >= ba.size()) ? ba.size() : quint16(len);

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



