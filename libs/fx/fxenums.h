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



//набор статических функций для работы с этими множествами
class FXEnumStaticObj
{
public:
    static QList<int> timeFrames(); //список всех валидных значений периодов свечей

};


#endif //FXENUMS_H

