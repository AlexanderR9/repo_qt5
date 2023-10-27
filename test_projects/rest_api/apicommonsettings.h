#ifndef APICOMMONSETTINGS_H
#define APICOMMONSETTINGS_H

#include <QMap>
#include <QStringList>


class QDomNode;

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
        InstrumentHistory() :timeout(1000) {}
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
    };

    QString token;
    qint64 user_id;
    QStringList services;
    QMap<QString, QString> service_colors;
    QMap<QString, QString> candle_sizes;
    QMap<QString, QString> cycle_metods;
    AutoStartReq start_reqs;
    InstrumentHistory i_history;

    static QString appDataPath();
    static QString beginPoint(const InstrumentHistory::HItem&, quint8 hour = 7);
    static QString endPoint(const InstrumentHistory::HItem&, quint8 hour = 19);

    void reset();
    void loadConfig(QString&);
    void parseServicesNode(const QDomNode&);
    void parseCandlesNode(const QDomNode&);
    void parseAutoStartNode(const QDomNode&);
    void parseHistoryNode(const QDomNode&);
    void parseToken(QString);

};

extern API_CommonSettings api_commonSettings;



#endif // APICOMMONSETTINGS_H
