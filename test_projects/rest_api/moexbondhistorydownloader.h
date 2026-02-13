#ifndef MOEXBONDHISTORYDOWNLOADER_H
#define MOEXBONDHISTORYDOWNLOADER_H


#include "lsimpleobj.h"


class QNetworkAccessManager;
class QUrl;


//MoexBondHistoryDownloader
class MoexBondHistoryDownloader : public LSimpleObject
{
    Q_OBJECT
public:
    MoexBondHistoryDownloader(QObject *parent = NULL);
    virtual ~MoexBondHistoryDownloader() {}

    void run(int n_month = 6);

    QString name() const {return QString("history_downloader");}


    inline void setTicker(const QString& ticker) {m_ticker = ticker.trimmed();}
    inline void setPath(const QString& path) {m_path = path.trimmed();}
    inline QString ticker() const { return m_ticker; }
    inline QString path() const { return m_path; }


protected:
    QNetworkAccessManager *m_nam;
    QString m_ticker;
    QString m_path;

    void checkPathTicker(QString&);
    void prepareUrl(QUrl&, int);
    QString curTargetFile() const;
    void faultFinished(QString);


private slots:
    void slotReplyFinished();
    void slotReplyError();


signals:
    void finished(QString);

};





#endif // MOEXBONDHISTORYDOWNLOADER_H



