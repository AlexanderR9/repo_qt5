#ifndef STRATEGYDATA_H
#define STRATEGYDATA_H

#include "lsimpleobj.h"

#include <QPair>
#include <QStringList>
#include <QList>

class QDomNode;
class QDomElement;


// параметры линии при открытии
struct StrategyLineParameters
{
    StrategyLineParameters() {reset();}

    float liq_size; // вносимая ликвиднось (в приоритетном токене)
    float range_width; // width ценового диапазона (в приоритетном токене)
    int prior_asset_size; // доля приоритетного токена (%) от общей вносимой ликвидности

    void reset();
};

// данные по одному шагу линии
struct StrategyLineStepPrices
{
    StrategyLineStepPrices() {reset();}

    quint8 prior_index;
    float start_price; // цена приоритетного токена на момент открытия позы
    float exit_price; // цена приоритетного токена на момент закрытия позы (завершение шага)
    QPair<float, float> p_range; // ценовой диапазон (в ценах приоритетного токена)
    QPair<int, int> t_range; // тиковый диапазон

    void reset();
};
struct StrategyLineStepAmounts
{
    StrategyLineStepAmounts() {reset();}

    quint8 prior_index;
    QPair<float, float> deposited; // объемы пары вносимых токенов в позу на этом шаге
    QPair<float, float> closed; // объемы пары токенов извлеченных на этом шаге (заполняется при закрытии позы)
    QPair<float, float> rewards; // полученные комиссии на этом шаге (заполняется при закрытии позы)

    void reset();
};
struct StrategyLineStepState
{
    StrategyLineStepState() {reset();}

    quint16 number; // номер шага 1..N
    quint32 ts_open; // время открытия шага, сек. эпохи линукс.
    quint32 ts_close; // время закрытия шага, сек. эпохи линукс. (если 0 то шаг еще не закрыт)

    StrategyLineStepPrices prices; // инфа по ценам (в приоритетном токене)
    StrategyLineStepAmounts amounts; // инфа по обьемам токенов на этом шаге

    inline bool isOpened() const {return (ts_close == 0 && ts_open > 0);} // поза этого шага еще открыта

    void reset();
    void loadStepNode(const QDomNode&, bool &ok);
    void fillStepNode(QDomElement&) const;
    void setPriorIndex(quint8, quint8);

};


// данные линии по конкретной комбинации strategy_type/pool_addr, она уникальна
struct StrategyLineData
{
    StrategyLineData() {reset();}

    int strategy_type; // тип стратегии
    QString pool_addr;
    quint32 ts_open; // время открытия линии, сек. эпохи линукс.
    quint32 ts_close; // время закрытия линии, сек. эпохи линукс.


    StrategyLineParameters start_parameters;
    QList<StrategyLineStepState> steps; // 1-й шаг лежит с индексом 0

    //load xml nodes
    void loadLineNode(const QDomNode&, bool &ok);
    void loadLineParameters(const QDomNode&, bool &ok);
    void loadLineSteps(const QDomNode&, bool &ok);

    //save xml nodes
    void fillLineNode(QDomElement&) const;
    void fillLineStepsNodes(QDomElement&) const;

    void reset();           
    void closeLastStep();
    void openNextStep();
    bool lastStepOpened() const;
    QString toStr() const;
    quint8 pricePriorIndex() const;
    quint8 amoutPriorIndex() const;


    inline bool isFinished() const {return (ts_close > 0);}

};



// контейнер для хранения всей инфы по текущим линиям со страницы стратегий
/*
    инфа хранится в файле xml,  открытых линиях всех возможных комбинация пул/стратегия.
    если по комбинации нет открытой линии, то здесь по ней ничего не будет.

    пример ноды-линии:
    <line pool="pool_addr" strategy="DefiStrategyType_enum" chain="chainName" ts_open="5465465"  >
        <settings liq_size="prior_token_size" range_width="prior_token_price_delta"  prior_token_part="30"(%)  />

        <step number="1" ts_open="234234234" ts_close="0" >
            <price start="178.9" exit="-1" />
            <range p1="130.65" p2="230.16" tick1="-283748" tick2="-263781" />
            <amounts>
                <deposited asset0="0.51" asset1="165.45" />
                <closed asset0="0.0" asset1="265.45" />
                <rewards asset0="0.0056" asset1="5.45" />
            </amounts>
        </step>

    </line>
  */
class DefiStrategyData : public LSimpleObject
{
    Q_OBJECT
public:
    DefiStrategyData(QString, QObject*);
    virtual ~DefiStrategyData() {}

    virtual QString name() const {return QString("defi_strategy_data");}
    void loadStrategyFile(bool&); // загрузить файл состояния текущих линий
    void startLine(const StrategyLineData&); // начать новую линию
    void finishLine(int); // завершить (удалить из контейнера) линии с указанным индексом
    void closeLastStep(int);
    void openNextStep(int);

    inline int lineCount() const {return m_lines.count();}
    inline bool linesEmpty() const {return m_lines.isEmpty();}

    bool hasOpenedLine(int, const QString&) const; // признак что есть открытая линия для указанной комбинации стратегия/пул
    const StrategyLineData* lineAt(int) const; // получить объект из контейнера по индексу, если нет вернет -1
    int lineIndexOf(int, const QString&) const; // индекс линии в контейнере для указанной комбинации стратегия/пул, если нет вернет -1


    static quint32 curTsPoint(); // текущие дата и время в формате quint32, т.е. кол. сек. с начала эпохи линукс
    static QString fromTsPointToStr(quint32);

protected:
    QString m_chain; // chain name
    QList<StrategyLineData> m_lines;

    void loadLineNode(const QDomNode&);
    void rewriteStrategyFile(); // перезаписать файл состояния текущих линий

};



#endif // STRATEGYDATA_H


