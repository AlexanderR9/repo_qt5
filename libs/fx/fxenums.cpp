#include "fxenums.h"


QList<int> FXEnumStaticObj::timeFrames()
{
    QList<int> list;
    list << tf5m << tf15m << tf1h <<  tf4h << tf1d <<  tf1w;
    return list;
}
QString FXEnumStaticObj::strTimeFrame(int tf)
{
    switch (tf)
    {
        case tf5m:      return QString("M5");
        case tf15m:     return QString("M15");
        case tf1h:      return QString("H1");
        case tf4h:      return QString("H4");
        case tf1d:      return QString("D1");
        case tf1w:      return QString("W1");
        default: break;
    }
    return "?";
}
bool FXEnumStaticObj::invalidTimeframe(int tf)
{
    return (!timeFrames().contains(tf));
}
QStringList FXEnumStaticObj::couplesAll()
{
    QStringList list;
    list.append(couplesByKind(ckCurrency));
    list.append(couplesByKind(ckStock));
    list.append(couplesByKind(ckCrypto));
    list.append(couplesByKind(ckIndex));
    return list;
}
QStringList FXEnumStaticObj::couplesByKind(int kind)
{
    QStringList list;
    switch (kind)
    {
        case ckCurrency:
        {
            list << "EURUSD" << "EURGBP" << "EURCHF" << "EURJPY" << "EURCAD" << "EURAUD" << "EURNZD";
            list << "GBPUSD" << "GBPCHF" << "GBPJPY" << "GBPCAD" << "GBPAUD" << "GBPNZD";
            list << "USDCHF" << "USDJPY" << "USDCAD";
            list << "AUDUSD" << "AUDCHF" << "AUDJPY" << "AUDCAD" << "AUDNZD";
            list << "NZDUSD" << "NZDCHF" << "NZDJPY" << "NZDCAD";
            break;
        }
        case ckStock:
        {
            list << "AA" << "AAPL" << "AIG" << "AEM" << "ABBV" << "BAC" << "BK" << "BMY" << "CAT" << "CVX" << "C" << "CSCO";
            list << "EOG" << "EBAY" << "FDX" << "F" << "HPQ" << "MMM" << "MCD" << "MRK" << "MGM" << "JNJ";
            list << "KO" << "KHC" << "IBM" << "INTC" << "IP" << "NEM" << "PFE" << "PG" << "PEP";
            list << "WMT" << "WFC" << "WDC" << "T" << "SBUX" << "S";
            break;
        }
        case ckCrypto:
        {
            list << "ethereum" << "bitcoin" << "litecoin";
            break;
        }
        case ckIndex:
        {
            list << "SPX" << "NDX" << "DAX" << "INDU";
            break;
        }
        default: break;
    }
    return list;
}
int FXEnumStaticObj::coupleKindByName(const QString &name)
{
    if (couplesByKind(ckCurrency).contains(name)) return ckCurrency;
    if (couplesByKind(ckStock).contains(name)) return ckStock;
    if (couplesByKind(ckCrypto).contains(name)) return ckCrypto;
    if (couplesByKind(ckIndex).contains(name)) return ckIndex;
    return -1;
}
bool FXEnumStaticObj::invalidCouple(const QString &name)
{
    return (!couplesAll().contains(name));
}
quint8 FXEnumStaticObj::digistByCouple(const QString &name)
{
    quint8 digist = 99;
    int kind = coupleKindByName(name);
    switch (kind)
    {
        case ckCurrency:
        {
            if (name.right(3) == "JPY") digist = 2;
            else digist = 4;
            break;
        }
        case ckStock:   {digist = 2; break;}
        case ckCrypto:  {digist = 0; break;}
        case ckIndex:   {digist = 1; break;}
        default: break;
    }
    return digist;
}





