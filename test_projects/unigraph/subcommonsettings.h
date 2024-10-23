#ifndef SUB_COMMONSETTINGS_H
#define SUB_COMMONSETTINGS_H

#include <QMap>
#include <QStringList>


class QDomNode;
class QSettings;


//SubGraph_CommonSettings
struct SubGraph_CommonSettings
{
    SubGraph_CommonSettings() {reset();}

    struct SGFactory
    {
        SGFactory() {reset();}
        QString sub_id;
        QString chain; //blockchain name
        QString swap_place; //resource (site)
        void reset() {sub_id.clear(); chain.clear(); swap_place.clear();}
        bool invalid() const {return (sub_id.isEmpty() || chain.isEmpty());}
    };



    QList<SGFactory> factories;
    int cur_factory;

    static QString appDataPath();

    void reset();
    void loadConfig(QString&);
    void parseFactoriesNode(const QDomNode&);
    void setCurFactory(QString sub_id);
    QString curChain() const; //cur chain name by cur_factory or ?

    QStringList factoryTitles() const;
    bool invalid() const {return (factories.isEmpty());}

};

extern SubGraph_CommonSettings sub_commonSettings;



#endif // SUB_COMMONSETTINGS_H
