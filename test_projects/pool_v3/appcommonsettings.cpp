#include "appcommonsettings.h"
#include "lfile.h"

#include <QDir>


#define APP_DATA_FOLDER     QString("data")


QString AppCommonSettings::m_nodejsPath = QString("none");

quint8 AppCommonSettings::interfacePricePrecision(float p)
{
    if (p <= 0) return 1;
    if (p < 0.1) return 6;
    if (p < 2) return 4;
    return 2;
}
QString AppCommonSettings::appDataPath()
{
    return QString("%1%2%3").arg(LFile::appPath()).arg(QDir::separator()).arg(APP_DATA_FOLDER);
}
QString AppCommonSettings::tabPageTitle(int p_kind)
{
    switch (p_kind)
    {
        case dpkWallet: return QString("Wallet");
        case dpkApproved: return QString("Approved");
        case dpkBalance: return QString("Balance history");
        case dpkPool: return QString("Pool list");
        case dpkTx: return QString("TX log");
        case dpkPositions: return QString("Wallet positions");
        case dpkStatPositions: return QString("Positions stat.");
        case dpkStrategy: return QString("Strategy");

        default: break;
    }
    return "ERR??";
}

