#ifndef DEFICONFIGLOADER_H
#define DEFICONFIGLOADER_H

#include "lsimpleobj.h"

class QDomNode;

//класс используется для загрузки конфига в глобальную структуру DefiConfiguration defi_config;
//объект класса работает только при старте программы
class DefiConfigLoader : public LSimpleObject
{
    Q_OBJECT
public:
    DefiConfigLoader(QObject*);
    virtual ~DefiConfigLoader() {}

    virtual QString name() const {return QString("defi_config_loader");}
    void loadDefiConfiguration(QString); //загрузить всю конфигурацию defi_config

protected:
    void readChainsNode(const QDomNode&);
    void readTokensNode(const QDomNode&);
    void readPoolsNode(const QDomNode&);
    void readPoolPrioritetNode(const QDomNode&);
    void readBBNode(const QDomNode&);
    void readTargetWalletNode(const QDomNode&);

};



#endif // DEFICONFIGLOADER_H


