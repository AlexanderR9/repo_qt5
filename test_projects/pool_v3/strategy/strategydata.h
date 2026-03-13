#ifndef STRATEGYDATA_H
#define STRATEGYDATA_H

#include "lsimpleobj.h"

#include <QPair>
#include <QStringList>
#include <QList>

class QDomNode;
class QDomElement;
struct StrategyStepDialogData;


// параметры линии при открытии
struct StrategyLineParameters
{
    StrategyLineParameters() {reset();}

    // параметры для 1-го шага
    float liq_size; // вносимая ликвиднось (в приоритетном токене) на 1-м шаге (т.е. при открытии линии)
    quint8 first_token_index; // индекс актива который используется для внесения ликвидности на 1-м шаге (т.е. при открытии линии)

    // параметры для всех шагов (в том числе и 1-го)
    float range_width; // width ценового диапазона (в приоритетном токене)
    int prior_asset_size; // доля приоритетного токена (%) от общей вносимой ликвидности, распределение в долях пары токенов при окрытии поз

    void reset();
};

// данные по одному шагу линии
// ПРИМЕЧАНИЕ: все цены указываются для  приоритетного токена (т.е. привычные значения)
struct StrategyLineStepPrices
{
    StrategyLineStepPrices() {reset();}

    //vars
    float start_price; // цена приоритетного токена на момент открытия позы
    float exit_price; // цена приоритетного токена на момент закрытия позы (завершение шага)
    QPair<float, float> p_range; // ценовой диапазон позиции на этом шаге
    QPair<int, int> t_range; // тиковый диапазон позиции на этом шаге

    //funcs
    void reset();
    QString strPriceRange() const;

};
struct StrategyLineStepAmounts
{
    StrategyLineStepAmounts() {reset();}

    // ------------- vars -----------------
    //quint8 prior_index; //
    QPair<float, float> line_liq; // текущие полные объемы токенов линии перед открытием позы на этом шаге (после свопа, если он был)
    QPair<float, float> deposited; // объемы пары вносимых токенов в позу на этом шаге (реально внесенные во время минта)
    QPair<float, float> closed; // объемы пары токенов извлеченных на этом шаге (заполняется при закрытии позы)
    QPair<float, float> rewards; // полученные комиссии на этом шаге (заполняется при закрытии позы)

    // ------------- functions -----------------
    void reset();
    QString strAssetsSum(QString) const;
    float totalStepResult() const; // итоговый текущий/закрытый результат по шагу, учитывая rewards, %
    //float userTokenSum(const QPair<float, float>&, float) const;

};
struct StrategyLineStepState
{
    StrategyLineStepState() {reset();}

    // ------------- vars -----------------
    quint16 number; // номер шага 1..N
    quint32 ts_open; // время открытия шага, сек. эпохи линукс.
    quint32 ts_close; // время закрытия шага, сек. эпохи линукс. (если 0 то шаг еще не закрыт)
    quint32 pid; // ID of position


    StrategyLineStepPrices prices; // инфа по ценам (в приоритетном токене)
    StrategyLineStepAmounts amounts; // инфа по обьемам токенов на этом шаге


    // ------------- functions -----------------
    inline bool isOpened() const {return (ts_close == 0 && ts_open > 0);} // поза этого шага еще открыта

    void reset();
    void loadStepNode(const QDomNode&, bool &ok);
    void fillStepNode(QDomElement&) const;
    //void setPriorIndex(quint8);
    QStringList tableStepRowData() const; // возвращает готовую строку для таблицы по указанному шагу для отображения в интерфейсе пользователя
    QString strResult() const; // итоговый текущий/закрытый результат по шагу, учитывая rewards, %


};


// данные линии по конкретной комбинации strategy_type/pool_addr, она уникальна
struct StrategyLineData
{
    StrategyLineData() {reset();}

    // ------------- vars -----------------
    int strategy_type; // тип стратегии
    QString pool_addr;
    quint32 ts_open; // время открытия линии, сек. эпохи линукс.
    quint32 ts_close; // время закрытия линии, сек. эпохи линукс.

    StrategyLineParameters start_parameters;
    QList<StrategyLineStepState> steps; // 1-й шаг лежит с индексом 0


    // ------------- functions -----------------
    inline bool isFinished() const {return (ts_close > 0);}

    //load xml nodes
    void loadLineNode(const QDomNode&, bool &ok);
    void loadLineParameters(const QDomNode&, bool &ok);
    void loadLineSteps(const QDomNode&, bool &ok);

    //save xml nodes
    void fillLineNode(QDomElement&) const;
    void fillLineStepsNodes(QDomElement&) const;

    void reset();           
    void closeLastStep();
    void openNextStep(const StrategyStepDialogData&);
    bool lastStepOpened() const;
    QString toStr() const;
    quint8 pricePriorIndex() const;
    quint8 amoutPriorIndex() const;
    QStringList tableStepRowData(int) const; // возвращает готовую строку для таблицы по указанному шагу для отображения в интерфейсе пользователя
    void getCurrentLiqSize(QPair<float, float>&) const; // возвращает текущую ликвидность линии по обоим токена, но только при закрытом последнем шаге или вовсе без шагов

};



// контейнер для хранения всей инфы по текущим линиям со страницы стратегий
/*
    инфа хранится в файле xml(по каждой сети отдельный файл), для открытых линий всех возможных комбинация пул/стратегия.
    если по комбинации нет открытой линии, то здесь по ней ничего не будет.

    пример ноды-линии:
    <line pool="pool_addr" strategy="DefiStrategyType_enum" chain="chainName" ts_open="5465465"  >
        <settings liq_size="prior_token_size" range_width="prior_token_price_delta"  prior_token_part="30"(%) first_token_index="0" />


        <!-- steps sections -->
        <step number="1" ts_open="234234234" ts_close="234254234" pid="4561654">
            <prices>
                <position start="0.1145" exit="0.1213" />
                <p_range p1="0.0975" p2="0.1368" />
                <t_range tick1="-297600" tick2="-296040" />
            </prices>
            <amounts>
                <line_liq asset0="0.51" asset1="165.45" />      // before mint
                <deposited asset0="0.51" asset1="164.98" />     // nested amounts
                <closed asset0="0.0" asset1="265.45" />         // taken amounts
                <rewards asset0="0.0056" asset1="5.45" />       // taken amounts
            </amounts>
        </step>

        ........


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
    void openNextStep(int, const StrategyStepDialogData&);

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


