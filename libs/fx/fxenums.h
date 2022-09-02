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
enum FXCoupleKind {ckCurrency = 355, ckStock, ckCrypto, ckIndex};


//набор статических функций для работы с этими множествами
class FXEnumStaticObj
{
public:
    static QList<int> timeFrames(); //список всех валидных значений периодов свечей
    static bool invalidTimeframe(int); //вернет true если такой timeframe не найден в множестве FXTimeFrame

    static QStringList couplesAll(); //список имен всех возможных инструментов
    static QStringList couplesByKind(int); //список имен инструментов конкретного типа
    static int coupleKindByName(const QString&); //вернет тип инструмента из множества FXCoupleKind или -1, (регистр важен)
    static bool invalidCouple(const QString&); //вернет true если такое имя инструмента не надено, (регистр важен)

    //вернет точность (число знаков после запятой), которая определяет размер(разумный) пункта цен инструмента, при этом сама цена может иметь больше знаков
    //если инструмент некорректен то вернет 99
    static quint8 digistByCouple(const QString&);

};


#endif //FXENUMS_H

