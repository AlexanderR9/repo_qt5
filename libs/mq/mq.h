#ifndef MQ_H
#define MQ_H
 
#include "lsimpleobj.h"
 
#include <QString>
#include <QColor>

struct mq_attr;
class QByteArray;

 
//класс для работы с одним экземпляром очереди POSIX
//в конструкторе необходимо указать имя очереди без символа '/'.
//при создании объект еще не может работать с очередью, сначала нужно к ней подключиться или создать ее.
//объект может подключиться(отключиться) к существующей очереди POSIX.
//объект может записывать/считывать данные в/из очередь.
//объект может создать новую очередь POSIX, а также удалить ее из системы.
class MQ : public LSimpleObject
{
    Q_OBJECT
public:
    enum MQState {mqsDeinit = 221,  mqsOpened, mqsClosed, mqsCreated, mqsNotFound, mqsInvalid};

    MQ(const QString&, QObject *parent = NULL);
    virtual ~MQ() {}
    
    inline QString name() const {return mq_name;} //имя очереди POSIX
    inline int size() const {return m_size;} //текущий размер всех сообщений находящихся в очереди
    inline bool invalid() const {return (m_handle <= 0);}
    inline int handle() const {return m_handle;} //дескриптор очереди, к которой подключен объект
    inline bool isOpened() const {return (m_state == mqsOpened || m_state == mqsCreated);}
    inline bool isNotFound() const {return (m_state == mqsNotFound);}
    inline bool isDeinit() const {return (m_state == mqsDeinit);}

    
    QString strMode() const;
    QString strState() const;
    QString strStatus() const;
    QString strAttrs() const;
    QColor colorStatus() const;
    void updateAttrs(); //обновить информацию об текущем состоянии очереди
    bool hasMsg() const; //признак наличия сообщений в очереди

    void tryOpen(int, bool&); //подключиться к существующей очереди POSIX
    void tryClose(bool&); //отключиться от очереди POSIX
    void tryCreate(int, quint32, bool&); //создать очередь POSIX (после создания объкт будет к ней подключен)
    void tryDestroy(bool&); //уничтожить из системы очередь POSIX
    void trySendMsg(const QByteArray&, bool&); //отправить сообщение(массив байт) в очередь POSIX (объект должен быть подключен к очереди)
    void tryReadMsg(QByteArray&); //считать сообщение из очереди, в случае ошибки ba будет пустой

    void resetState();

    	
protected:
    QString  	mq_name;   	
    int 		m_size;		//текущий размер очереди
    int			m_handle;	//дескриптор очереди posix
    int 		m_state;	// MQState enum element
    int 		m_mode; 	//IODevice enum element
    mq_attr 	*m_attrs; 	//current attributes
    
    //проверить наличие файла-очереди POSIX для этого объекта в каталоге /dev/mqueue.
    //если такая очередь не существует, а статус объекта висит как ОТКРЫТА то функция пытается ее закрыть.
    //после выполнения этой функции нужно выполнить: if (isNotFound()) { to do }
    void checkQueueFile(bool check_invalid = true);

private:
    const char* charName() const; 
    int mqModeByMode() const;
    bool existPosixFile() const; //признак наличия файла-очереди /dev/mqueue/mq_name
    
};
 
#endif
 
  
