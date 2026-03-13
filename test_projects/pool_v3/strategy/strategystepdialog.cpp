#include "strategystepdialog.h"
#include "strategystepdialogstruct.h"
#include "strategystagemanager.h"
#include "strategytxlogmaker.h"
#include "ltable.h"
#include "nodejsbridge.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "lstring.h"
#include "txlogrecord.h"


#include <QDebug>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTableWidget>
#include <QJsonObject>
#include <QJsonValue>


#define STAGE_NOTE_COL          3


//StrategyStepDialog
StrategyStepDialog::StrategyStepDialog(int a_type,  StrategyStepDialogData &data, QWidget *parent)
    :QDialog(parent),
      m_actionType(a_type),
      m_data(data),
      m_stage(-1),
      js_obj(NULL),
      m_stageManager(NULL),
      m_txLogMaker(NULL)
{
    setupUi(this);
    setObjectName(QString("next_step_dialog"));
    //setWindowTitle(QString("Mint position (%1)").arg(m_data.chain_name.trimmed().toUpper()));
    resize(1200, 600);
    setModal(true);


    m_data.out();
    initActionEdit();
    initTables();
    initJsObj();
    initManagerObj();
    fillParamsTable();

    // start
    startScenario();

}
StrategyStepDialog::~StrategyStepDialog()
{
    if (js_obj)
    {
        delete js_obj;
        js_obj = NULL;
    }
}
void StrategyStepDialog::initJsObj()
{

    definePoolAssets();

    m_data.out();

    if (m_data.invalid()) return;

    int i_pool = defi_config.getPoolIndex(m_data.pool_addr);
    if (i_pool < 0) return;

    int cid = defi_config.pools.at(i_pool).chain_id;
    js_obj = new NodejsBridge(this, cid);


}
void StrategyStepDialog::initManagerObj()
{
    //StrategyStageManagerObj
    m_stageManager = new StrategyStageManagerObj(m_data, this);
    m_stageManager->fillStagesList(m_actionType);

    int n_stages = m_stageManager->stagesCount();
    stageLabel->setText(QString("Stages counter: 0/%1").arg(n_stages));
    if (n_stages == 0)
    {
        stepParamsTable->setEnabled(false);
        jsProcessingTable->setEnabled(false);
        return;
    }

    //LogMaker
    m_txLogMaker = new StrategyTxLogMaker(m_data, this);

    connect(m_stageManager, SIGNAL(signalTxWasDone(const QJsonObject&)), m_txLogMaker, SLOT(slotTxWasDone(const QJsonObject&)));
    connect(m_txLogMaker, SIGNAL(signalStrategyTx(const TxLogRecord&)), this, SIGNAL(signalStrategyTx(const TxLogRecord&)));
    connect(m_txLogMaker, SIGNAL(signalStrategyTxStatus(const QMap<QString, QString>&)), this, SIGNAL(signalStrategyTxStatus(const QMap<QString, QString>&)));
    connect(m_stageManager, SIGNAL(signalDelayAfterTx(int)), this, SLOT(slotDelayAfterTx(int)));
    connect(m_stageManager, SIGNAL(signalTxStatusDone(const QJsonObject&)), m_txLogMaker, SLOT(slotTxStatusDone(const QJsonObject&)));

}
void StrategyStepDialog::definePoolAssets()
{
    m_data.pool_tickers.first = defi_config.token0NameByPoolAddr(m_data.pool_addr);
    m_data.pool_tickers.second = defi_config.token1NameByPoolAddr(m_data.pool_addr);

    int i_pool = defi_config.getPoolIndex(m_data.pool_addr);
    if (i_pool >= 0)
    {
        m_data.pool_token_addrs.first = defi_config.pools.at(i_pool).token0_addr;
        m_data.pool_token_addrs.second = defi_config.pools.at(i_pool).token1_addr;
        m_data.prior_amount_i = defi_config.getPriorAmountIndexByPoolAddr(m_data.pool_addr);
        m_data.prior_price_i = defi_config.getPriorPriceIndexByPoolAddr(m_data.pool_addr);
        m_data.pool_fee = defi_config.pools.at(i_pool).fee;
    }

    if (m_data.invalid())
    {
        stepParamsTable->item(1, 0)->setText("invalid line data");
        stepParamsTable->item(1, 0)->setTextColor(Qt::red);
        stepParamsTable->setEnabled(false);
        jsProcessingTable->setEnabled(false);
    }

}
void StrategyStepDialog::fillParamsTable()
{
    int row = 0;    
    stepParamsTable->item(row, 0)->setText(QString::number(m_data.next_step)); row++;
    for (int i=row; i<stepParamsTable->rowCount(); i++)
        stepParamsTable->item(i, 0)->setText(QString("-")); row++;

    if (js_obj)
    {
        stepParamsTable->item(1, 0)->setText(defi_config.shortPoolDescByAddr(m_data.pool_addr));
        stepParamsTable->item(5, 0)->setText(QString::number(m_data.price_width));
        stepParamsTable->item(9, 0)->setText(QString("%1 %").arg(m_data.prior_asset_size));
        stepParamsTable->item(11, 0)->setText(QString("%1 / %2").arg(m_data.prior_amount_i).arg(m_data.prior_price_i));
        updateLineAmountsCell(7);
    }
    else
    {
        stepParamsTable->item(row, 0)->setText("invalid pool");
        stepParamsTable->item(row, 0)->setTextColor(Qt::red);
    }
    row++;



    LTable::resizeTableContents(stepParamsTable);
}
void StrategyStepDialog::initTables()
{
    // params table
    QStringList headers;
    headers << "Next step" << "Pool" << "TVL, k" << "Current price" << "Current tick"  << "Price range" << "Tick range";
    headers << "Line amounts"  << "Wallet amounts" << "Prior liq part, %" << "Swapping info" << "Prior indexes (amount/price)";
    headers << "After swap result" << "Line amounts (after swap)" << "Next position amounts" << "After mint result";
    LTable::fullClearTable(stepParamsTable);
    LTable::setTableHeaders(stepParamsTable,headers, Qt::Vertical);
    headers.clear();
    headers << "Value";
    LTable::setTableHeaders(stepParamsTable, headers, Qt::Horizontal);
    LTable::resizeTableContents(stepParamsTable);
    LTable::createAllItems(stepParamsTable);
    stepParamsTable->setSelectionMode(QAbstractItemView::NoSelection);

    // processing table
    headers.clear();
    headers << "Stage" << "Name" << "Kind" << "Note" << "State";
    LTable::fullClearTable(jsProcessingTable);
    LTable::setTableHeaders(jsProcessingTable, headers, Qt::Horizontal);
    LTable::resizeTableContents(jsProcessingTable);
    jsProcessingTable->setSelectionMode(QAbstractItemView::NoSelection);

}
void StrategyStepDialog::updateTableAfterStage()
{
    qDebug()<<QString("StrategyStepDialog::updateTableAfterStage stage=%1").arg(m_stage);

    int rows = jsProcessingTable->rowCount();
    if (m_stage == sssGetWalletBalances || m_stage == sssGetWalletBalancesAfterSwap)
    {
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());
        stepParamsTable->item(8, 0)->setText(m_stageManager->resultNote());
        if (m_stage == sssGetWalletBalancesAfterSwap)
        {
            stepParamsTable->item(12, 0)->setText(m_data.afterSwapResult());
            updateLineAmountsCell(13);
        }
        if (m_data.mintDone())
        {
            stepParamsTable->item(15, 0)->setText(m_data.afterMintResult());
        }
    }
    else if (m_stage == sssGetPoolState)
    {
        QStringList list = LString::trimSplitList(m_stageManager->resultNote(), "/");
        if (list.count() < 2) jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());
        else
        {
            foreach (const QString &v, list)
            {
                int pos = v.indexOf("=");
                if (v.contains("tvl"))
                {
                    float tvl = LString::strTrimLeft(v, pos+1).toFloat();
                    stepParamsTable->item(2, 0)->setText(QString::number(tvl/1000, 'f', 1));
                }
                else if (v.contains("price"))
                {
                    float p = LString::strTrimLeft(v, pos+1).toFloat();
                    quint8 prec = AppCommonSettings::interfacePricePrecision(p);
                    stepParamsTable->item(3, 0)->setText(QString::number(p, 'f', prec));
                    jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(v);
                }
                else if (v.contains("tick")) stepParamsTable->item(4, 0)->setText(LString::strTrimLeft(v, pos+1));
            }
        }
    }
    else if (m_stage == sssReadLineSettings)
    {
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());
        updateLineAmountsCell(7);
    }
    else if (m_stage == sssCalcSwapPart)
    {
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());
        updateSwapInfoCell();
    }
    else if (m_stage == sssGetPriceRangeNextPos)
    {
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());
        updateRangeInfoCell();
    }
    else  if (m_stage == sssCheckWalletTokenAmounts)
    {
        stepParamsTable->item(8, 0)->setTextColor(Qt::darkGreen);
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());
    }
    else  if (m_stage == sssTxSwap || m_stage == sssGetTxSwapResult || m_stage == sssTxMint || m_stage == sssGetTxMintResult)
    {
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setTextColor(Qt::darkBlue);
    }
    else  if (m_stage == sssGetPositionsList)
    {
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());
    }

    LTable::resizeTableContents(stepParamsTable);

}
void StrategyStepDialog::initActionEdit()
{
    actionLineEdit->clear();
    switch (m_actionType)
    {
        case ssaCloseStep:
        {
            actionLineEdit->setText("CLOSING ...");
            actionLineEdit->setStyleSheet("color: #FF4500;");
            break;
        }
        case ssaFirstStep:
        {
            actionLineEdit->setText("FIRST STEP ...");
            actionLineEdit->setStyleSheet("color: #0000bb;");
            break;
        }
        case ssaNextStep:
        {
            actionLineEdit->setText("NEXT STEP ...");
            actionLineEdit->setStyleSheet("color: #4682B4;");
            break;
        }
        default:
        {
            actionLineEdit->setText("error");
            actionLineEdit->setStyleSheet("color: #aa0000;");
            stepParamsTable->setEnabled(false);
            jsProcessingTable->setEnabled(false);
            break;
        }
    }
}
void StrategyStepDialog::startScenario()
{
    if (!js_obj || !m_stageManager) return;

    buttonBox->button(QDialogButtonBox::Close)->setEnabled(false);
    LTable::removeAllRowsTable(jsProcessingTable);
    LTable::resizeTableContents(jsProcessingTable);

    connect(m_stageManager, SIGNAL(signalStartStage(int)), this, SLOT(slotStartStage(int)));
    connect(m_stageManager, SIGNAL(signalFinishedAll()), this, SLOT(slotFinishedAll()));
    connect(m_stageManager, SIGNAL(signalStageFinished(QString)), this, SLOT(slotStageFinished(QString)));
    connect(m_stageManager, SIGNAL(signalRunScriptArgs(const QStringList&)), js_obj, SLOT(slotRunScriptArgs(const QStringList&)));
    connect(js_obj, SIGNAL(signalNodejsReply(const QJsonObject&)), m_stageManager, SLOT(slotNodejsReply(const QJsonObject&)));
    //connect(js_bridge, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    //connect(js_bridge, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(js_obj, SIGNAL(signalFinished(int)), m_stageManager, SLOT(slotJSScriptFinished(int)));
    connect(m_stageManager, SIGNAL(signalRewriteJsonFile(const QJsonObject&, QString)), this, SIGNAL(signalRewriteJsonFile(const QJsonObject&, QString)));
    connect(m_stageManager, SIGNAL(signalAddGasPriceField(QJsonObject&)), this, SIGNAL(signalAddGasPriceField(QJsonObject&)));

    m_stageManager->run();

}
void StrategyStepDialog::slotStartStage(int stage)
{
    qDebug()<<QString("StrategyStepDialog::slotStartStage  %1").arg(stage);
    m_stage = stage;

    int rows = jsProcessingTable->rowCount();
    int cols = jsProcessingTable->columnCount();
    QStringList row_data;
    row_data << QString::number(rows+1) << StrategyStageManagerObj::captionByStage(stage) <<
                StrategyStageManagerObj::kindByStage(stage) << "---" << "executing ....";

    LTable::addTableRow(jsProcessingTable, row_data);
    LTable::resizeTableContents(jsProcessingTable);
    jsProcessingTable->item(rows, cols-1)->setTextColor("#BDB76B");

    m_stageManager->execCurrentStage();

}
void StrategyStepDialog::slotFinishedAll()
{
    qDebug("StrategyStepDialog::slotFinishedAll");
    buttonBox->button(QDialogButtonBox::Close)->setEnabled(true);

}
void StrategyStepDialog::slotStageFinished(QString result)
{
    qDebug()<<QString("StrategyStepDialog::slotStageFinished stage=%1").arg(m_stageManager->currentStage());

    result = result.trimmed().toUpper();
    int rows = jsProcessingTable->rowCount();
    int cols = jsProcessingTable->columnCount();
    jsProcessingTable->item(rows-1, cols-1)->setText(result);
    //jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());


    if (result == "OK")
    {
        jsProcessingTable->item(rows-1, cols-1)->setTextColor("#00aa00");
        updateTableAfterStage();
    }
    else if (result == "FAULT")
    {
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setTextColor("#B22222");
    }
    else jsProcessingTable->item(rows-1, cols-1)->setTextColor("#A9A9A9");

    int pos = stageLabel->text().indexOf("/");
    QString sn = LString::strTrimLeft(stageLabel->text(), pos+1);
    stageLabel->setText(QString("Stages counter: %1/%2").arg(rows).arg(sn));
    LTable::resizeTableContents(jsProcessingTable);


    if (result == "FAULT")
    {
        m_stageManager->stop(); // принудительный останов сценария
        if (js_obj->buzy()) js_obj->breakProcessing();
    }
}
void StrategyStepDialog::slotDelayAfterTx(int x)
{
    int rows = jsProcessingTable->rowCount();
    if (x > 0)
    {
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(QString("delay %1 sec ....").arg(x));
        LTable::setTableRowColor(jsProcessingTable, rows-1, "#DDFF99");
    }
    else
    {
        jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(QString("delay OFF"));
        LTable::setTableRowColor(jsProcessingTable, rows-1, "#FFFFFF");
    }
}



// private funcs of StrategyStepDialog
void StrategyStepDialog::updateLineAmountsCell(int row)
{
    QString liq0 = QString::number(m_data.line_liq.first, 'f', AppCommonSettings::interfacePricePrecision(m_data.line_liq.first));
    QString liq1 = QString::number(m_data.line_liq.second, 'f', AppCommonSettings::interfacePricePrecision(m_data.line_liq.second));
    stepParamsTable->item(row, 0)->setText(QString("%1 / %2").arg(liq0).arg(liq1));
}
void StrategyStepDialog::updateSwapInfoCell()
{
    QString swap_info = "none";
    if (m_data.swap_info.first > 0 &&  m_data.swap_info.second < 0)
    {
        QString s0 = QString::number(m_data.swap_info.first, 'f', AppCommonSettings::interfacePricePrecision(m_data.swap_info.first));
        QString s1 = QString::number(-1*m_data.swap_info.second, 'f', AppCommonSettings::interfacePricePrecision(-1*m_data.swap_info.second));
        swap_info = QString("%1%2 => %3%4").arg(s1).arg(m_data.pool_tickers.second).arg(s0).arg(m_data.pool_tickers.first);
    }
    if (m_data.swap_info.first < 0 &&  m_data.swap_info.second > 0)
    {
        QString s0 = QString::number(-1*m_data.swap_info.first, 'f', AppCommonSettings::interfacePricePrecision(-1*m_data.swap_info.first));
        QString s1 = QString::number(m_data.swap_info.second, 'f', AppCommonSettings::interfacePricePrecision(m_data.swap_info.second));
        swap_info = QString("%1%2 => %3%4").arg(s0).arg(m_data.pool_tickers.first).arg(s1).arg(m_data.pool_tickers.second);
    }

    stepParamsTable->item(10, 0)->setText(swap_info);
}
void StrategyStepDialog::updateRangeInfoCell()
{
    QString ticks = QString("[%1 : %2]").arg(m_data.pos_range_tick.first).arg(m_data.pos_range_tick.second);
    stepParamsTable->item(6, 0)->setText(ticks);

    QString p1 = QString::number(m_data.pos_range.first, 'f', AppCommonSettings::interfacePricePrecision(m_data.pos_range.first));
    QString p2 = QString::number(m_data.pos_range.second, 'f', AppCommonSettings::interfacePricePrecision(m_data.pos_range.second));
    stepParamsTable->item(5, 0)->setText(QString("[%1 : %2]").arg(p1).arg(p2));

    QString a0 = QString::number(m_data.real_mint_amounts.first, 'f', AppCommonSettings::interfacePricePrecision(m_data.real_mint_amounts.first));
    QString a1 = QString::number(m_data.real_mint_amounts.second, 'f', AppCommonSettings::interfacePricePrecision(m_data.real_mint_amounts.second));
    stepParamsTable->item(14, 0)->setText(QString("%1 / %2").arg(a0).arg(a1));

}




/*

/////////////StrategyStepDialogData//////////////////
void StrategyStepDialogData::reset()
{
    pool_addr.clear();
    line_liq.first = line_liq.second = 0;
    swap_info.first = swap_info.second = 0;
    price_width = -1;
    prior_asset_size = 50;
    next_step = first_token_index = 0;
    wallet_assets_balance.first = wallet_assets_balance.second = 0;
    prior_amount_i = prior_price_i = 0;
    start_line_liq = -1;


    pool_tickers.first.clear();
    pool_tickers.second.clear();
    pool_token_addrs.first.clear();
    pool_token_addrs.second.clear();
    pool_price = -1;
    pool_tick = 0;

    none_swap = true;
    tx_swap_hash.clear();
}
bool StrategyStepDialogData::invalid() const
{
    if (price_width <= 0 || next_step < 1) return true;
    if (pool_tickers.first.length() < 2 || pool_tickers.second.length() < 2) return true;
    if (pool_tickers.first.contains("?") || pool_tickers.second.contains("?")) return true;
    if (pool_token_addrs.first.length() < 20 || pool_token_addrs.second.length() < 20) return true;
    return false;
}
void StrategyStepDialogData::out()
{
    qDebug("-----------StrategyStepDialogData-----------");
    qDebug()<<QString("pool_addr: %1").arg(pool_addr);
    qDebug()<<QString("price_width: %1").arg(price_width);
    qDebug()<<QString("prior_asset_size: %1").arg(prior_asset_size);
    qDebug()<<QString("first_token_index: %1").arg(first_token_index);
    qDebug()<<QString("line_liq: %1 / %2").arg(line_liq.first).arg(line_liq.second);

    qDebug()<<QString("pool_tickers: %1 / %2").arg(pool_tickers.first).arg(pool_tickers.second);
    qDebug()<<QString("pool_token_addrs: %1 / %2").arg(pool_token_addrs.first).arg(pool_token_addrs.second);

}

*/


