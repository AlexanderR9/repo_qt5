#include "lbot.h"
#include "lstatic.h"


#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <QDomNode>
#include <QJsonDocument>

#include <TarnaBasicSender>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QByteArray>


LBot::LBot(QObject *parent)
    :LSimpleObject(parent),
    m_botObj(NULL),
    bot_url(QString("https://api.telegram.org")),
    m_request(NULL),
    m_netManager(NULL),
    m_reqCode(tgrcInvalid)
{
    setObjectName("lbot");
    m_token = QString("???");


    m_request = new QNetworkRequest();
    m_request->setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_netManager = new QNetworkAccessManager(this);
    QObject::connect(m_netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotRequestFinished(QNetworkReply*)));

    connect(this, SIGNAL(signalJsonReceived(QJsonObject)), this, SLOT());

}
LBot::~LBot()
{
    if (m_request) delete m_request;


}
void LBot::slotRequestFinished(QNetworkReply *reply)
{
    qDebug("LBot::slotRequestFinished");

    if (reply)
    {
        QJsonObject result = QJsonDocument::fromJson(reply->readAll()).object();
        emit signalJsonReceived(result);
    }
}
void LBot::slotJsonReceived(QJsonObject reply_obj)
{

}
void LBot::init()
{
    if (m_botObj) {delete m_botObj; m_botObj = NULL;}


    if (m_token.isEmpty())
    {
        emit signalError("LBot: WARNING - token is empty");
        return;
    }

    bot_url = QString("%1/bot%2").arg(bot_url).arg(m_token);

    //QNetworkProxy proxy = QNetworkProxy(QNetworkProxy::Socks5Proxy, "localhost", 9050);
//    TarnaBasicSender *basic_sender = new TarnaBasicSender(m_token);
  //  m_botObj = new TarnaBot(basic_sender);

    signalMsg("Bot object initializate OK!");
    signalMsg(QString("Bot url: [%1]").arg(bot_url));

}
void LBot::getMe()
{
    if (!m_botObj) {signalError("Bot object is NULL!"); return;}

    /*
    User u = m_botObj->getMe();
    QString msg = QString("ME: first_name=[%1]  last_name=[%2]  user_name=[%3]").arg(u.getFirstName()).arg(u.getLastName()).arg(u.getUsername());
    msg = QString("%1  is_bot=[%2]").arg(msg).arg(u.getIsBot() ? "yes" : "no");
    msg = QString("%1  ID=[%2]").arg(msg).arg(u.hasId() ? u.getId() : -1);
    */

    m_reqCode = tgrcGetMe;
    QJsonObject jsonObject;
    sendJsonRequest(jsonObject, "getMe");


    //emit signalMsg(msg);
}
void LBot::sendMsg()
{
    if (!m_botObj) {signalError("Bot object is NULL!"); return;}

 //   qint64 id = 2007125359;
    qint64 id = 1975188389;
    QString text("test text");
    m_botObj->sendMessage(id, text);
    emit signalMsg("msg sended");

}
void LBot::getUpdates()
{
    if (!m_botObj) {signalError("Bot object is NULL!"); return;}

//    QVector<Update> getUpdates(qint64 offset = 0, int limit = 100, qint64 timeout = 100, QVector<QString> allowedUpdates = QVector<QString>());

    QVector<Update> updates(m_botObj->getUpdates(529822261, 100));
    if (updates.isEmpty())
    {
        emit signalMsg("updates is empty");
        return;
    }

    QString msg = QString("UPDATES: count=%1").arg(updates.count());


}
void LBot::sendJsonRequest(const QJsonObject &json_obj, const QString &api_method)
{
    //QJsonObject result;
    QUrl request_url = QString("%1/%2").arg(bot_url).arg(api_method);
    m_request->setUrl(request_url);
    m_netManager->post(*m_request, QJsonDocument(json_obj).toJson());

    /*
    QUrl url;
    url.setUrl(mUrl + apiMethod);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(url);

    QEventLoop loop;
    QObject::connect(&mNam, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    QNetworkReply* reply = mNam.post(request, QJsonDocument(jsonObject).toJson());
    loop.exec();

    result = QJsonDocument::fromJson(reply->readAll()).object();
    delete reply;
    return result;
    */
}



void LBot::loadConfig(const QString &fname)
{
    QString err;
    if (fname.trimmed().isEmpty())
    {
        err = QString("LBot: WARNING - config filename is empty ");
        emit  signalError(err);
        return;
    }
    QFile f(fname.trimmed());
    if (!f.exists())
    {
        err = QString("LBot: WARNING - config filename [%1] not found").arg(fname);
        emit  signalError(err);
        return;
    }
    QDomDocument dom;
    if (!dom.setContent(&f))
    {
        err = QString("LBot: WARNING - config filename [%1] can't load to DomDocument").arg(fname);
        emit  signalError(err);
        if (f.isOpen()) f.close();
        return;
    }
    f.close();

    QString root_node_name("config");
    QDomNode root_node = dom.namedItem(root_node_name);
    if (root_node.isNull())
    {
        err = QString("LBot: invalid struct XML document [%1], node <%2> not found.").arg(fname).arg(root_node_name);
        emit signalError(err);
        return;
    }

    QString token_node_name("token");
    QDomNode token_node = root_node.namedItem(token_node_name);
    if (token_node.isNull())
    {
        err = QString("LBot: invalid struct XML document [%1], node <%2> not found.").arg(fname).arg(token_node_name);
        emit signalError(err);
        return;
    }

    m_token = LStatic::getStringAttrValue("value", token_node);
    if (m_token.isEmpty())
    {
        err = QString("LBot: invalid token value.").arg(fname).arg(token_node_name);
        emit signalError(err);
        return;
    }

     err = QString("MBConfigLoader: config loaded ok!");
     emit signalMsg(err);
     err = QString("token = [%1]").arg(m_token);
     emit signalMsg(err);

     init();
}



////////////////////////////////////////////
/*
#include <QCoreApplication>
#include <QNetworkProxy>

#include "gamebot.h"
using namespace Telegram;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    GameBot bot("token", QNetworkProxy(QNetworkProxy::Socks5Proxy, "localhost", 9050), 1000);
    return a.exec();
}
*/
////////////////////////////////////////////
/*
#include <QRandomGenerator>

#include <TarnaBot>
#include <InlineKeyboardMarkup>

namespace Telegram
{
    class GameBot : public TarnaBot
    {
    public:
        GameBot(QString token, QNetworkProxy proxy, qlonglong interval, QObject* parent = nullptr);

    public slots:
        void handleUpdate(Update update);

    private:
        void processQuery(CallbackQuery query);
        void sendMyGame(int newScore, qint64 chatId, qint64 messageId);
        InlineKeyboardMarkup createKeyboard(int score, QVector<int> numbers);

        QVector<int> decode(QString data);
        QString encode(QVector<int> values);
        QVector<int> createRandomList();
    };
}

*/
////////////////////////////////////////////
/*




#include "gamebot.h"
using namespace Telegram;

GameBot::GameBot(QString token, QNetworkProxy proxy, qlonglong interval, QObject *parent) :
    TarnaBot(token, proxy, interval, parent)
{
    connect(this, &GameBot::updateReceived, this, &GameBot::handleUpdate);
}

void GameBot::handleUpdate(Update update)
{
    //Determine whether a button was pressed or a /start command was sent
    if(update.hasCallbackQuery())
    {
        processQuery(update.getCallbackQuery());
    }
    if(update.hasMessage() && update.getMessage().hasText())
    {
        if(update.getMessage().getText().startsWith("/start"))
            sendMyGame(0, update.getMessage().getChat().getId(), -1);
    }
}

void GameBot::processQuery(CallbackQuery query)
{
    //check if data is valid, and decode it
    QVector<int> values = decode(query.getData());
    if(values.isEmpty())
        return;

    //Check if the game is finished
    if(values[0] == 99)
    {
        sendMessage(query.getMessage().getChat().getId(),
                    "Congratulations! You have finished the game!");
        return;
    }
    //assign new score based on the key pressed, and update the message
    sendMyGame(values[1] + values[2] == values[3] ? values[0] + 1 : values[0],
            query.getMessage().getChat().getId(),
            query.getMessage().getMessageId());
}

void GameBot::sendMyGame(int newScore, qint64 chatId, qint64 messageId)
{
    //Create random numbers, create keyboard using this numbers
    QVector<int> numbers = createRandomList();
    InlineKeyboardMarkup replyMarkup = createKeyboard(newScore, numbers);

    QString messageText = QString("Score: %1\n"
                                  "%2 + %3 = ?").arg(QString::number(newScore),
                                                     QString::number(numbers[0]),QString::number(numbers[1]));
    //Send or update message
    if(messageId < 0)
    {
        sendMessage(chatId, messageText, "", false, false, -1, &replyMarkup);
    }

    else
        editMessageText(messageText, QString::number(chatId), messageId, "", "", false, &replyMarkup);
}

QVector< int > GameBot::createRandomList()
{
    QVector< int > numbers;
    QRandomGenerator random(QDateTime::currentDateTime().toSecsSinceEpoch());
    numbers.resize(8);

    //Generate the numbers about which we'll ask
    numbers[0] = random.bounded(20) + 1;
    numbers[1] = random.bounded(20) + 1;
    //Find a random place for the correct answer
    int resultIndex = random.bounded(6) + 2;

    //Generate some random numbers
    for(int i = 2; i < 8; i++)
    {
        if(i == resultIndex)
            continue;
        numbers[i] = random.bounded(40) + 1;
    }

    numbers[resultIndex] = numbers[0] + numbers[1];
    return numbers;
}

QString GameBot::encode(QVector<int> values)
{
    //values: score, a, b, result
    QString encoded = "";

    foreach(int i, values)
    {
        if(i < 10)
            encoded += '0';
        encoded += QString::number(i);
    }
    return encoded;
}

QVector< int > GameBot::decode(QString data)
{
    QVector< int > values;
    if(data.length() != 8)
        return values;
    for(int i =  0; i < 8; i+= 2)
    {
        values.append(QString(data.mid(i, 2)).toInt());
    }
    return values;
}

InlineKeyboardMarkup GameBot::createKeyboard(int score, QVector<int> numbers)
{
    QVector< QVector< InlineKeyboardButton > > buttons;
    QVector< int > values;
    values.resize(4);
    values[0] = score;
    values[1] = numbers[0];
    values[2] = numbers[1];
    buttons.resize(2);

    for(int i = 0; i < 2; i++)
    {
        buttons[i].resize(3);
        for(int j = 0; j < 3; j++)
        {
            values[3] = numbers[i * 3 + j + 2];
            buttons[i][j] = InlineKeyboardButton(QString::number(values[3]));
            buttons[i][j].setCallbackData(encode(values));
        }
    }
    return InlineKeyboardMarkup(buttons);
}



 * */


