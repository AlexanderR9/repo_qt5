#ifndef TG_CONFIGLOADERBASE_H
#define TG_CONFIGLOADERBASE_H


#include <QString>

struct LTGParamsBot;
class QDomDocument;
class QDomNode;
class QDomNodeList;



//класс для загрузки параметров бота из конфига .xml
// LTGConfigLoaderBase
class LTGConfigLoaderBase
{
public:
    LTGConfigLoaderBase(const QString&);
    virtual ~LTGConfigLoaderBase() {}

    //загрузить конфиг и установить значения в структуре TGParamsBot, в случае ошибки записать текст ошибки в параметр err
    virtual void loadBotParams(LTGParamsBot&, QString &err);

    //выдать список всех нод из секции конфига: bot_params
    virtual QDomNodeList getTGConfigNodes() const;

protected:
    QString m_config;   //имя конфигурационного файла

    virtual void parseDom(const QDomDocument&, LTGParamsBot&, QString&);
    virtual void parseBotParamsNode(const QDomNode&, LTGParamsBot&, QString&);

};



#endif //TG_CONFIGLOADERBASE_H




