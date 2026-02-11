#include "moexbondhistorydownloader.h"
#include "lfile.h"
#include "ltime.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>



#include <QDebug>

#define SOURCE_SERV         QString("https://iss.moex.com")
#define URL_PATH            QString("iss/engines/stock/markets/bonds/securities")
#define URL_PATH_BOARD      QString("iss/engines/stock/markets/bonds/boards/TQOB/securities")




// MoexBondHistoryDownloader
MoexBondHistoryDownloader::MoexBondHistoryDownloader(QObject *parent)
    :LSimpleObject(parent),
      m_nam(NULL),
      m_ticker(QString()),
      m_path(QString())
{
    setObjectName("history_downloader_obj");

    m_nam = new QNetworkAccessManager(this);

}
void MoexBondHistoryDownloader::checkPathTicker(QString &err)
{
    err.clear();
    if (m_ticker.length() < 5)
    {
        err = QString("Invalid ticker value [%1]").arg(m_ticker);
        return;
    }
    if (m_path.isEmpty())
    {
        err = QString("Target folder is empty");
        return;
    }
    if (!LFile::dirExists(m_path))
    {
        err = QString("Target folder [%1] not found").arg(m_path);
        return;
    }
}
QString MoexBondHistoryDownloader::curTargetFile() const
{
    return QString("%1%2%3.txt").arg(m_path).arg(QDir::separator()).arg(m_ticker);
}
void MoexBondHistoryDownloader::run(int n_month)
{
    QString err;
    checkPathTicker(err);
    if (!err.isEmpty()) {faultFinished(err); return;}

    if (n_month <= 0) n_month = 1;
    const QString outFile = curTargetFile();
    emit signalMsg(QString("%0  try download file [%1] .....").arg(LTime::strCurrentTime()).arg(outFile));

    // ✅ НОВОЕ ПОВЕДЕНИЕ: файл уже есть
    if (QFile::exists(outFile))
    {
        faultFinished(QString("history for %1 already exist").arg(m_ticker));
        return;
    }

    QUrl url;
    prepareUrl(url, n_month);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "QtApp/1.0 (MoexBondHistoryDownloader)");
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkReply *nam_reply = m_nam->get(req);
    connect(nam_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotReplyError()));
    connect(nam_reply, SIGNAL(finished()), this, SLOT(slotReplyFinished()));

}
void MoexBondHistoryDownloader::prepareUrl(QUrl &url, int n_month)
{
    url.setUrl(QString("%1/%2/%3/candles.csv").arg(SOURCE_SERV).arg(URL_PATH).arg(m_ticker));

    const QDate till = QDate::currentDate();
    const QDate from = till.addMonths(-1*n_month);

    QString s_range = QString("HISTORY RANGE: [%1 : %2]").arg(from.toString("yyyy-MM-dd")).arg(till.toString("yyyy-MM-dd"));
   // qDebug()<<s_range;
    emit signalMsg(s_range);

    QUrlQuery q;
    q.addQueryItem("interval", "24");
    q.addQueryItem("from", from.toString("yyyy-MM-dd"));
    q.addQueryItem("till", till.toString("yyyy-MM-dd"));
    url.setQuery(q);
}
void MoexBondHistoryDownloader::slotReplyFinished()
{
    QNetworkReply *rpl = qobject_cast<QNetworkReply*>(sender());
    if (!rpl) {qWarning("MoexBondHistoryDownloader::slotReplyFinished WARNING reply is NULL"); return;}

    const auto err_code = rpl->error();
    //const QString s_err = rpl->errorString();
    const QByteArray data = rpl->readAll();
    const QString outFile = curTargetFile();

    rpl->abort();
    rpl->deleteLater();
    rpl = NULL;

    if (err_code != QNetworkReply::NoError)
    {
        //emit signalError(QString("Network error: [%0]  code=%1").arg(s_err).arg(err_code));
        faultFinished("MOEX download failed");
        return;
    }
    if (data.isEmpty()) {faultFinished("MOEX returned empty response"); return;}

    QFile f(outFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        faultFinished(QString("Can't write file: %1").arg(outFile));
        return;
    }

    f.write(data);
    f.close();

    emit finished(LFile::shortFileName(outFile));
}
void MoexBondHistoryDownloader::slotReplyError()
{
    qWarning("MoexBondHistoryDownloader  ERROR ");
    QNetworkReply *rpl = qobject_cast<QNetworkReply*>(sender());
    if (!rpl) {qWarning("MoexBondHistoryDownloader::slotReplyError WARNING reply is NULL"); return;}

    faultFinished(QString("Network error: %1, code=%2").arg(rpl->errorString()).arg(rpl->error()));
}
void MoexBondHistoryDownloader::faultFinished(QString err)
{
    emit signalError(err);
    emit finished(QString());
}




