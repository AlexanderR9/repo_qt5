#include "nodejsbridge.h"
#include "processobj.h"
#include "appcommonsettings.h"
#include "deficonfig.h"
#include "ltime.h"
#include "lstring.h"


#include <QDebug>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>



#define SIGN_START      QString("JSON_RESULT_START")
#define SIGN_END        QString("JSON_RESULT_END")
#define QUTE_SYMBOL1    QChar('\"')
#define QUTE_SYMBOL2    QChar('\'')


//NodejsBridge
NodejsBridge::NodejsBridge(QObject *parent, int cid)
    :LSimpleObject(parent)
{
    setObjectName("nodejs_bridge");
    m_userSign = cid;

    initProcessObj();
}
bool NodejsBridge::buzy() const
{
    if (!m_procObj) return false;
    return m_procObj->isRunning();
}
void NodejsBridge::initProcessObj()
{
    m_procObj = new LProcessObj(this);
    connect(m_procObj, SIGNAL(signalFinished()), this, SLOT(slotJSScriptFinished()));

    m_procObj->setCommand("node");
    m_procObj->setProcessDir(AppCommonSettings::nodejsPath());
    m_procObj->setDebugLevel(5);
}
QString NodejsBridge::jsonCommandValue(int cmd)
{
    switch (cmd)
    {
        //read reqs
        case nrcBalance:    return QString("balance");
        case nrcTXCount:    return QString("tx_count");
        case nrcApproved:   return QString("approved");
        case nrcGasPrice:   return QString("gas_price");
        case nrcChainID:    return QString("chain_id");
        case nrcTXStatus:   return QString("tx_status");
        case nrcPoolState:  return QString("pool_state");
        case nrcPositions:  return QString("positions");

        //tx reqs
        case txWrap:        return QString("wrap");
        case txUnwrap:      return QString("unwrap");
        case txTransfer:    return QString("transfer");
        case txApprove:     return QString("approve");
        case txSwap:        return QString("swap");

        default: break;
    }
    return QString("cmd_invalid");
}
int NodejsBridge::commandCodeByTxKind(QString req_name)
{
    req_name = req_name.trimmed().toLower();
    if (req_name == "wrap") return txWrap;
    else if (req_name == "unwrap") return txUnwrap;
    else if (req_name == "transfer") return txTransfer;
    else if (req_name == "approve") return txApprove;
    else if (req_name == "swap") return txSwap;
    return -1;
}
void NodejsBridge::slotJSScriptFinished()
{
    qDebug()<<QString("NodejsBridge::slotJSScriptFinished tab_%1").arg(m_userSign);

    QString result(m_procObj->buffer().trimmed());
    qDebug()<<QString("NodeJS REPLY:  ") << result;

    int code = 0; //код успешности выполнения скрипта
    if (!result.contains(SIGN_START))
    {
        emit signalError(QString("invalid NODE_JS result, not found sign [%1]").arg(SIGN_START));
        code = -1;
    }
    else if (!result.contains(SIGN_END))
    {
        emit signalError(QString("invalid NODE_JS result, not found sign [%1]").arg(SIGN_END));
        code = -2;
    }
    else
    {
        //извлекаем полезный кусок строки
        QString s_json = LString::strBetweenStr(result, SIGN_START, SIGN_END);
        emit signalMsg("---------NODE_JS string result----------");
        emit signalMsg(s_json);
        parseJsReply(s_json, code);
    }
    emit signalFinished(code);
}
void NodejsBridge::parseJsReply(const QString &js_reply, int &code)
{
    //преобразуем полученный ответ в корретную для парсинга JSON строку
    QString s_json = transformJsonResult(js_reply);
    emit signalMsg("--------- transformed result----------");
    emit signalMsg(s_json);

    // Парсим JSON-ответ
    QJsonParseError parseError;
    QJsonDocument outputDoc = QJsonDocument::fromJson(s_json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        emit signalError(QString("Ошибка парсинга JSON: %1").arg(parseError.errorString()));
        code = -3;
        return;
    }

    //получен готовый QJsonObject - ответ
    QJsonObject resultObj = outputDoc.object();
    emit signalMsg("JSON result was parsed successful!");
    //qDebug()<<QString("result object contains %1 fields").arg(resultObj.keys().count());
    if (resultObj.keys().contains("error")) // в результате присутствует поле 'error'
    {
        emit signalError(resultObj.value("error").toString());
        code = -4;
        return;
    }

    //reply OK, send reply to current page
    emit signalNodejsReply(resultObj);
}
void NodejsBridge::slotRunScriptArgs(const QStringList &args)
{
    qDebug()<<QString("NodejsBridge::slotRunScriptArgs tab_%1").arg(m_userSign);
    if (!m_procObj) {emit signalError("m_procObj object is NULL"); return;}

    m_procObj->setArgs(args);
    emit signalMsg(QString("%1  start node_js process %2").arg(LTime::strCurrentTime()).arg(LString::symbolString('.', 50)));
    emit signalMsg(QString("command [%1]").arg(m_procObj->fullCommand()));

    m_procObj->startCommand();
}





//private
QString NodejsBridge::transformJsonResult(const QString &str_json) const
{
    QString s = str_json.trimmed();
  //  qDebug()<<QString("transformJsonResult STAGE1: %1").arg(s);
    if (s.isEmpty()) return s;
    int len = s.length();
    if (s.at(0) != '{' || s.at(len-1) != '}') return QString("invalid_json");
//    s = LString::strBetweenStr(s, "{", "}");

    //удаляем обрамляющие скобки JSON '{  }'
    s = LString::strTrimLeft(s, 1);
    s = LString::strTrimRight(s, 1);
    s = s.trimmed();

    //удаляем лишние пробелы и спец символы
    s = LString::removeLongSpaces(s);
    s = LString::removeSymbol(s, QChar('\n'));
    s = LString::removeSymbol(s, QUTE_SYMBOL1);
    s = LString::removeSymbol(s, QUTE_SYMBOL2);
    s = s.trimmed();

    //определяем длину полезных данных JSON
    len = s.length();
    if (s.at(len-1) == ',') {s = LString::strTrimRight(s, 1); s = s.trimmed(); len = s.length();}
   // qDebug()<<QString("trimed JSON: |%1|").arg(s);
    //строка готова к разметке позиций пар ключ/значение.


    //для начала проведем поиск элементов значений-массивов и в них разделитель символ ',' заменим на ';'
    int arr_start = -1;
    int arr_end = -1;
    int pos = 0;
    while (2 > 1)
    {
        if (arr_start > 0 && arr_end > 0) //found arr-value
        {
            s = LString::replaceByRange(s, QString(","), QString(";"), arr_start, arr_end);
            arr_start = arr_end = -1;
        }
        if (pos >= s.length()) break;

        if (arr_start < 0)
        {
            if (s.at(pos) == QChar('[')) arr_start = pos;
            pos++;
            continue;
        }
        if (arr_end < 0)
        {
            if (s.at(pos) == QChar(']')) arr_end = pos;
            pos++;
            continue;
        }
    }
    qDebug()<<QString("trimed JSON_next: |%1|").arg(s);

    //разбиваем строку на пары JSON (<ключ> : <значение>)
    QStringList list = LString::trimSplitList(s, ",");
    qDebug()<<QString("----------json pairs %1-------------").arg(list.count());
    s.clear(); //final transformed JSON-result
    for(int i=0; i<list.count(); i++)
    {
        qDebug()<<QString("PAIR_%1  [%2]").arg(i+1).arg(list.at(i));
        if (!s.isEmpty()) s.append(QString(", "));

        QString field = list.at(i).trimmed();
        pos = field.indexOf(QChar(':'));
        if (pos > 0)
        {
            //qDebug()<<QString("pos = %1").arg(pos);
            QString f_name = field.left(pos).trimmed();
            QString f_value = LString::strTrimLeft(field, pos+1).trimmed();
            QString j_line = QString("%1%2%1 : %1%3%1").arg(QUTE_SYMBOL1).arg(f_name).arg(f_value);
            if (f_value.at(0) == QChar('['))  //is arr element
            {
                f_value.replace(QString(";"), QString(","));
                j_line = QString("%1%2%1 : %3").arg(QUTE_SYMBOL1).arg(f_name).arg(f_value);
            }
            s.append(j_line);
        }
        else s.append(field);
    }

    s.prepend(QChar('{'));
    s.append(QChar('}'));
    return s;
}










