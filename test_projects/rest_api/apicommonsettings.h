#ifndef APICOMMONSETTINGS_H
#define APICOMMONSETTINGS_H

#include <QMap>
#include <QStringList>


class QDomNode;
class QSettings;


//API_CommonSettings
struct API_CommonSettings
{
    API_CommonSettings() {reset();}

    struct AutoStartReq
    {
        AutoStartReq() {reset();}
        int timeout;
        QStringList src;
        void reset() {timeout = -1; src.clear();}
        bool invalid() const {return (timeout < 100 || src.isEmpty());}
    };
    struct InstrumentHistory
    {
        InstrumentHistory() :timeout(1000), block_size(50) {}
        struct HItem
        {
            HItem() {reset();}
            QString begin_date; //must be google format: example 2023-09-15
            QString end_date;
            void reset() {begin_date = end_date = QString();}
            void parseConfigNode(const QDomNode&);
            QString toStr() const {return QString("[%1 : %2]").arg(begin_date).arg(end_date);}
        };


        int timeout;
        HItem prices;
        HItem coupons;
        HItem divs;
        HItem events;
        quint16 block_size; //data block size (prices)
    };
    struct GlobalFilter
    {
        GlobalFilter() {}
        struct GFItem
        {
            GFItem() :country("all") {}
            QString country;
            void parseConfigNode(const QDomNode&);
        };

        GFItem bond;
        GFItem stock;
    };
    struct TradeDialog
    {
        TradeDialog() :deviation_index(-1), lots_index(-1) {}
        int deviation_index;
        int lots_index;
        void save(QSettings&);
        void load(QSettings&);
    };
    struct UidClone
    {
        UidClone() :uid(QString()), ticker(QString()), is_bond(false) {clones.clear();}
        QString uid;
        QString ticker;
        QStringList clones; //clones of uids
        bool is_bond;
        bool invalid() const {return (uid.isEmpty() || clones.isEmpty());}
        void parseXmlNode(const QDomNode&);
    };


    QString token;
    qint64 user_id;
    QStringList services;
    QMap<QString, QString> service_colors;
    QMap<QString, QString> candle_sizes;
    QMap<QString, QString> cycle_metods;
    AutoStartReq start_reqs;
    InstrumentHistory i_history;
    GlobalFilter g_filter;
    TradeDialog t_dialog;
    QList<UidClone> uid_clones;

    static QString appDataPath();
    static QString beginPoint(const InstrumentHistory::HItem&, quint8 hour = 7);
    static QString endPoint(const InstrumentHistory::HItem&, quint8 hour = 19);

    void reset();

    //load basic config
    void loadConfig(QString&);
    void parseServicesNode(const QDomNode&);
    void parseCandlesNode(const QDomNode&);
    void parseAutoStartNode(const QDomNode&);
    void parseHistoryNode(const QDomNode&);
    void parseGlobalFilterNode(const QDomNode&);
    void parseToken(QString);

    //load uid clones data
    void loadUidClones(QString&);
    void parseUidClones(const QDomNode&);

    bool isCloneUid(const QString&) const;
    QString getOrigUidByClone(const QString&) const;
    bool hasCloneUid(const QString&) const;
    QString getLastCloneUidByOrig(const QString&) const;
    bool isHasCloneBond(const QString&) const;

};

extern API_CommonSettings api_commonSettings;



#endif // APICOMMONSETTINGS_H
