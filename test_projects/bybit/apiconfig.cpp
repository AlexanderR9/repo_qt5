#include "apiconfig.h"
#include "lfile.h"

#include <QDir>
#include <QDebug>



//global var
APIConfig api_config;

void APIConfig::reset()
{
    candle.reset();
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


