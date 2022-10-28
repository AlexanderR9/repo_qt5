#ifndef MQ_WORKER_H
#define MQ_WORKER_H

#include "lsimpleobj.h"


class QSettings;
class MQ;

//если нужно работать с набором очередей, то испльзуйте этот класс.

//класс для работы с контейнером очередей posix
class MQWorker : public LSimpleObject
{
    Q_OBJECT
public:
	MQWorker(QObject *parent = NULL);
    virtual ~MQWorker() {}

    void reset();
    void createQueueObj(const QString&); //создать новый обьект MQ и добавить в m_queues (это не означает что будет создана реальная очередь POSIX)
    void destroyQueueObj(const QString&); //разрушить обьект MQ и удалить из m_queues

    //user actions
    void openQueue(int, int); //params: 1-index_mq, 2-open_mode
    void closeQueue(int);
    void newQueue(const QString&, int, bool&); ////params: 1-new_mq_name, 2-open_mode, 3-result
    void removeQueue(int);
    void sendMsg(int, const QByteArray&, bool&); //отправить сообщение в заданную очередь
    void readMsg(int, QByteArray&); //считать сообщение из заданной очереди (в случае ошибки ba будет пустой)
    void updateState();


    const MQ* lastQueue() const;
    const MQ* firstQueue() const;
    const MQ* queueAt(int) const;
    bool queueContains(const QString&) const; //проверить, существует ли очередь с таким именем
    inline bool isEmpty() const {return m_queues.isEmpty();}
    inline int count() const {return m_queues.count();}

    static QString mqLinuxDir() {return QString("/dev/mqueue");}

protected:
    QList<MQ*>	m_queues;

private:
    MQ* getMQByName(const QString&) const; //выдать элемент из m_queues по имени очереди

};



#endif

