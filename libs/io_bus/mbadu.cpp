#include "mbadu.h"
#include "mbcrckeytable.h"
#include <QModbusPdu>
#include <QDebug>

#define MIN_RAWDATA_SIZE_RTU    4
#define MIN_RAWDATA_SIZE_TCP    8


//////////////////// LMBAduBase ////////////////////////////
LMBAduBase::LMBAduBase(const QByteArray &input_data)
    :m_data(input_data),
    is_valid(false),
    m_err(-99)
{


}
quint16 LMBAduBase::convertToLittleEndian(const quint16 v16)
{
    return ((v16 << 8) | (v16 >> 8));
}
bool LMBAduBase::functionCodeOk(qint8 cmd)
{
    switch (cmd)
    {
        case QModbusPdu::EncapsulatedInterfaceTransport:        //0x2B
            return true;

        //for Holding Registers (16 bit)
        case QModbusPdu::WriteMultipleRegisters:                //0x10
        case QModbusPdu::WriteSingleRegister:                   //0x06
        case QModbusPdu::ReadHoldingRegisters: return true;     //0x03

        //for Coils inputs (1 bit)
        case QModbusPdu::ReadCoils:                             //0x01
        case QModbusPdu::WriteMultipleCoils:                    //0x0F
        case QModbusPdu::WriteSingleCoil:                       //0x05
            return true;

        //for discrete inputs (1 bit)
        case QModbusPdu::ReadDiscreteInputs:                    //0x02
            return true;

        default: break;
    }
    return false;
}
QString LMBAduBase::stringErr() const
{
    switch (m_err)
    {
        case -1: return QString("Raw data size(%1) < 4 bytes.").arg(rawSize());
        case -3: return QString("Request command fault: %1.").arg(cmdCode());
        case -99: return QString("ADU is empty.");
        default: break;
    }
    return QString("no error");
}
void LMBAduBase::getPduData(QModbusPdu &pdu) const
{
    if (invalid())
    {
        pdu.setData(QByteArray());
        pdu.setFunctionCode(QModbusPdu::Invalid);
    }
}
QString LMBAduBase::rawDataToStr() const
{
    QString s1 = QString("ADU: Received raw data, size %1").arg(rawSize());
    QString s2;
    if (rawSize() == 0) s2 = "data is empty";
    else s2 = m_data.toHex(':');
    return QString("%1    [%2]").arg(s1).arg(s2);
}



//////////////////// LMBAdu ////////////////////////////
LMBAdu::LMBAdu(const QByteArray &input_data)
    :LMBAduBase(input_data),
    must_crc(9999)
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
    if (n < MIN_RAWDATA_SIZE_RTU) return 9999;
    return quint16((quint8(m_data.at(n-2)) << 8) | quint8(m_data.at(n-1)));
}
quint16 LMBAdu::startPosReg() const
{
    if (rawSize() < MIN_RAWDATA_SIZE_RTU) return 9998;
    return quint16((quint8(m_data.at(2)) << 8) | quint8(m_data.at(3)));
}
void LMBAdu::getPduData(QModbusPdu &pdu) const
{
    if (!invalid())
    {
        pdu.setFunctionCode(QModbusPdu::FunctionCode(cmdCode()));
        pdu.setData(m_data.mid(2, m_data.size()-4)); // берем весь массив ADU и отрываем по 2 байта с начала и с конца
    }
    else LMBAduBase::getPduData(pdu);
}
void LMBAdu::checkData()
{
    m_err = 0;
    if (m_data.size() < MIN_RAWDATA_SIZE_RTU) {m_err = -1; return;}

    checkSumOk();
    if (m_err < 0) return;
    if (!LMBAduBase::functionCodeOk(cmdCode())) {m_err = -3; return;}
    is_valid = true;
}
void LMBAdu::checkSumOk()
{
    must_crc = convertToLittleEndian(MB_CRC16(m_data, rawSize()-2));
    if (must_crc != rawDataCRC())
    {
        QByteArray save_raw_data(m_data);
        int successed_packet_size = -1;
        while (rawSize() > MIN_RAWDATA_SIZE_RTU)
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
QString LMBAdu::stringErr() const
{
    if (m_err == -2) return QString("Check sum fault,   raw_data_crc=%1  validity_crc=%2").arg(rawDataCRC()).arg(must_crc);
    return LMBAduBase::stringErr();
}



//////////////////// LMBTcpAdu ////////////////////////////
LMBTcpAdu::LMBTcpAdu(const QByteArray &input_data)
    :LMBAduBase(input_data)
{
    checkData();
}
void LMBTcpAdu::checkData()
{
    m_err = 0;
    if (m_data.size() < MIN_RAWDATA_SIZE_TCP) {m_err = -1; return;}
    if (!LMBAduBase::functionCodeOk(cmdCode())) {m_err = -3; return;}

    qint16 len = packetLen();
    if (len <= 0) {m_err = -4; return;}
    if (len != (rawSize() - 6)) {m_err = -5; return;}

    is_valid = true;
}
qint8 LMBTcpAdu::serverAddress() const
{
    if (m_data.isEmpty() || invalid()) return -1;
    return uchar(m_data.at(6));
}
qint8 LMBTcpAdu::cmdCode() const
{
    if (m_data.isEmpty() || invalid()) return -1;
    return uchar(m_data.at(7));
}
qint16 LMBTcpAdu::packetLen() const
{
    if (m_data.isEmpty() || invalid()) return -1;
    return quint16((quint8(m_data.at(4)) << 8) | quint8(m_data.at(5)));
}
QString LMBTcpAdu::stringErr() const
{
    if (m_err == -4 || m_err == -5) return QString("Incorrect MBTCP header len %1, raw size %2").arg(packetLen()).arg(rawSize());
    return LMBAduBase::stringErr();
}
void LMBTcpAdu::getPduData(QModbusPdu &pdu) const
{
    if (!invalid())
    {
        pdu.setFunctionCode(QModbusPdu::FunctionCode(cmdCode()));
        pdu.setData(m_data.right(rawSize()-7)); // берем весь массив ADU и отрываем по 7 байт с начала(заголовок mbtcp)
    }
    else LMBAduBase::getPduData(pdu);
}


