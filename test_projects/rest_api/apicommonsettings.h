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

    QString token;
    qint64 user_id;
    QStringList services;
    QMap<QString, QString> service_colors;
    QMap<QString, QString> candle_sizes;
    AutoStartReq start_reqs;

    static QString appDataPath();

    void reset();
    void loadConfig(QString&);
    void parseServicesNode(const QDomNode&);
    void parseCandlesNode(const QDomNode&);
    void parseAutoStartNode(const QDomNode&);
    void parseToken(QString);

};

extern API_CommonSettings api_commonSettings;



#endif // APICOMMONSETTINGS_H
