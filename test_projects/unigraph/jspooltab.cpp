#include "jspooltab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"
#include "jstxdialog.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>

#define JS_POOL_FILE          "pools.txt"
#define JS_JSONPARAMS_FILE    "params.json"
#define FEE_COL                 3
#define POOL_ADDR_COL           1
#define POOL_STATE_COL          5
#define NOTE_COL                6



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

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetPoolState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTrySwapAssets())); i_menu++;

}

void JSPoolTab::rewriteParamJson(const QJsonObject &j_params)
{
    QJsonDocument j_doc(j_params);
    QString fdata(j_doc.toJson());
    fdata.append(QChar('\n'));
    QString fname = QString("%1%2%3").arg(sub_commonSettings.nodejs_path).arg(QDir::separator()).arg(JS_JSONPARAMS_FILE);
    emit signalMsg(QString("prepare json params for node_js script [%1].........").arg(fname));
    QString err = LFile::writeFile(fname, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}
    else emit signalMsg("JSON params file done!");
}
void JSPoolTab::slotGetPoolState()
{
    qDebug("JSPoolTab::slotGetPoolState()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    //prepare json params
    QJsonObject j_params;
    j_params.insert("address", m_poolData.at(row).address);
    j_params.insert("fee", m_poolData.at(row).fee);
    j_params.insert("token0", m_poolData.at(row).token0_addr);
    j_params.insert("token1", m_poolData.at(row).token1_addr);
    j_params.insert("tick_space", m_poolData.at(row).tickSpace());    
    rewriteParamJson(j_params); //rewrite json-file

    m_table->table()->item(row, NOTE_COL)->setText("getting state ...");
    m_table->table()->item(row, NOTE_COL)->setTextColor("#DC143C");
    m_table->resizeByContents();


    //send action
    emit signalMsg(QString("Try get pool[%1] state ...").arg(m_poolData.at(row).address));
    m_table->table()->setEnabled(false);
    QStringList tx_params;
    tx_params << "qt_pool_state.js" << JS_JSONPARAMS_FILE;
    emit signalPoolAction(tx_params);
}
void JSPoolTab::slotTrySwapAssets()
{
    qDebug("JSPoolTab::slotTrySwapAssets()");
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
        rewriteParamJson(j_params); //rewrite json-file
        //return;

        m_table->table()->item(row, NOTE_COL)->setText("try swap ...");
        m_table->table()->item(row, NOTE_COL)->setTextColor("#DC143C");
        m_table->resizeByContents();


        m_table->table()->setEnabled(false);
        QStringList tx_params;
        tx_params << "qt_pool_swap.js" << JS_JSONPARAMS_FILE;
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
    headers << "Chain" << "Pool address" << "Assets" << "Fee" << "Tick space" << "Current state" << "Note";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#87CEEB");

    v_splitter->addWidget(m_table);
}
void JSPoolTab::loadPoolsFromFile()
{
    qDebug("-----------------JSPoolTab::loadPoolsFromFile()--------------------------");
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

    foreach (const QString &v, fdata)
    {
        QString fline = v.trimmed();
        if (fline.isEmpty()) continue;
        if (fline.left(1) == "#") continue;

        JSPoolRecord rec;
        rec.fromFileLine(fline);
        if (!rec.invalid()) m_poolData.append(rec);
    }
    emit signalMsg(QString("loaded %1 POOL records").arg(m_poolData.count()));
    qDebug()<<QString("loaded %1 POOL records").arg(m_poolData.count());

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
        row_data << QString::number(rec.tickSpace()) << "?" << "---";
        LTable::addTableRow(t, row_data);

        if (rec.fee == 100) t->item(i, FEE_COL)->setTextColor(Qt::gray);
        else if (rec.fee == 500) t->item(i, FEE_COL)->setTextColor(Qt::darkGreen);
        else if (rec.fee == 3000) t->item(i, FEE_COL)->setTextColor(Qt::blue);
        else t->item(i, FEE_COL)->setTextColor(Qt::red);

        t->item(i, POOL_ADDR_COL)->setTextColor("#4682B4");
    }

    m_table->searchExec();
    m_table->resizeByContents();
    qDebug()<<QString("JSPoolTab: POOL table rows %1").arg(t->rowCount());
}
void JSPoolTab::parseJSResult(const QJsonObject &j_result)
{
    qDebug("JSPoolTab::parseJSResult");
    qDebug() << j_result;
    m_table->table()->setEnabled(true);

    QString operation = j_result.value("type").toString().trimmed();
    if (operation == "state") answerState(j_result);
    else if (operation == "swap") answerSwap(j_result);
    else emit signalError(QString("invalid answer type: %1").arg(operation));

    m_table->resizeByContents();
}
void JSPoolTab::answerState(const QJsonObject &j_result)
{
    if (m_poolData.isEmpty()) return;

    qDebug("JSPoolTab::answerState");
    int n = m_poolData.count();
    for(int i=0; i<n; i++)
    {
        if (j_result.value("pool_address").toString() == m_poolData.at(i).address)
        {
            bool stb = m_poolData.at(i).isStablePool();
            qDebug()<<QString("find row, %1, pool: %2").arg(i).arg(m_poolData.at(i).address);
            int  tick = j_result.value("tick").toString().toInt();
            float p0 = j_result.value("price0").toString().toFloat();
            float p1 = j_result.value("price1").toString().toFloat();
            QString str_state = QString("tick=%1").arg(tick);
            str_state = QString("%1  price0=%2").arg(str_state).arg(QString::number(p0, 'f', pricePrecision(p0, stb)));
            str_state = QString("%1  price1=%2").arg(str_state).arg(QString::number(p1, 'f', pricePrecision(p1, stb)));
            m_table->table()->item(i, POOL_STATE_COL)->setText(str_state);

            m_table->table()->item(i, NOTE_COL)->setText("getted state");
            m_table->table()->item(i, NOTE_COL)->setTextColor("#4B0082");

            break;
        }
    }
}
void JSPoolTab::answerSwap(const QJsonObject &j_result)
{
    if (m_poolData.isEmpty()) return;

    qDebug("JSPoolTab::answerState");
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



///////////////////////////JSPoolRecord//////////////////////////////

void JSPoolRecord::reset()
{
    address.clear();
    fee = 0;
    token0_addr.clear();
    token1_addr.clear();
    chain = QString("POLYGON");
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
    qDebug()<<QString("fromFileLine [%1]").arg(fline);
    reset();
    QString s = fline.trimmed();
    if (s.isEmpty()) return;

    QStringList list = LString::trimSplitList(s, "/");
    if (list.count() != 5) {qWarning()<<QString("JSPoolRecord::fromFileLine WARNING list.count(%1)!=5").arg(list.count()); return;}
   // qDebug()<<QString("fromFileLine list.count()[%1]").arg(list.count());


    int i = 0;
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
