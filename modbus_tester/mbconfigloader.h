#ifndef MB_CONFIG_LOADER_H
#define MB_CONFIG_LOADER_H

#include "lsimpleobj.h"

class QDomDocument;
class QDomNode;
struct EmulValueSettings;


//MBConfigLoader
class MBConfigLoader : public LSimpleObject
{
    Q_OBJECT
public:
    MBConfigLoader(QObject *parent = NULL);
    virtual ~MBConfigLoader() {}

    QString name() const {return objectName();}
    inline void setConfig(const QString &f_name) {m_configName = f_name.trimmed();}
    inline QString currentConfig() const {return m_configName;}

    void tryLoadConfig(bool &ok);

protected:
    QString m_configName;

private:
    void loadConfig(const QDomDocument&);
    void loadDevice(const QDomNode&);
    void loadDeviceSignals(quint8, const QDomNode&);
    void loadDeviceSignal(quint8, const QDomNode&);
    void loadEmulParams(EmulValueSettings&, const QDomNode&);

};



#endif //MB_CONFIG_LOADER_H


