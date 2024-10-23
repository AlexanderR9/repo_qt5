#ifndef LMBADU_H
#define LMBADU_H


#include <QByteArray>

class QModbusPdu;

//LMBAduBase
class LMBAduBase
{
public:
    LMBAduBase(const QByteArray &input_data);

    inline bool invalid() const {return !is_valid;}
    inline int errCode() const {return m_err;}
    inline int rawSize() const {return m_data.size();} //размер сырых данных запроса
    inline const QByteArray& rawData() const {return m_data;}

    virtual qint8 serverAddress() const = 0;
    virtual qint8 cmdCode() const = 0;

    virtual QString stringErr() const;
    virtual void getPduData(QModbusPdu &pdu) const;

    QString rawDataToStr() const;

    static bool functionCodeOk(qint8); //проверяет валидность кода функции modbus
    static quint16 convertToLittleEndian(const quint16); //развернуть порядок байт в значении
    static int registerTypeByFunc(quint8); //выдает тип регистров по коду функции или -1
    static QString functionType(quint8); //выдает тип команды (write/read) и вид регистров
    static quint8 bitSwither(quint8 bit_index); //значения (8шт) - степени 2, служат для вкл/выкл битов в байте, bit_index= 0..7


    //функция подготавливает QByteArray(данные запроса) для установки его в переменную QModbusRequest.
    //данные QByteArray соответствуют команде «0x10» — запись группы регистров Holding.
    //результат записывает в переданный в параметре QByteArray.
    static void prepareWriteHoldingReqData(quint16 start_reg, const QVector<float> &values, QByteArray&);

    //функция подготавливает QByteArray(данные запроса) для установки его в переменную QModbusRequest.
    //данные QByteArray соответствуют команде «0x0F» — запись группы регистров Coils.
    //результат записывает в переданный в параметре QByteArray.
    static void prepareWriteCoilsReqData(quint16 start_reg, const QVector<qint8> &values, QByteArray&);

protected:
    QByteArray m_data;
    bool is_valid;

    // -1 размер сырых данных меньше допустимого
    // -2 контрольная сумма запроса неверная
    // -3 код команды неверный
    // -4 длина пакета mbtcp <= 0
    // -5 длина пакета mbtcp некорректна
    // -99 код команды при инициализации объекта
    int m_err;

    virtual void checkData() = 0;

};

//MBAdu
class LMBAdu : public LMBAduBase
{
public:
    LMBAdu(const QByteArray &input_data);

    qint8 serverAddress() const;
    qint8 cmdCode() const;
    quint16 rawDataCRC() const;
    quint16 startPosReg() const;
    void getPduData(QModbusPdu &pdu) const;

    QString stringErr() const;
    inline bool readingCmd() const {return (cmdCode() == quint8(0x03));}
    inline bool writingCmd() const {return (cmdCode() == quint8(0x10));}

    static quint16 MB_CRC16(const QByteArray&, int len = -1); // если len < 0, то значит использовать весь BA

protected:
    quint16 must_crc;

    void checkData();
    void checkSumOk();

};

//LMBTcpAdu
class LMBTcpAdu : public LMBAduBase
{
public:
    LMBTcpAdu(const QByteArray &input_data);

    qint8 serverAddress() const; //адрес устройсва
    qint8 cmdCode() const; //код команды (MODBUS)
    qint16 packetLen() const; //длина пакета, которая лежит в заголовке mbtcp запроса (sizeof PDU + 1)
    QString stringErr() const;
    void getPduData(QModbusPdu &pdu) const;
    bool isExeptionRequest() const; //признак того что этот ответ является исключением, где cmdCode() содержит код ошибки, который говорит почему сервер не может выполнить текущий запрос
    QString strExeption() const;

    QString toStr() const; //выдает в строковом виде основные параметры сообщения (для отладки)

    static quint16 mbapSize() {return 7;} //заголовок MBTCP запроса/ответа



protected:
    void checkData();

};



#endif // MBADU_H
