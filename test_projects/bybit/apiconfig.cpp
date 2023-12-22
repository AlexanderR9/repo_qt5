#include "apiconfig.h"
#include "lfile.h"

#include <QDir>
#include <QChar>
#include <QDebug>
#include <QCryptographicHash>



//global var
APIConfig api_config;

void APIConfig::reset()
{
    candle.reset();
    req_delay=3000;
}
QString APIConfig::appDataPath()
{
    return QString("%1%2data").arg(LFile::appPath()).arg(QDir::separator());
}
void APIConfig::loadTickers()
{
    QString fname(QString("%1%2tickers.txt").arg(appDataPath()).arg(QDir::separator()));
    QString err = LFile::readFileSL(fname, tickers);
    if (!err.isEmpty()) qWarning()<<QString("APIConfig::loadTickers() WARNING - %1").arg(err);
}
void APIConfig::setApiKeys(QString key1, QString key2)
{
    int h = key1.length()/2;
    QString api_key = QString("%1%2").arg(key1.right(key1.length()-h)).arg(key1.left(h));

    api_key_private.clear();
    int n = key2.length();
    for (int i=0; i<n; i++)
    {
        QChar c(key2.at(i));
        if (c.isDigit())
        {
            quint8 a = key2.mid(i, 1).toUInt();
            a++;
            if (a > 9) a = 0;
            c = QString::number(a).at(0);
        }
        api_key_private.append(c);
    }

    QString s_left = api_key_private.left(4);
    QString s_right = api_key_private.right(4);
    api_key_private = api_key_private.right(n-4);
    api_key_private = api_key_private.left(api_key_private.length()-4);
    api_key_private.prepend(s_right);
    api_key_private.append(s_left);
}
QByteArray APIConfig::calcHMACSha256(QByteArray key, QByteArray baseString)
{
    QCryptographicHash::Algorithm metod = QCryptographicHash::Sha256;
    int blockSize = 64; // HMAC-SHA-1 block size, defined in SHA-1 standard
    if (key.length() > blockSize) // if key is longer than block size (64), reduce key length with SHA-1 compression
    {
        qDebug("key.length() > blockSize");
        key = QCryptographicHash::hash(key, metod);
    }

    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "quot;
    for (int i = 0; i < key.length(); i++)
    {
        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
    }

     QByteArray total = outerPadding;
     QByteArray part = innerPadding;
     part.append(baseString);
     total.append(QCryptographicHash::hash(part, metod));
     return QCryptographicHash::hash(total, metod);
}


