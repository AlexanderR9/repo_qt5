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
        QString begin_date;
        QString end_date;
        QString beginPoint() const {return QString("%1T%2Z").arg(begin_date).arg(QString("07:00:00"));}
        QString endPoint() const {return QString("%1T%2Z").arg(end_date).arg(QString("19:00:00"));}
    };

    QString token;
    qint64 user_id;
    QStringList services;
    QMap<QString, QString> service_colors;
    QMap<QString, QString> candle_sizes;
    AutoStartReq start_reqs;
    InstrumentHistory i_history;

    static QString appDataPath();

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
