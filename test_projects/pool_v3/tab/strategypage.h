#ifndef STRATEGYPAGE_H
#define STRATEGYPAGE_H


#include "basetabpage_v3.h"

class QComboBox;
class QWidget;
class DefiStrategyData;
class QLineEdit;
class QFrame;
class QToolButton;
struct StrategyLineData;


// страница для управления линией игры различных стратегий.
// кнопками управления можно стартовать новую линию / остановить линию / закрыть текущую позу / перейти к следующему шагу.
// можно открыть только одную линию по указанному пулу для заданной стратегии.
// закрыть линию можно только если она открыта в текущий момент, но при этом поза на текущем шаге закрыта, т.е. состояние - между шагами.
// перед стартом любой линии настраиваются параметры (соответствующей стратегии) и более они не меняются, пока линия не будет закрыта.
// после открытия линии далее идет управление шагами, для перехода к следующему шагу, закрываем текущую позу, затем открываем новую (т.е. 2 итерации)
// расчеты убытков/прибыли считаются в приоритетном токене.
// цены и объемы ликвидности указываются в приоритетном токене.
//
// 1. dstFollowPrice
// поза переставляется следуя за ценой, ликвидность устанавливается только на 1-м шаге,
// далее при изменении диапазона вся ликвидность и накопленные реварды перераспределяются согласно новому диапазону и текущей цене.
// новая ликвидность не добавляется, т.е. весь риск это ставка на 1-м шаге при открытии линии.
// размер диапазона т.е. дельта p2-p1 фиксирована (именно дельта).
// поля настроек:
//   - сумма приоритетного токена (вносимая ликвиднось)
//   - размер диапазона цены приоритеного токена в единицах 2-го
//   - доля приоритетного токена(%) от  общей ликвидности позы в приритетном токене при текущей цене
// алгоритм выполнения очередного шага:
//  - посылается запрос в сеть через js_node для определения текущей цены
//  - обновить текущие балансы кошелька
//  - собирается инфа о текущих объемах обоих активов всей результирующей ликвидности позы (включая реварды) на данном шаге.
//  - вычисляется доля одного из токенов которую нужно свапнуть на другой чтобы выполнилось условие 3-й настройки.
//  - производится рельный своп этой доли в пуле, с которым работаем.
//  - проверяется статус выполненного свопа.
//  - вычисляем p2 / p1 так что бы выполнилось условие настроек, а так же чтобы доли токенов как раз подошли для открытия позы на этом шаге.
//  - открываем новую позицию
//  - проверяется статус выполненной операции.
// алгоритм закрытия текущего шага:
//  - закрывается открытая поза
//  - проверяется статус выполненной операции.
//  - обновить текущие балансы кошелька
//  - производится расчет текущего состояния линии на предмет размера ликвидности а также прироста/убытка



//DefiStrategyPage
class DefiStrategyPage : public BaseTabPage_V3
{
    Q_OBJECT
public:
    DefiStrategyPage(QWidget*);
    virtual ~DefiStrategyPage() {}

    //выполняется когда пользователь в тулбаре нажимает кнопку "Update"
    void sendUpdateDataRequest() {}

    virtual void setChain(int);

protected:
    QComboBox *m_strategyCombo;
    QComboBox *m_poolCombo;
    QLineEdit *m_startTimeEdit;
    QLineEdit *m_lineResultEdit;
    QMap<QString, QWidget*> m_controls;
    DefiStrategyData *m_dataObj;


    void initPageBoxes();
    void initDataObj();
    void initControlButtons(QFrame*);
    int curStrategy() const;
    QString curPool() const;
    bool curStrategyStable() const;
    void updateControlButtonsState(int line_index = -1);
    void controlButtonsDisable();
    void restoreStartParamsByLine(const StrategyLineData*); // восстановить на форме значения настроек линии при переключении пулов (если линия открыта)
    void resetStartParamsControls(); // сбросить на форме настройки линии

public slots:
    void slotNodejsReply(const QJsonObject&) {} //получен успешный ответ от скрипта nodejs


private:
    QToolButton* startLineBtn() const;
    QToolButton* stopLineBtn() const;
    QToolButton* closeStepBtn() const;
    QToolButton* nextStepBtn() const;

    float liqSize() const; // полный объем вносимой ликвидности в позу (в приоритетном токене)
    float rangeWidth() const; // ширина ценового диапазона
    quint16 priorTokenPart() const; // доля приоритетного токена от общей ликвидности, %



protected slots:    
    void slotUpdateComboPools();
    void slotPoolChanged();

    void slotStartLine();
    void slotStopLine();
    void slotCloseStep();
    void slotNextStep();



};





#endif // STRATEGYPAGE_H







