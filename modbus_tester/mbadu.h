#ifndef MBADU_H
#define MBADU_H


#include <QByteArray>


class QModbusPdu;

//MBAdu
class MBAdu
{
public:
    MBAdu(const QByteArray &raw_data);

    qint8 serverAddress() const;
    qint8 cmdCode() const;
    quint16 rawDataCRC() const;
    quint16 startPosReg() const;
    void getPduData(QModbusPdu &pdu);

    inline bool invalid() const {return !is_valid;}
    inline int errCode() const {return m_err;}
    inline int rawSize() const {return m_data.size();}
    QString stringErr() const;
    QString rawDataToStr() const;
    inline const QByteArray& rawData() const {return m_data;}
    inline bool readingCmd() const {return (cmdCode() == quint8(0x03));}
    inline bool writingCmd() const {return (cmdCode() == quint8(0x10));}

    static quint16 MB_CRC16(const QByteArray&, int len = -1); // если len < 0, то значит использовать весь BA
    static quint16 convertToLittleEndian(const quint16); //развернуть порядок байт в значении


protected:
    QByteArray m_data;
    bool is_valid;
    quint16 must_crc;

    // -1 размер сырых данных < 4
    // -2 контрольная сумма запроса неверная
    // -3 код команды неверный
    // -99 код команды при инициализации объекта
    int m_err;

    void checkData();

private:
    bool checkSumOk() const;
    bool reqCodeOk() const;
    //quint16 MB_CRC16_cs(const unsigned char * puchMsg, quint16 usDataLen) const;


};




#endif // MBADU_H
