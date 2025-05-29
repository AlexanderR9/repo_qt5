#include "jspooltab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"
#include "jstxdialog.h"
#include "ethers_js.h"
#include "jstxlogger.h"


#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>

#define JS_POOL_FILE          "pools.txt"
//#define JS_JSONPARAMS_FILE    "params.json"
#define FEE_COL                 3
#define POOL_ADDR_COL           1
#define POOL_STATE_COL          5
#define TVL_COL                 6
#define NOTE_COL                7



// JSPoolTab
JSPoolTab::JSPoolTab(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_table(NULL)
{
    setObjectName("js_pools_tab");

    initTable();
    initPopupMenu();
}
void JSPoolTab::initPopupMenu()
{
    //prepare menu actions data
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Get state", QString("%1/r_scale.svg").arg(SubGraph_CommonSettings::commonIconsPath()));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Swap assets", TxDialogBase::iconByTXType(txSwap));
    act_list.append(pair2);
    QPair<QString, QString> pair3("Mint position", TxDialogBase::iconByTXType(txMint));
    act_list.append(pair3);

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetPoolState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTrySwapAssets())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotMintPosition())); i_menu++;

}
void JSPoolTab::slotGetPoolState()
{
   // qDebug("JSPoolTab::slotGetPoolState()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    //prepare json params
    QJsonObject j_params;
    j_params.insert("address", m_poolData.at(row).address);
    j_params.insert("fee", m_poolData.at(row).fee);
    j_params.insert("token0", m_poolData.at(row).token0_addr);
    j_params.insert("token1", m_poolData.at(row).token1_addr);
    j_params.insert("tick_space", m_poolData.at(row).tickSpace());    
    emit signalRewriteParamJson(j_params); //rewrite json-file

    m_table->table()->item(row, NOTE_COL)->setText("getting state ...");
    m_table->table()->item(row, NOTE_COL)->setTextColor("#DC143C");
    m_table->resizeByContents();


    //send action
    emit signalMsg(QString("Try get pool[%1] state ...").arg(m_poolData.at(row).address));
    m_table->table()->setEnabled(false);
    QStringList tx_params;
    tx_params << "qt_pool_state.js" << EthersPage::inputParamsJsonFile();
    emit signalPoolAction(tx_params);
}
void JSPoolTab::slotMintPosition()
{
    qDebug("JSPoolTab::slotMinPosition()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    QTableWidget *t = m_table->table();
    if (t->item(row, POOL_STATE_COL)->text() == "?") {emit signalError("You must get pool state"); return;}
    QString cur_state = t->item(row, POOL_STATE_COL)->text();
    QString cur_tvl = t->item(row, TVL_COL)->text();

    emit signalMsg(QString("Try mint new position, POOL(%1) ...").arg(m_poolData.at(row).address));


    //prepare dialog params
    TxDialogData mint_data(txMint);
    mint_data.token_addr = m_poolData.at(row).address;
    mint_data.token_name = QString("%1  %2").arg(m_poolData.at(row).assets).arg(m_poolData.at(row).strFee());
    mint_data.dialog_params.insert("state", cur_state);
    mint_data.dialog_params.insert("tvl", cur_tvl);
    mint_data.dialog_params.insert("note", t->item(row, NOTE_COL)->text());

    float cur_approved = 0;
    QString s_approved = "?";
    emit signalGetApprovedSize("pos_manager", m_poolData.at(row).token0_addr, cur_approved);
    s_approved = QString::number(cur_approved, 'f', 2);
    emit signalGetApprovedSize("pos_manager", m_poolData.at(row).token1_addr, cur_approved);
    s_approved = QString("%1 / %2").arg(s_approved).arg(QString::number(cur_approved, 'f', 2));
    mint_data.dialog_params.insert("approved", s_approved);

    TxMintPositionDialog d(mint_data, this);
    d.exec();
    if (d.isApply()) sendMintTx(mint_data, row);
    else emit signalMsg("operation was canceled!");
}
void JSPoolTab::slotTrySwapAssets()
{
   // qDebug("JSPoolTab::slotTrySwapAssets()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    //prepare dialog params
    TxDialogData swap_data(txSwap);
    swap_data.token_addr = m_poolData.at(row).address;
    swap_data.token_name = QString("%1  %2").arg(m_poolData.at(row).assets).arg(m_poolData.at(row).strFee());
    TxSwapDialog d(swap_data, this);
    d.exec();
    if (d.isApply())
    {
        emit signalMsg(QString("Try send TX swap"));
        int t_input = swap_data.dialog_params.value("input_token").toInt();
        emit signalMsg(QString("PARAMS: pool=%1, amount=%2, input_token=%3").arg(swap_data.token_name).arg(swap_data.dialog_params.value("amount")).arg(t_input));
        if (swap_data.dialog_params.value("amount") == "invalid") {signalError("invalid amount value"); return;}
        if (swap_data.dialog_params.value("dead_line") == "invalid") {signalError("invalid dead_line value"); return;}


        if (swap_data.dialog_params.value("is_simulate") == "no") //need check approved size
        {
            bool ok;
            float amount_in = swap_data.dialog_params.value("amount").toFloat(&ok);
            if (!ok || amount_in < 0.01) {signalError(QString("invalid amount value [%1]").arg(swap_data.dialog_params.value("amount"))); return;}

            float cur_approved = -1;
            if (t_input == 0) emit signalGetApprovedSize("swap_router", m_poolData.at(row).token0_addr, cur_approved);
            else  emit signalGetApprovedSize("swap_router", m_poolData.at(row).token1_addr, cur_approved);
            if (amount_in > cur_approved)
            {
                signalError(QString("swapping amount[%1] > approved amount[%2]").arg(amount_in).arg(cur_approved));
                return;
            }

            if (t_input == 0) emit signalResetApproved(m_poolData.at(row).token0_addr);
            else if (t_input == 1) emit signalResetApproved(m_poolData.at(row).token1_addr);
        }


        //prepare json params
        QJsonObject j_params;
        j_params.insert("pool_address", swap_data.token_addr);
        j_params.insert("input_token", t_input);
        j_params.insert("dead_line", swap_data.dialog_params.value("dead_line"));
        j_params.insert("size", swap_data.dialog_params.value("amount"));
        j_params.insert("simulate_mode", swap_data.dialog_params.value("is_simulate"));
        emit signalRewriteParamJson(j_params); //rewrite json-file
        //return;

        m_table->table()->item(row, NOTE_COL)->setText("try swap ...");
        m_table->table()->item(row, NOTE_COL)->setTextColor("#DC143C");
        m_table->resizeByContents();


        m_table->table()->setEnabled(false);
        QStringList tx_params;
        tx_params << "qt_pool_swap.js" << EthersPage::inputParamsJsonFile();
        emit signalPoolAction(tx_params);
    }
    else emit signalMsg("operation was canceled!");
}
void JSPoolTab::initTable()
{
    m_table = new LSearchTableWidgetBox(this);
    m_table->setTitle("Pools list");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Chain" << "Pool address" << "Assets" << "Fee" << "Tick space" << "Current state" << "TVL" << "Note";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#87CEEB");

    v_splitter->addWidget(m_table);
}
void JSPoolTab::loadPoolsFromFile(QString chain_name)
{
 //   qDebug("-----------------JSPoolTab::loadPoolsFromFile()--------------------------");
    m_poolData.clear();
    QString fname = QString("%1%2%3").arg(sub_commonSettings.nodejs_path).arg(QDir::separator()).arg(JS_POOL_FILE);
    emit signalMsg(QString("try load pools list [%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("POOL file not found");
        return;
    }
    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}

    chain_name = chain_name.trimmed().toUpper();
    foreach (const QString &v, fdata)
    {
        QString fline = v.trimmed();
        if (fline.isEmpty()) continue;
        if (fline.left(1) == "#") continue;

        JSPoolRecord rec;
        rec.fromFileLine(fline);
        if (!rec.invalid())
        {
            if (rec.chain == chain_name)
                m_poolData.append(rec);
        }
    }
    emit signalMsg(QString("loaded %1 POOL records").arg(m_poolData.count()));
  //  qDebug()<<QString("loaded %1 POOL records").arg(m_poolData.count());

    reloadTable();
}
void JSPoolTab::reloadTable()
{
    m_table->removeAllRows();
    if (m_poolData.isEmpty()) {m_table->searchExec(); return;}

    QTableWidget *t = m_table->table();
    QStringList row_data;
    for (int i=0; i<m_poolData.count(); i++)
    {
        row_data.clear();
        const JSPoolRecord &rec = m_poolData.at(i);
        row_data << rec.chain << rec.address << rec.assets << rec.strFee();
        row_data << QString::number(rec.tickSpace()) << "?" << "-1" << "---";
        LTable::addTableRow(t, row_data);

        if (rec.fee == 100) t->item(i, FEE_COL)->setTextColor(Qt::gray);
        else if (rec.fee == 500) t->item(i, FEE_COL)->setTextColor(Qt::darkGreen);
        else if (rec.fee == 3000) t->item(i, FEE_COL)->setTextColor(Qt::blue);
        else t->item(i, FEE_COL)->setTextColor(Qt::red);

        t->item(i, POOL_ADDR_COL)->setTextColor("#4682B4");
    }

    m_table->searchExec();
    m_table->resizeByContents();
   // qDebug()<<QString("JSPoolTab: POOL table rows %1").arg(t->rowCount());
}
void JSPoolTab::parseJSResult(const QJsonObject &j_result)
{
    m_table->table()->setEnabled(true);

    QString operation = j_result.value("type").toString().trimmed();
    if (operation == "state") answerState(j_result);
    else if (operation == "tx_swap") answerSwap(j_result);
    else if (operation == "tx_mint") answerMintTx(j_result);
    else emit signalError(QString("invalid answer type: %1").arg(operation));

    m_table->resizeByContents();
}
void JSPoolTab::answerState(const QJsonObject &j_result)
{
    if (m_poolData.isEmpty()) return;

   // qDebug("JSPoolTab::answerState");
    int n = m_poolData.count();
    for(int i=0; i<n; i++)
    {
        if (j_result.value("pool_address").toString() == m_poolData.at(i).address)
        {
            bool stb = m_poolData.at(i).isStablePool();
          //  qDebug()<<QString("find row, %1, pool: %2").arg(i).arg(m_poolData.at(i).address);
            int  tick = j_result.value("tick").toString().toInt();
            float p0 = j_result.value("price0").toString().toFloat();
            float p1 = j_result.value("price1").toString().toFloat();
            QString str_state = QString("tick=%1").arg(tick);
            str_state = QString("%1  price0=%2").arg(str_state).arg(QString::number(p0, 'f', pricePrecision(p0, stb)));
            str_state = QString("%1  price1=%2").arg(str_state).arg(QString::number(p1, 'f', pricePrecision(p1, stb)));
            m_table->table()->item(i, POOL_STATE_COL)->setText(str_state);

            m_table->table()->item(i, NOTE_COL)->setText("getted state");
            m_table->table()->item(i, NOTE_COL)->setTextColor("#4B0082");

            float tvl0 = j_result.value("tvl0").toString().toFloat();
            float tvl1 = j_result.value("tvl1").toString().toFloat();
            QString str_tvl = QString("%1/%2").arg(int(tvl0)).arg(int(tvl1));


            emit signalGetTokenPrice(m_poolData.at(i).token0_addr, p0);
            emit signalGetTokenPrice(m_poolData.at(i).token1_addr, p1);
            if (p0 > 0 && p1 > 0)
            {
                float tvl_st = (p0*tvl0 + p1*tvl1)/1000;
                str_tvl = QString("%1 (%3 k_USD)").arg(str_tvl).arg(QString::number(tvl_st, 'f', 1));
            }
            else str_tvl = QString("%1 (? k_USD)").arg(str_tvl);

            m_table->table()->item(i, TVL_COL)->setText(str_tvl);


            break;
        }
    }
}
void JSPoolTab::answerSwap(const QJsonObject &j_result)
{
    if (m_poolData.isEmpty()) return;

   // qDebug("JSPoolTab::answerState");
    int n = m_poolData.count();
    for(int i=0; i<n; i++)
    {
        if (j_result.value("pool_address").toString() == m_poolData.at(i).address)
        {
            bool stb = m_poolData.at(i).isStablePool();

            QString note;
            float size_in = j_result.value("input_size").toString().toFloat();
            int input = j_result.value("input_token").toString().toInt();
            QString t_in = ((input == 0) ? m_poolData.at(i).ticker0() : m_poolData.at(i).ticker1());
            QString t_out = ((input == 1) ? m_poolData.at(i).ticker0() : m_poolData.at(i).ticker1());
            if (j_result.value("simulate_mode").toString() == "yes")
            {
                float size_out = j_result.value("output_size").toString().toFloat();
                note = QString("SIMULATE:");
                note = QString("%1 %2 %3").arg(note).arg(QString::number(size_in, 'f', pricePrecision(size_in, stb))).arg(t_in);
                note = QString("%1 => %2 %3").arg(note).arg(QString::number(size_out, 'f', pricePrecision(size_out, stb))).arg(t_out);
            }
            else
            {
                note = QString("SWAP:");
                note = QString("%1 %2 %3").arg(note).arg(QString::number(size_in, 'f', pricePrecision(size_in, stb))).arg(t_in);
                note = QString("%1 => X %2").arg(note).arg(t_out);

                sendTxRecordToLog(i, j_result);
            }
            m_table->table()->item(i, NOTE_COL)->setText(note);
            m_table->table()->item(i, NOTE_COL)->setTextColor("#4B0082");
            break;
        }
    }
}
int JSPoolTab::pricePrecision(float p, bool is_stable_pool) const
{
    if (p <= 0) return -1;
    if (is_stable_pool) return 6;

    if (p > 1000) return 1;
    if (p > 10) return 2;
    if (p > 4) return 3;
    if (p > 0.1) return 4;
    return 6;
}
void JSPoolTab::sendMintTx(const TxDialogData &mint_data, int row)
{
    emit signalMsg(QString("Send TX mint"));

    //prepare json params
    QJsonObject j_params;
    QMap<QString, QString>::const_iterator it = mint_data.dialog_params.constBegin();
    while (it != mint_data.dialog_params.constEnd())
    {
        bool is_int = (it.key().contains("tick") || it.key().contains("index") || it.key().contains("line"));
        bool is_double = (it.key().contains("price") || it.key().contains("amount"));

        if (is_int) j_params.insert(it.key(), it.value().toInt());
        else if (is_double) j_params.insert(it.key(), it.value().toDouble());
        else j_params.insert(it.key(), it.value());
        it++;
    }
    emit signalRewriteParamJson(j_params); //rewrite json-file

    m_table->table()->item(row, NOTE_COL)->setText("try mint ...");
    m_table->table()->item(row, NOTE_COL)->setTextColor("#DC143C");
    m_table->resizeByContents();
    m_table->table()->setEnabled(false);
    QStringList tx_params;
    tx_params << "qt_mint.js" << EthersPage::inputParamsJsonFile();
    emit signalPoolAction(tx_params);
}
void JSPoolTab::answerMintTx(const QJsonObject &j_result)
{
    qDebug("**************JSPoolTab::answerMintTx");
    int row = m_table->curSelectedRow();

    QString note;
    float amount0 = j_result.value("token0_amount").toString().toFloat();
    float amount1 = j_result.value("token1_amount").toString().toFloat();
    int t1 = j_result.value("tick_lower").toString().toInt();
    int t2 = j_result.value("tick_upper").toString().toInt();
    bool is_simulate = (j_result.value("is_simulate").toString().trimmed().toLower() == "true");
    note = QString("AMOUNTS(%1 / %2)").arg(QString::number(amount0, 'f', 2)).arg(QString::number(amount1, 'f', 2));
    note = QString("%1 RANGE[%2; %3]").arg(note).arg(t1).arg(t2);
    m_table->table()->item(row, NOTE_COL)->setText(note);
    m_table->table()->item(row, NOTE_COL)->setTextColor("#4B0082");


    QString tx_hash("IS_SIMULATE_MODE");
    if (!is_simulate)
    {
        tx_hash = j_result.value("tx_hash").toString().trimmed().toLower();
        sendTxRecordToLog(row, j_result);
    }
    m_table->table()->item(row, NOTE_COL)->setToolTip(tx_hash);

    emit signalMsg(LString::symbolString('-', 100));
    emit signalMsg("TX mint done!");
    emit signalMsg(note);
    emit signalMsg(QString("REAL_PRICES: %1").arg(j_result.value("real_price_range").toString()));
    emit signalMsg(tx_hash);

}
void JSPoolTab::slotScriptBroken()
{
    m_table->table()->setEnabled(true);
    m_table->resizeByContents();
}
void JSPoolTab::sendTxRecordToLog(int row, const QJsonObject &j_result)
{
    qDebug("");
    qDebug("JSPoolTab::sendTxRecordToLog");

    QString tx_kind = j_result.value("type").toString().trimmed();
    tx_kind.remove("tx_");
    tx_kind = tx_kind.toLower().trimmed();

    QString cur_chain;
    emit signalGetChainName(cur_chain);
    JSTxLogRecord rec(tx_kind, cur_chain);
    if (j_result.contains("tx_hash"))
        rec.tx_hash = j_result.value("tx_hash").toString().trimmed();

    if (tx_kind.contains("mint"))
    {
        rec.token_address = "---";
        rec.token0_size = j_result.value("token0_amount").toString().trimmed().toFloat();
        rec.token1_size = j_result.value("token1_amount").toString().trimmed().toFloat();
        rec.current_price = j_result.value("current_price").toString().trimmed().toFloat();
        rec.current_tick = j_result.value("current_tick").toString().trimmed().toInt();
        int t1 = j_result.value("tick_lower").toString().trimmed().toInt();
        int t2 = j_result.value("tick_upper").toString().trimmed().toInt();
        rec.tick_range = QString("%1:%2").arg(t1).arg(t2);
        QString s_prange = j_result.value("real_price_range").toString().trimmed();
        s_prange.remove("(");
        s_prange.remove(")");
        QStringList plist = LString::trimSplitList(s_prange, ";");
        if (plist.count() == 2)
            rec.price_range = QString("%1:%2").arg(plist.at(0).trimmed()).arg(plist.at(1).trimmed());
        rec.note = m_poolData.at(row).assets.trimmed();
        rec.note.replace("/", ":");
        float fee_p = float(m_poolData.at(row).fee)/float(10000);
        rec.note = QString("%1:%2%").arg(rec.note).arg(fee_p);
        rec.note = LString::removeSpaces(rec.note);
        rec.note = QString("POOL[%1]").arg(rec.note);
    }
    else if (tx_kind.contains("swap"))
    {
        qDebug(" - branch SWAP");
        rec.token0_size = j_result.value("input_size").toString().trimmed().toFloat();
        rec.token1_size = j_result.value("output_size").toString().trimmed().toFloat();
        int t_index = j_result.value("input_token").toString().trimmed().toInt();
        rec.token_address = ((t_index == 0) ? m_poolData.at(row).token0_addr : m_poolData.at(row).token1_addr);
        rec.current_price = j_result.value("price_token").toString().trimmed().toFloat();
        rec.current_tick = j_result.value("pool_tick").toString().trimmed().toInt();

        qDebug()<<QString("current_price=%1  current_tick=%2").arg(rec.current_price).arg(rec.current_tick);

        rec.note = QString::number(rec.token0_size, 'f', SubGraph_CommonSettings::interfacePrecision(rec.token0_size));
        rec.note = QString("%1 %2").arg(rec.note).arg((t_index == 0) ? m_poolData.at(row).ticker0() : m_poolData.at(row).ticker1());
        rec.note = QString("%1 => %2").arg(rec.note).arg(QString::number(rec.token1_size, 'f', SubGraph_CommonSettings::interfacePrecision(rec.token1_size)));
        rec.note = QString("%1 %2").arg(rec.note).arg((t_index == 0) ? m_poolData.at(row).ticker1() : m_poolData.at(row).ticker0());
        qDebug() << QString("NOTE [%1]").arg(rec.note);
    }
    else
    {
        emit signalError(QString("JSPoolTab: invalid TX_kind - [%1]").arg(tx_kind));
        return;
    }


    rec.pool_address = m_poolData.at(row).address;
    emit signalSendTxLog(rec);
}

///////////////////////////JSPoolRecord//////////////////////////////

void JSPoolRecord::reset()
{
    address.clear();
    fee = 0;
    token0_addr.clear();
    token1_addr.clear();
    chain = QString("?");
    assets.clear();
}
bool JSPoolRecord::invalid() const
{
    if (address.isEmpty() || token0_addr.isEmpty() || token1_addr.isEmpty()) return true;
    if (address.length()<30 || token0_addr.length()<30 || token1_addr.length()<30) return true;
    if (fee != 100 && fee != 500 && fee != 3000 && fee != 10000) return true;
    if (assets.isEmpty() || !assets.contains("/")) return true;
    return false;
}
void JSPoolRecord::fromFileLine(const QString &fline)
{
  //  qDebug()<<QString("fromFileLine [%1]").arg(fline);
    reset();
    QString s = fline.trimmed();
    if (s.isEmpty()) return;

    QStringList list = LString::trimSplitList(s, "/");
    if (list.count() != 6) {qWarning()<<QString("JSPoolRecord::fromFileLine WARNING list.count(%1)!=5").arg(list.count()); return;}
   // qDebug()<<QString("fromFileLine list.count()[%1]").arg(list.count());


    int i = 0;
    chain = list.at(i).trimmed().toUpper(); i++;
    address = list.at(i).trimmed().toLower(); i++;
    assets =  list.at(i).trimmed(); i++;
    QStringList desc_list = LString::trimSplitList(assets, ":");
    if (desc_list.count() == 3) assets = QString("%1 / %2").arg(desc_list.at(0).trimmed()).arg(desc_list.at(1).trimmed());
    token0_addr = list.at(i).trimmed().toLower(); i++;
    token1_addr = list.at(i).trimmed().toLower(); i++;
    fee = list.at(i).trimmed().toUInt(); i++;
}
QString JSPoolRecord::strFee() const
{
    switch (fee)
    {
        case 100: return QString("0.01%");
        case 500: return QString("0.05%");
        case 3000: return QString("0.3%");
        case 10000: return QString("1.0%");
        default: break;
    }
    return QString("?");
}
int JSPoolRecord::tickSpace() const
{
    switch (fee)
    {
        case 100: return 2;
        case 500: return 10;
        case 3000: return 60;
        case 10000: return 200;
        default: break;
    }
    return -1;
}
bool JSPoolRecord::isStablePool() const
{
    QStringList list = LString::trimSplitList(assets, "/");
    if (list.count() != 2) return false;

    QString t0 = list.at(0).trimmed().toLower();
    if (t0 == "dai" || t0.contains("usd"))
    {
        QString t1 = list.at(1).trimmed().toLower();
        if (t1 == "dai" || t1.contains("usd")) return true;
    }
    return false;
}
QString JSPoolRecord::ticker0() const
{
    QStringList list = LString::trimSplitList(assets, "/");
    if (list.count() != 2) return "?";
    return list.at(0).trimmed().toUpper();
}
QString JSPoolRecord::ticker1() const
{
    QStringList list = LString::trimSplitList(assets, "/");
    if (list.count() != 2) return "?";
    return list.at(1).trimmed().toUpper();
}
