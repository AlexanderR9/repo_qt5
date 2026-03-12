#ifndef STRATEGYSTEPDIALOG_STRUCT_H
#define STRATEGYSTEPDIALOG_STRUCT_H


#include <QPair>
#include <QDebug>


// интерфейс для взаимодействия основного виджета с диалогом StrategyStepDialog
// хранит промежуточные данные по линии, а так же после выполнения действия в нее записываются новые.
struct StrategyStepDialogData
{
    StrategyStepDialogData(QString chain) :cur_chain(chain.trimmed()) {reset();}

    QString cur_chain; // название сети в которой это все выполняется

    // ------------ текущие параметры и настройки линии ---------------
    QString pool_addr;
    QPair<float, float> line_liq; // текущая ликвидность линии
    float price_width; // ширина диапазона
    int prior_asset_size; // доля приоритетного токена (%) от общей вносимой ликвидности
    quint8 next_step; // номер шага к которому переходим на этом этапе

    // настройки используемые на 1-м шаге
    quint8 first_token_index; // индекс токена из пары пула, который будет использоваться для внесения ликвидности при открытии линии
    float start_line_liq; // вложенная ликвидность при открытии линии (в приоритетном токене)

    // ---------------- параметры которые определяются при старте диалога ------------------------
    QPair<QString, QString> pool_tickers; // определяется по pool_addr
    QPair<QString, QString> pool_token_addrs; // определяется по pool_addr
    quint16 pool_fee; // 100(0.01%) /  500(0.05%) / 3000(0.3%) / 10000 (1.0%)
    quint8 prior_amount_i; // определяется по pool_addr приоритетный индекс актива для расчетов количества
    quint8 prior_price_i; // определяется по pool_addr  приоритетный индекс актива для отображения цен

    // --------------------------расчетные параметры--------------------------------
    //данные полученные из сети в процессе выполнения сценария
    QPair<float, float> wallet_assets_balance; // текущие балансы пары токенов пула в кошельке перед началом выполнения сценария
    int pool_tick; // текущий тик в пуле
    float pool_price; // текущая цена в пуле (приоритетного токена) т.е. привычная для пользователя

    // данны о предстоящем свопе на текущем шаге,
    // рассчитывается после получения текущего состояния пула,
    // то значение, которое отрицательное нужно отдать для обмена,
    // то значение которое положительное, предполагается получить после обмена.
    // если значение нулевые, то значит обмен не нужен.
    QPair<float, float> swap_info;
    bool none_swap; // признак что своп не требуется
    QString tx_swap_hash; // заполняется если была транзакция swap
    QPair<float, float> wallet_balances_after_swap; // текущие балансы пары токенов пула в кошельке после успешного свопа
    bool swap_done; // признак что в сценарии производился своп и при этом был успешно выполнен

    // mint data
    QPair<float, float> pos_range; // ценовые границы диапазона следующей позы (в нормальных единицах т.е. для приоритетного токена)
    QPair<int, int> pos_range_tick; // тиковые границы диапазона следующей позы
    QPair<float, float> real_mint_amounts; // реальные расчетные объемы токенов для открытия сл. позы, они подходят по тиковый диапазон pos_range_tick
    QString tx_mint_hash; // заполняется если была транзакция mint
    quint32 mint_pid; // PID новой позы, если > 0 признак что в сценарии производился mint и при этом был успешно выполнен
    QPair<float, float> wallet_balances_after_mint; // текущие балансы пары токенов пула в кошельке после успешного MINT


    // после выполнения каждой транзакции необходимо выдержать паузу
    // перед проверкой статуса.
    // код -55 означает что требуется запустить задержку.
    // код 0 означает что задержка прошла и можно проверять статус.
    // код > 0  означает что задержка сейчас идет.
    int delay_after_tx;

    /////////////funcs//////////////////

    QString afterSwapResult() const // показывает дельты, насколько убавились/прибавились объемы пары токенов пула после свопа
    {
        float s0 = wallet_balances_after_swap.first - wallet_assets_balance.first;
        float s1 = wallet_balances_after_swap.second - wallet_assets_balance.second;
        QString ss0 = ((s0 > 0) ? QString("+%1").arg(QString::number(s0, 'f', 4)) : QString::number(s0, 'f', 4));
        QString ss1 = ((s1 > 0) ? QString("+%1").arg(QString::number(s1, 'f', 4)) : QString::number(s1, 'f', 4));
        return QString("%1 / %2").arg(s0).arg(s1);
    }
    QString afterMintResult() const // показывает дельты, насколько убавились объемы пары токенов пула после минта
    {
        float s0 = wallet_balances_after_mint.first - wallet_balances_after_swap.first;
        float s1 = wallet_balances_after_mint.second - wallet_balances_after_swap.second;
        if (none_swap)
        {
            s0 = wallet_balances_after_mint.first - wallet_assets_balance.first;
            s1 = wallet_balances_after_mint.second - wallet_assets_balance.second;
        }
        return QString("%1 / %2").arg(QString::number(s0, 'f', 5)).arg(QString::number(s1, 'f', 5));
    }

    bool needDelay() const {return (delay_after_tx == -55);}
    bool delayRunning() const {return (delay_after_tx > 0);}
    bool mintDone() const {return (mint_pid > 0);}



    void reset()
    {
        pool_addr.clear();
        line_liq.first = line_liq.second = 0;
        swap_info.first = swap_info.second = 0;
        price_width = -1;
        prior_asset_size = 50;
        next_step = first_token_index = 0;
        wallet_assets_balance.first = wallet_assets_balance.second = 0;
        wallet_balances_after_swap.first = wallet_balances_after_swap.second = 0;
        wallet_balances_after_mint.first = wallet_balances_after_mint.second = 0;

        prior_amount_i = prior_price_i = 0;
        start_line_liq = -1;

        pool_tickers.first.clear();
        pool_tickers.second.clear();
        pool_token_addrs.first.clear();
        pool_token_addrs.second.clear();
        pool_price = -1;
        pool_tick = pool_fee = 0;

        none_swap = true;
        swap_done = false;
        tx_swap_hash.clear();
        delay_after_tx = -1;

        pos_range.first = pos_range.second = -1;
        pos_range_tick.first = pos_range_tick.second = 0;
        real_mint_amounts.first = real_mint_amounts.second = 0;
        tx_mint_hash.clear();
        mint_pid = 0;

    }
    bool invalid() const
    {
        if (price_width <= 0 || next_step < 1) return true;
        if (pool_tickers.first.length() < 2 || pool_tickers.second.length() < 2) return true;
        if (pool_tickers.first.contains("?") || pool_tickers.second.contains("?")) return true;
        if (pool_token_addrs.first.length() < 20 || pool_token_addrs.second.length() < 20) return true;
        return false;
    }
    void out()
    {
        qDebug("-----------StrategyStepDialogData-----------");
        qDebug()<<QString("pool_addr: %1").arg(pool_addr);
        qDebug()<<QString("price_width: %1").arg(price_width);
        qDebug()<<QString("prior_asset_size: %1").arg(prior_asset_size);
        qDebug()<<QString("first_token_index: %1").arg(first_token_index);
        qDebug()<<QString("line_liq: %1 / %2").arg(line_liq.first).arg(line_liq.second);
        qDebug()<<QString("pool_tickers: %1 / %2").arg(pool_tickers.first).arg(pool_tickers.second);
        qDebug()<<QString("pool_token_addrs: %1 / %2").arg(pool_token_addrs.first).arg(pool_token_addrs.second);
        qDebug()<<QString("prior_index amount/price: %1 / %2").arg(prior_amount_i).arg(prior_price_i);
        qDebug()<<QString("wallet_balances_begin: %1 / %2").arg(wallet_assets_balance.first).arg(wallet_assets_balance.second);

        qDebug()<<QString("IMPLEMENTATION_SWAP:");
        qDebug()<<QString("swap_info: %1 / %2").arg(swap_info.first).arg(swap_info.second);
        qDebug()<<QString("none_swap: %1").arg(none_swap?"TRUE":"FALSE");
        qDebug()<<QString("tx_swap_hash: %1").arg(tx_swap_hash);
        qDebug()<<QString("wallet_balances_after_swap: %1 / %2").arg(wallet_balances_after_swap.first).arg(wallet_balances_after_swap.second);
        qDebug()<<QString("swap_done: %1").arg(swap_done?"YES":"NO");
        if (swap_done) qDebug()<<QString("afterSwapResult: %1").arg(afterSwapResult());

        qDebug()<<QString("IMPLEMENTATION_MINT:");
        qDebug()<<QString("price_range: [%1 : %2]").arg(pos_range.first).arg(pos_range.second);
        qDebug()<<QString("tick_range: [%1 : %2]").arg(pos_range_tick.first).arg(pos_range_tick.second);
        qDebug()<<QString("real_mint_amounts: %1 / %2").arg(real_mint_amounts.first).arg(real_mint_amounts.second);
        qDebug()<<QString("tx_mint_hash: %1").arg(tx_mint_hash);
        qDebug()<<QString("Minted PID: %1").arg(mint_pid);
        qDebug()<<QString("wallet_balances_after_mint: %1 / %2").arg(wallet_balances_after_mint.first).arg(wallet_balances_after_mint.second);
        if (mintDone()) qDebug()<<QString("afterMintResult: %1").arg(afterMintResult());

    }


};





#endif // STRATEGYSTEPDIALOG_STRUCT_H



