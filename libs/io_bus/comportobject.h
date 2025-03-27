#ifndef LCOMPORT_OBJECT_H
#define LCOMPORT_OBJECT_H


#include <QSerialPort>


struct LComParams;


//LComPortObj
class LComPortObj : public QSerialPort
{
    Q_OBJECT
public:
    LComPortObj(QObject *parent = NULL);
    virtual ~LComPortObj() {}

    void setPortParams(const LComParams&); //установить все рабочие параметры порта
    void updatePortName(QString); // обновить имя порта (tty..)
    QString strDirection() const;

    inline const QByteArray& currentBuffer() const {return m_buffer;}
    inline int bufferSize() const {return m_buffer.size();}
    inline void bufferClear() {m_buffer.clear();}
    inline void setAutoClean(bool b) {m_autoClean = b;}
    inline void setMinReceivedBytes(int n) {m_minReceivedBytes = n; if (m_minReceivedBytes <= 0) m_minReceivedBytes = -1;}

    void tryOpen(QString&); //попытка открыть порт, в случае ошибки в параметр запишется текст ошибки
    void tryClose(QString&);//попытка закрыть порт, в случае ошибки в параметр запишется текст ошибки
    void tryWrite(const QByteArray&, QString&);//попытка записать данные в порт, в случае ошибки в параметр запишется текст ошибки
    bool portOpened() const; //признак того что порт открыт

protected:
    int              m_direction;   //возможные направлления работы порта in/out
    QByteArray       m_buffer;      // рабочий буфер, куда добавляются для обработки принятые данный

    //признак того что надо принудительно очищать m_buffer при приходе новых данных,
    //т.е. m_buffer будет всегда содержать только последние пришедшие данные
    bool m_autoClean;

    //минимальное количество принятых байт на которое нужно реагировать, -1 означает что нет ограничений,
    //по умолчанию -1
    int m_minReceivedBytes;

    int openModeByDirection() const; //тип направлления работы порта

protected slots:
    void slotReadyRead(); //пришли данные
    void slotError();

signals:
    void signalDataReceived(int); // int - размер полученных данных в последней порции, эмитится если текущий размер bufferSize >= m_minReceivedBytes
    void signalPortError(int); //эмитится только в случае ошибки самого порта

};


#endif // COMPORT_OBJECT_H
