#include "fxenums.h"


QList<int> FXEnumStaticObj::timeFrames()
{
    QList<int> list;
    list << tf5m << tf15m << tf1h <<  tf4h << tf1d <<  tf1w;
    return list;
}
QList<int> FXEnumStaticObj::testTypes()
{
    QList<int> list;
    list << ttSpring << ttChannel << ttExtremum <<  ttPeriodShadows;
    return list;
}
QString FXEnumStaticObj::paramShortName(int param)
{
    switch (param)
    {
        //input
        case ipStartSum:        return QString("Start sum");
        case ipStopPips:        return QString("Stop pips");
        case ipProfitPips:      return QString("Profit pips");
        case ipDist:            return QString("Dist steps");
        case ipNBars:           return QString("N bars");
        case ipNPips:           return QString("N pips");
        case ipStartLot:        return QString("Start lot");
        case ipFixLot:          return QString("Fix lot");
        case ipNextLotFactor:   return QString("Lot factor");
        case ipNextLineFactor:  return QString("Line factor");
        case ipStopFactor:      return QString("Stop factor");
        case ipProfitFactor:    return QString("Profit factor");
        case ipSpreadPips:      return QString("Spread pips");


        //out
        case spSum:    return QString("Sum");
        case spMinSum:    return QString("Min sum");
        case spMaxSum:    return QString("Max sum");
        case spStep:    return QString("Step");
        case spMaxStep:    return QString("Max step");
        case spNumber:    return QString("Number");
        case spWinNumber:    return QString("Win number");
        case spCWinNumber:    return QString("CWin number");
        case spLossPips:    return QString("Loss pips");
        case spWinPips:    return QString("Win pips");
        case spLotSize:    return QString("Lot size");


        default: break;
    }
    return "??";
}
QString FXEnumStaticObj::testShortName(int t)
{
    switch (t)
    {
        case ttSpring:              return QString("Spring");
        case ttChannel:             return QString("Standard channel");
        case ttExtremum:            return QString("Find extremums");
        case ttPeriodShadows:       return QString("Take shadows");
        default: break;
    }
    return "?";
}
QString FXEnumStaticObj::testDesc(int t)
{
    switch (t)
    {
        case ttSpring:              return QString("каждый следующий шаг делается на расстоянии n_pips");
        case ttChannel:             return QString("to do");
        case ttExtremum:            return QString("to do");
        case ttPeriodShadows:       return QString("to do");
        default: break;
    }
    return "?";
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
        case ckIndex:   {digist = 0; break;}
        default: break;
    }
    return digist;
}
double FXEnumStaticObj::pipsPriceByCouple(const QString &name)
{
    double pp = -1;
    int kind = coupleKindByName(name);
    switch (kind)
    {
        case ckCurrency:    {pp = 1; break;}
        case ckStock:       {pp = 1; break;}
        case ckCrypto:      {pp = 1; break;}
        case ckIndex:       {pp = 1; break;}
        default: break;
    }
    return pp;
}
double FXEnumStaticObj::spreadByCouple(const QString &name)
{
    double pp = -1;
    int kind = coupleKindByName(name);
    switch (kind)
    {
        case ckCurrency:    {pp = 3; break;}
        case ckStock:       {pp = 3; break;}
        case ckCrypto:      {pp = 2; break;}
        case ckIndex:       {pp = 2; break;}
        default: break;
    }
    return pp;
}
double FXEnumStaticObj::marginFactorByCouple(const QString &name)
{
    double pp = -1;
    int kind = coupleKindByName(name);
    switch (kind)
    {
        case ckCurrency:    {pp = 1; break;}
        case ckStock:       {pp = 10; break;}
        case ckCrypto:      {pp = 1; break;}
        case ckIndex:       {pp = 1; break;}
        default: break;
    }
    return pp;
}


