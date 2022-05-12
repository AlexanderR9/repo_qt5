#ifndef LMBSLAVESERVERBASE_H
#define LMBSLAVESERVERBASE_H


#include <QModbusServer>

class QSerialPort;
struct LComParams;

//LMBSlaveServerBase
class LMBSlaveServerBase : public QModbusServer
{
    Q_OBJECT
public:
    LMBSlaveServerBase(QObject *parent = NULL);
    virtual ~LMBSlaveServerBase() {}

    virtual void setPortParams(const LComParams&); //установка параметров COM порта

    bool isConnected() const;           //COM port state - ConnectedState
    bool isDisconnected() const;        //COM port state - UnconnectedState
    bool isClosing() const;             //COM port state - ClosingState

    //должна вернуть true, если текущий обрабатываемый запрос является широковещательным запросом
    virtual bool processesBroadcast() const {return is_broadcast;}

    inline bool bufferEmpty() const {return m_buffer.isEmpty();}
    inline void clearBuffer() {m_buffer.clear();}
    inline int bufferSize() const {return m_buffer.size();}

protected:
    QSerialPort *m_port;
    QByteArray m_buffer;
    bool is_broadcast;

    virtual bool open(); //открытие COM порта
    virtual void close(); //закрытие COM порта
    virtual void parseCurrentBuffer() = 0; //считанные байты из m_port добавляются к m_buffer, после чего требуется обработать m_buffer
    virtual void convertResponseToBA(const QModbusResponse&, QByteArray&); //преобразовать ответ (QModbusResponse) в QByteArray для возможной записи в COM порт
    virtual void trySendResponse(const QModbusResponse&); //попытка записи ответа в COM порт
    virtual void makeResponseByRequest(const QModbusRequest&, QModbusResponse&); //сформировать ответ на запрос по правилам протокола modbus

    virtual quint16 minRequestSize() const {return 4;}  //минимально допустимый размер запроса
    inline bool bufferSmall() const {return (m_buffer.size() < minRequestSize());} //текущий буффер слишком маленький для обработки, необходимо ждать пока придут еще данные

    QModbusResponse processRequest(const QModbusPdu&) override; //подготовить ответ (QModbusResponse) на полученный запрос (request) по правилам протокола modbus
    virtual QModbusResponse exeptionRequest(const QModbusPdu&) const = 0; //обработать исключение запроса

    //Вызывается перед попыткой открытия COM порта.
    //Устанавливает зарегистрированную таблицу типов регистров, значения регистров инициализируются 0.
    //При вызове этой функции любое ранее установленное значение регистра отбрасывается.
    virtual void initRegistersMap() = 0;

protected slots:
    virtual void slotReadyRead(); //пришли данные в m_port, обработать, возможно запрос от мастера
    virtual void slotAboutToClose(); //выполняется при закрытии устройства(m_port) по каким-либо причинам
    virtual void slotError(); //выполняется при возникновении ошибки в работе m_port

signals:
    void signalError(const QString&); //для отправки сообщения в протокол (внешнему объекту)

private:
    void initComPort(); //инициализация COM порта

};




#endif // MBSLAVESERVERBASE_H


