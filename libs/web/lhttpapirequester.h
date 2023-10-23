#ifndef LHTTP_API_REQUESTER_H
#define LHTTP_API_REQUESTER_H

#include "lsimpleobj.h"

#include <QJsonObject>
#include <QStringList>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

//структура для хванения ответа полученного от сервера в результате выполнения REST_API запроса
struct LHttpApiReplyData
{
    LHttpApiReplyData() {reset();}

    int result_code; //код результата выполнения, 0 - OK (взят из QNetworkReply)
    QJsonObject data; //сам ответ (объект JSON)
    QStringList headers; // заголовки в ответе

    void reset() {result_code = -1; data = QJsonObject(); headers.clear();}
    bool isOk() const; //обобщенный результат выполнения запроса
    void takeHeaders(const QNetworkReply*); //получить заголовки из пришедшего ответа
    void takeData(QNetworkReply*); //получить код ошибки и данные пришедшего ответа

};


// LHttpApiRequester - классс для выполнения  HTTP запросов к серверу, используя его API.
//для использования необходимо:
// - создать объект
// - подключить сигнал signalFinished(int), который содержит код ошибки/успешности
// - установить тип протокола (m_protocolType )
// - установить домен сервера, предоставляющего АПИ (m_apiServer)
// - установить URI запрашиваемого ресурса (m_uri)
// - заполнить http заголовки (void addReqHeader(QString, QString))
// - заполнить дополнительные данные запроса при необходимости (m_metadata)
// - отправить запрос (void start(int http_metod))
// - после завершения выполнения запроса считать ответ (const LHttpApiReplyData& lastReply() )
//перед каждым запросом можно менять параметры запроса при необходимости
class LHttpApiRequester : public LSimpleObject
{
    Q_OBJECT
public:
    LHttpApiRequester(QObject *parent = NULL);
    virtual ~LHttpApiRequester() {destroyObj();}

    QString name() const {return QString("lhttp_api_requester_obj");}
    virtual void getReqHeaders(QStringList&); //получить http заголовки нашего запроса (diag func)

    //добавить/заменить http заголовок в наш запрос, заголовоки из запроса удалить нельзя,
    //можно переинициализировать объект m_request выполнив функцию reinit(), затем заполнить запрос заново
    virtual void addReqHeader(QString, QString);

    virtual void reinitReq(); //recreate объекта m_request
    virtual QString fullUrl() const; //получить полную ссылку на адрес ресурса сервера (diag func)
    virtual void start(int); // отправить REST_API запрос серверу (param - http metod)

    inline const LHttpApiReplyData& lastReply() const {return m_lastReply;} //получить данные последнего полученного ответа
    inline void clearMetaData() {m_metadata = QJsonObject();} //очистить m_metadata
    inline void addMetaData(QString key, QString value) {m_metadata[key] = value;} //добавить пару(ключ, значение) в m_metadata
    inline void setApiServer(QString s) {m_apiServer = s.trimmed();} //установить доменное имя АПИ сервера
    inline QString apiServer() const {return m_apiServer;}
    inline void setHttpProtocolType(int t) {m_protocolType = t;} //установить тип протогола http
    inline void setUri(QString s) {m_uri = s.trimmed();} //установить адрес(путь) получаемого ресурса на сервере
    inline bool isBuzy() const {return m_buzy;}

    static QString httpType(int t); //return 'http://' or 'https://' or '???'

protected:
    QNetworkAccessManager   *m_manager; // объект, который непосредственно выполняет запрос
    QNetworkRequest         *m_request; //объект, содержащий параметры http запроса
    QJsonObject              m_metadata;  //дополнительные данные, прилагаемые к телу запроса (JSON объект может быть пустым)
    LHttpApiReplyData        m_lastReply; //последний полученный ответ
    int                      m_protocolType; //тип протокола http, елемент множества HttpProtocolType
    QString                  m_apiServer; //доменное имя сервера или его IP (то что идет от http до .ru, включая .ru)
    QString                  m_uri; //адрес(путь) ресурса на сервере (то что идет после .ru без слешей в начале и конце), может быть пустым
    bool                     m_buzy; //признак того что запрос еще выполняется

    virtual void destroyObj();
    virtual void initNetObjects(); //инициализация объектов QNetworkAccessManager, QNetworkRequest
    virtual QString validity() const; // если вернет непустую строку, то это признак того, что некоторые параметры запроса некорректны или отсутствуют

protected slots:
    virtual void slotFinished(QNetworkReply*); //выполняется когда пришел ответ от сервера, не важно с каким результатом
    virtual void slotAuthenticationRequired() {emit signalError(QString("LHttpApiRequester::slotAuthenticationRequired()"));} // заготовка на будущее
    virtual void slotSSLErrors() {emit signalError(QString("LHttpApiRequester::slotSSLErrors()"));} // заготовка на будущее

signals:
    void signalFinished(int); //эмитится когда запрос выполнен (завершен), param - обобщенный результат запроса, елемент множества HttpReplyError

};


#endif

