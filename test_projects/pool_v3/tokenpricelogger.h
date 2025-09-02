#ifndef TOKENPRICELOGGER_H
#define TOKENPRICELOGGER_H


#include "lsimpleobj.h"


//TokenPriceLogger
//объект для отслеживания цен токен и ведения журнала.
//объект один на все сети.
//при изменении цены более чем на 0,5% записывает запись в файл prices_history.log.
// записи в файле имеют вид: token_name / address / price / <datetime> в виде одного целого числа
//при инициализации загружает файл журнала prices_history.log и записывает поля DefiToke.last_price последними значениями.
class TokenPriceLogger : public LSimpleObject
{
    Q_OBJECT
public:
    struct TokenPriceRecord
    {
        TokenPriceRecord() {reset();}
        QString name;
        QString address;
        float price;
        quint32 time_point;
        void reset() {name.clear(); address.clear(); price=-1; time_point=0;}
    };

    TokenPriceLogger(QObject*);
    virtual ~TokenPriceLogger() {}

    void loadLogFile(); // загрузить при старте ПО всю историю в m_log

    float getLastPrice(QString, QString) const; // получить последнюю цену по имени токена и его адресу, в случае ошибки вернет -1
    void checkLastPrices(); // после выполнения http запроса получения цен необходимо выполнить эту функцию
    virtual QString name() const {return QString("tokenpriceslogger");}

    inline int logSize() const {return m_log.count();}
    inline bool logEmpty() const {return m_log.isEmpty();}


protected:
    QList<TokenPriceRecord> m_log;

    void addLastRecToFile(); // пришла новая цена, которая отличается от предыдущей более чем на 0,5%, ее нужно добавить в лог-файл

private:
    float priceDeviation(float, float) const;  //возвращает отклонение 2-й цены от 1-й в процентах

};




#endif // TOKENPRICELOGGER_H


