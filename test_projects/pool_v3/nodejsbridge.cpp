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
        case nrcPosState:  return QString("pos_state");

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
void NodejsBridge::parseNextJsonElement(QString &raw_json, QString &s_el) const
{
    //необходимо подготовить трансформированный элемент вида: "key" : "value"
    s_el.clear();
    raw_json = raw_json.trimmed();
    if (raw_json.isEmpty()) return;

    //извлекаем ключ
    int pos = raw_json.indexOf(QChar(':'));
    if (pos <= 0) return;
    QString key = raw_json.left(pos).trimmed();
    raw_json = LString::strTrimLeft(raw_json, pos+1).trimmed(); // отрезаем слева ключ вместе с разделителем ':'
    if (raw_json.isEmpty()) return;

    //извлекаем значение
    QString value = QString();
    if (raw_json.at(0) == QChar('[')) // element is arr
    {
        pos = raw_json.indexOf(QChar(']'));
        if (pos <= 0) return; // wrong json
        value = raw_json.left(pos+1).trimmed(); // извлекаем значение-arr полностью
        raw_json = LString::strTrimLeft(raw_json, pos+1).trimmed(); // отрезаем слева из основной строки
    }
    else if (raw_json.at(0) == QChar('{')) // element is json
    {
        pos = raw_json.indexOf(QChar('}'));
        if (pos <= 0) return; // wrong json
        value = raw_json.left(pos+1).trimmed(); // извлекаем значение-json полностью
        raw_json = LString::strTrimLeft(raw_json, pos+1).trimmed(); // отрезаем слева из основной строки
    }
    else // simple elemment
    {
        pos = raw_json.indexOf(QChar(','));
        value = raw_json.trimmed();
        if (pos > 0) value = raw_json.left(pos).trimmed();
        raw_json = LString::strTrimLeft(raw_json, pos).trimmed(); // отрезаем слева из основной строки
        if (value.isEmpty()) return;
    }

    qDebug()<<QString("ROW_PAIR - %1 : %2").arg(key).arg(value);


    wrapValueQuotes(key); // обрамляем ключ кавычками
    wrapValueQuotes(value); // обрамляем значение кавычками
    s_el = QString("%1 : %2").arg(key).arg(value);
}
void NodejsBridge::wrapValueQuotes(QString &j_value) const
{
    j_value = j_value.trimmed();
    if (j_value.isEmpty()) return; //invalid value

    if (j_value.at(0) != QChar('[') && j_value.at(0) != QChar('{')) // значение - простой элемент
    {
        j_value.prepend(QUTE_SYMBOL1);
        j_value.append(QUTE_SYMBOL1);
        return;
    }

    // сложный элемент
    QChar open_tag = j_value.at(0);
    QChar close_tag = j_value.at(j_value.length()-1);
    j_value = LString::strTrimLeft(j_value, 1);
    j_value = LString::strTrimRight(j_value, 1);
    j_value = j_value.trimmed();

    //удаляем последнюю висячую запятую если она присутствует
    if (j_value.at(j_value.length()-1) == QChar(','))
        j_value = LString::strTrimRight(j_value, 1).trimmed();

    if (open_tag == QChar('[')) // array
    {
        QStringList list = LString::trimSplitList(j_value, ","); // разбиваем на элементы
        j_value.clear();
        for (int i=0; i<list.count(); i++) // оборачиваем каждый элемент массива в кавычки и составляем строку обратно
        {
            if (j_value.isEmpty()) j_value = QString("%1%2%1").arg(QUTE_SYMBOL1).arg(list.at(i));
            else j_value = QString("%1, %2%3%2").arg(j_value).arg(QUTE_SYMBOL1).arg(list.at(i));
        }
    }
    else //json-element, этот подэлемент должен содержать элементы только простых типов
    {
        QStringList list = LString::trimSplitList(j_value, ","); // разбиваем на элементы:  key : value
        j_value.clear();
        foreach (const QString &v, list) // перебираем пары и оборачиваем ключ и значение каждой в кавычки
        {
            QStringList key_value = LString::trimSplitList(v.trimmed(), ":"); // разбиваем пару на ключ и значение
            if (key_value.count() != 2) continue; // wrong element
            QString finished_el = QString("%1%2%1 : %1%3%1").arg(QUTE_SYMBOL1).arg(key_value.first()).arg(key_value.last());

            if (j_value.isEmpty()) j_value = finished_el.trimmed(); // собираем обратно отформатированную строку
            else j_value = QString("%1, %2").arg(j_value).arg(finished_el.trimmed());
        }
    }

    //добавляем скобки обратно
    j_value.prepend(open_tag);
    j_value.append(close_tag);
}

QString NodejsBridge::transformJsonResult(const QString &str_json) const
{
    QString s = str_json.trimmed();
  //  qDebug()<<QString("transformJsonResult STAGE1: %1").arg(s);
    if (s.isEmpty()) return s;
    int len = s.length();
    if (s.at(0) != '{' || s.at(len-1) != '}') return QString("invalid_json");

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

    //удаляем последнюю висячую запятую если она присутствует
    if (s.at(s.length()-1) == QChar(',')) s = LString::strTrimRight(s, 1).trimmed();


    // перебираем элементы и собираем новую правильную строку
    int i = 0;
    QString result;
    while (2 > 1)
    {
        i++;
        QString s_el;
        parseNextJsonElement(s, s_el);
        if (s_el.trimmed().isEmpty())  break;

        if (result.isEmpty()) result = s_el.trimmed(); // собираем обратно отформатированную строку
        else result = QString("%1, %2").arg(result).arg(s_el.trimmed());

       // qDebug()<<QString("TRANSFORMED_PAIR_%1 - %2").arg(i).arg(s_el);

        s = s.trimmed();
        if (s.at(0) == QChar(','))
            s = LString::strTrimLeft(s, 1).trimmed(); // отрезаем оставшийся разделитель с прошлого элемента

        if (i > 1000) {qWarning("WARNING while incorrect"); break;}
    }



    result.prepend(QChar('{'));
    result.append(QChar('}'));
    return result;
}










