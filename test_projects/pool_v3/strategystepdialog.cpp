#include "strategystepdialog.h"
#include "ltable.h"
#include "nodejsbridge.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "lstring.h"


#include <QDebug>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTimer>
#include <QTableWidget>
#include <QJsonObject>
#include <QJsonValue>


#define STAGE_TIMER_INTERVAL    1200
#define STAGE_TIMEOUT           15 // seconds
#define STAGE_NOTE_COL          3


//StrategyStepDialog
StrategyStepDialog::StrategyStepDialog(int a_type,  StrategyStepDialogData &data, QWidget *parent)
    :QDialog(parent),
      m_actionType(a_type),
      m_data(data),
      m_stage(-1),
      js_obj(NULL),
      m_stageManager(NULL)
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
    if (js_obj) stepParamsTable->item(row, 0)->setText(defi_config.shortPoolDescByAddr(m_data.pool_addr));
    else
    {
        stepParamsTable->item(row, 0)->setText("invalid pool");
        stepParamsTable->item(row, 0)->setTextColor(Qt::red);
    }
    row++;
    stepParamsTable->item(row, 0)->setText(QString("-")); row++;


    LTable::resizeTableContents(stepParamsTable);
}
void StrategyStepDialog::initTables()
{
    // params table
    QStringList headers;
    headers << "Next step" << "Pool" << "TVL, k" << "Current price" << "Current tick"  << "Price range" << "Tick range";
    headers << "Line amounts"  << "Wallet amounts" << "Prior size" << "Prior liq size" << "Swapping info";
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

    m_stageManager->run();

}
void StrategyStepDialog::slotStartStage(int stage)
{
    qDebug()<<QString("StrategyStepDialog::slotStartStage  %1").arg(stage);

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
    qDebug("StrategyStepDialog::slotStageFinished");
    result = result.trimmed().toUpper();
    int rows = jsProcessingTable->rowCount();
    int cols = jsProcessingTable->columnCount();
    jsProcessingTable->item(rows-1, cols-1)->setText(result);
    jsProcessingTable->item(rows-1, STAGE_NOTE_COL)->setText(m_stageManager->resultNote());


    if (result == "OK") jsProcessingTable->item(rows-1, cols-1)->setTextColor("#00aa00");
    else jsProcessingTable->item(rows-1, cols-1)->setTextColor("#A9A9A9");

    int pos = stageLabel->text().indexOf("/");
    QString sn = LString::strTrimLeft(stageLabel->text(), pos+1);
    stageLabel->setText(QString("Stages counter: %1/%2").arg(rows).arg(sn));

    if (result == "FAULT")
    {
        m_stageManager->stop(); // принудительный останов сценария
        if (js_obj->buzy()) js_obj->breakProcessing();
    }
}




/////////////StrategyStepDialogData//////////////////
void StrategyStepDialogData::reset()
{
    pool_addr.clear();
    line_liq.first = line_liq.second = 0;
    price_width = -1;
    prior_asset_size = 50;
    next_step = first_token_index = 0;
    wallet_assets_balance.first = wallet_assets_balance.second = 0;


    pool_tickers.first.clear();
    pool_tickers.second.clear();
    pool_token_addrs.first.clear();
    pool_token_addrs.second.clear();


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





//StrategyStageManagerObj
StrategyStageManagerObj::StrategyStageManagerObj(StrategyStepDialogData &data, QObject *parent)
    :LSimpleObject(parent),
      m_timer(NULL),
      m_data(data)
{
    setObjectName("stage_manager_obj");

    m_timer = new QTimer(this);
    m_timer->setInterval(STAGE_TIMER_INTERVAL);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTick()));

}
void StrategyStageManagerObj::run()
{
    if (m_actionStages.isEmpty()) return;

    m_stageState.reset();
    m_userSign = -1;
    m_timer->start();
}
QString StrategyStageManagerObj::captionByStage(int stage)
{
    switch (stage)
    {
        case sssGetPoolState: return QString("Getting pool state");
        case sssGetWalletTokenAmounts: return QString("Update wallet balances");
        case sssReadLineSettings: return QString("Read line settings");
        case sssDefineLineLiq: return QString("Define line liq size");

        case sssCalcSwapPart: return QString("Calc swap part");
        case sssCheckWalletTokenAmounts: return QString("Check wallet assets size");
        case sssTxSwap: return QString("TX swap");
        case sssGetTxSwapResult: return QString("Check TX-swap result");
        case sssGetPriceRangeNextPos: return QString("Define next price range");

        case sssTxMint: return QString("TX mint");
        case sssGetTxMintResult: return QString("Check TX-mint result");
        case sssGetPositionsList: return QString("Get wallet positions list");

        case sssGetStepPositionState: return QString("Getting position state");
        case sssSavePositionState: return QString("Save position state");
        case sssTxClosePosition: return QString("TX takeaway liq");
        case sssGetTxClosePosResult: return QString("Check TX-takeaway result");
        case sssCalcStepResult: return QString("Calc total result");


        default:    break;
    }
    return "?";
}
QString StrategyStageManagerObj::kindByStage(int stage)
{
    switch (stage)
    {
        case sssGetStepPositionState:
        case sssGetTxClosePosResult:
        case sssGetPoolState:
        case sssGetTxSwapResult:
        case sssGetTxMintResult:
        case sssGetPriceRangeNextPos:
        case sssGetPositionsList:
        case sssGetWalletTokenAmounts: return QString("ETHERS_READ");

        case sssTxClosePosition:
        case sssTxMint:
        case sssTxSwap: return QString("ETHERS_TX");

        case sssCalcSwapPart:
        case sssCalcStepResult:
        case sssSavePositionState:
        case sssCheckWalletTokenAmounts:
        case sssReadLineSettings:
        case sssDefineLineLiq: return QString("LOCAL_CALC");

        default:    break;
    }
    return "?";
}
void StrategyStageManagerObj::fillStagesList(int a_type)
{
    m_actionStages.clear();
    switch (a_type)
    {
        case ssaCloseStep:
        {
            m_actionStages << sssGetWalletTokenAmounts << sssGetStepPositionState << sssSavePositionState <<
                      sssTxClosePosition << sssGetTxClosePosResult << sssGetWalletTokenAmounts << sssCalcStepResult;
            break;
        }
        case ssaFirstStep:
        case ssaNextStep:
        {
            m_actionStages << sssGetWalletTokenAmounts << sssGetPoolState << sssReadLineSettings << sssDefineLineLiq <<
                  sssCalcSwapPart << sssCheckWalletTokenAmounts << sssTxSwap << sssGetTxSwapResult << sssGetPriceRangeNextPos <<
                  sssTxMint << sssGetTxMintResult << sssGetPositionsList << sssGetWalletTokenAmounts;
            break;
        }
        default:
        {
            qWarning()<<QString("StrategyStageManagerObj::fillStagesList WARNING invalid action %1").arg(a_type);
            break;
        }
    }

}
void StrategyStageManagerObj::slotTimerTick()
{
    qDebug("StrategyStageManagerObj::slotTimerTick()");
    if (m_actionStages.isEmpty() && m_userSign < 0)
    {
        m_timer->stop();
        emit signalFinishedAll();
        return;
    }

    checkStageState();
    if (isRunning()) {checkTimeout(); return;}

    if (m_userSign < 0)
    {
        m_userSign = m_actionStages.takeFirst();
        m_stageState.reset();
        m_stageState.ts_start = QDateTime::currentDateTime();
        emit signalStartStage(m_userSign);
    }
    else
    {
        m_userSign = -1;
        emit signalStageFinished(m_stageState.result);
    }
}
bool StrategyStageManagerObj::isRunning() const
{
    return (m_timer->isActive() && currentStage() > 0 && m_stageState.ts_start.isValid() && m_stageState.result.isEmpty());
}
void StrategyStageManagerObj::checkTimeout()
{
    int t_cur = QDateTime::currentDateTime().toSecsSinceEpoch();
    int t_diff = t_cur - m_stageState.ts_start.toSecsSinceEpoch();
    if (t_diff > STAGE_TIMEOUT)
    {
        m_userSign = -1;
        m_timer->stop();
        emit signalStageFinished("Timeout");
        emit signalFinishedAll();
    }
}
void StrategyStageManagerObj::checkStageState()
{


}
void StrategyStageManagerObj::stop()
{
    m_userSign = -1;
    m_timer->stop();
    emit signalStageFinished("Breaked");
    emit signalFinishedAll();
}
void StrategyStageManagerObj::execCurrentStage()
{
    if (m_userSign == sssGetWalletTokenAmounts)
    {
        getWalletTokenAmounts();
    }
}
void StrategyStageManagerObj::jsScriptRun()
{
    QStringList args;
    QString kind = StrategyStageManagerObj::kindByStage(currentStage());
    if (kind.trimmed().toLower().contains("_tx"))
    {
        qDebug("StrategyStageManagerObj::jsScriptRun()  WRITE");
        args << "tx_writer.js" << AppCommonSettings::txParamsNodeJSFile();
    }
    else
    {
        qDebug("StrategyStageManagerObj::jsScriptRun()  READ");
        args << "reader.js" << AppCommonSettings::readParamsNodeJSFile();
    }

    emit signalRunScriptArgs(args);
}
bool StrategyStageManagerObj::needToEthersReq() const
{
    QString kind = StrategyStageManagerObj::kindByStage(currentStage());
    return kind.trimmed().toLower().contains("ethers");
}
void StrategyStageManagerObj::setFaultResult(QString note)
{
    m_stageState.result = "fault";
    m_stageState.note = note;
}


// -----------------NodejsReply----------------------
void StrategyStageManagerObj::slotNodejsReply(const QJsonObject &js_reply)
{
    qDebug("StrategyStageManagerObj::slotNodejsReply OK!!!");
    //QStringList keys = js_reply.keys();

    if (m_userSign == sssGetWalletTokenAmounts) readWalletAssetsBalance(js_reply);

    //m_stageState.result = "ok";

}
void StrategyStageManagerObj::slotJSScriptFinished(int code)
{
    qDebug()<<QString("StrategyStageManagerObj::slotJSScriptFinished code=%1").arg(code);
    if (code != 0)
    {
        setFaultResult(QString("code = %1").arg(code));
    }
}
void StrategyStageManagerObj::readWalletAssetsBalance(const QJsonObject &js_reply)
{
    QStringList keys = js_reply.keys();
    if (keys.isEmpty()) {setFaultResult("js_reply is empty"); return;}

    QString t0 = m_data.pool_tickers.first;
    QString t1 = m_data.pool_tickers.second;
    if (!keys.contains(t0) || !keys.contains(t1)) {setFaultResult("not found tickers"); return;}

    m_data.wallet_assets_balance.first = js_reply.value(t0).toString().toFloat();
    m_data.wallet_assets_balance.second = js_reply.value(t1).toString().toFloat();
    m_stageState.note = QString("%1 / %2").arg(m_data.wallet_assets_balance.first).arg(m_data.wallet_assets_balance.second);
    m_stageState.result = "ok";

}




//private funcs
void StrategyStageManagerObj::getWalletTokenAmounts()
{
    if (needToEthersReq())
    {
        QJsonObject j_params;
        j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcBalance));
        emit signalRewriteJsonFile(j_params, AppCommonSettings::readParamsNodeJSFile()); //rewrite params json-file

        jsScriptRun();
    }
}

