#ifndef FXENUMS_H
#define FXENUMS_H

#include <QList>


//набор возможных периодов свечей
enum FXTimeFrame {
                    tf5m    = 5,
                    tf15m   = 15,
                    tf1h    = 60,
                    tf4h    = 240,
                    tf1d    = 1440,
                    tf1w    = 10080
                  };


//типы инструментов
enum FXCoupleKind {ckCurrency = 391, ckStock, ckCrypto, ckIndex};


// множество входных параметров для тестирования стратегии
enum FXInputParam {ipStopPips = 301, ipProfitPips, ipDist, ipNBars, ipNPips, ipTimeFrame, ipStartLot, ipFixLot, ipNextLotFactor, ipNextLineFactor,
                   ipStartSum, ipMondayTime, ipFridayTime, ipDecStep, exipIncStep, ipStopFactor, ipProfitFactor, ipSpreadPips};


// множество параметров текущего состояния тестирования (они же выходные)
enum FXStateParam {spStep = 401, spPrevStep, spMaxStep, spSum, spNumber, spWinNumber, spCWinNumber, spFullWinNumber, spWinPips,
                   spLossNumber, spFullLossNumber, spLossPips, spLotSize, spStopLossCount, spTakeProfitCount, spMinSum, spMaxSum,
                   spLineNumber};


// типы тестов (стратегий)
enum FXTestType {ttSpring = 501, ttChannel, ttExtremum, ttPeriodShadows};


//набор статических функций для работы с этими множествами
class FXEnumStaticObj
{
public:
    static QList<int> timeFrames(); //список всех валидных значений периодов свечей
    static QList<int> testTypes(); //список всех видов тестов
    static QStringList couplesAll(); //список типов всех возможных инструментов

    static bool invalidTimeframe(int); //вернет true если такой timeframe не найден в множестве FXTimeFrame
    static QString testShortName(int); //название теста (для интерфейса)
    static QString testDesc(int); //описание стратегии теста (для интерфейса)
    static QStringList couplesByKind(int); //список имен инструментов конкретного типа
    static int coupleKindByName(const QString&); //вернет тип инструмента из множества FXCoupleKind или -1, (регистр важен)
    static bool invalidCouple(const QString&); //вернет true если такое имя инструмента не надено, (регистр важен)
    static QString strTimeFrame(int); //строковое название заданного таймфрейма
    static QString paramShortName(int); //название входного/выходного параметра (для интерфейса)


    ///////////////////////////ГЛОБАЛЬНЫЕ НАСТРОЙКИ///////////////////////////////////
    //вернет точность (число знаков после запятой), которая определяет размер(разумный) пункта цен инструмента, при этом сама цена может иметь больше знаков
    //если инструмент некорректен то вернет 99
    static quint8 digistByCouple(const QString&);


    static double pipsPriceByCouple(const QString&); //вернет цену одного пункта при объеме 1 лот (центы),  или -1
    static double spreadByCouple(const QString&); //вернет spread для указанного инструмента в пунктах, или -1
    static double marginFactorByCouple(const QString&); //вернет коэфф. размера залога при объеме 1 лот (центы) от цены открытия,  или -1
    static double minLotSize() {return 0.1;} //минимальный размер лота
    static double lotStep() {return 0.01;} //шаг изменения лота


};


#endif //FXENUMS_H

