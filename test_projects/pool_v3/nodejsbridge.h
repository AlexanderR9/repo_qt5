#ifndef NODEJSBRIDGE_H
#define NODEJSBRIDGE_H

#include "lsimpleobj.h"


class LProcessObj;
class QStringList;



//класс для взаимодействия со скриптами nodejs.
//NodejsBridge
class NodejsBridge : public LSimpleObject
{
    Q_OBJECT
public:
    NodejsBridge(QObject*, int);
    virtual ~NodejsBridge() {}

    bool buzy() const;

    static QString jsonCommandValue(int); //значение поля 'req_name' в файле-параметрах  по коду команды
    static int commandCodeByTxKind(QString); //инерсное выполнение функции jsonCommandValue

protected:
    LProcessObj     *m_procObj; //объект для запуска скриптов

    void initProcessObj();
    void parseJsReply(const QString&, int&); // прочитать полученный очередной ответ по завршению работы скрипта

private:
    //при формировании результа (json) в node_js скриптах - недопустимо использовать символы ',' '['  ']' ':' в текстовых значениях полей,
    //т.к. по этим символам распознаються границы пар-значений и значения-массивы.
    //данная функция подрехтовывает JS json-result чтобы он был валиден для загрузки в QJsonObject
    QString transformJsonResult(const QString&) const;


protected slots:
    void slotJSScriptFinished();

public slots:
    void slotRunScriptArgs(const QStringList&);

signals:
    void signalFinished(int); // признак завершения работы скрипта, в том числе и неудачно, выполняется всегда при завершении nodejs скрипта
    void signalNodejsReply(const QJsonObject&); //выполняется при завершении скрипта и при этом был получен нормальный ответ (без ошибок)


};





#endif // NODEJSBRIDGE_H


