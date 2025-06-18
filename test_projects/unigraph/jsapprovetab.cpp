#include "jsapprovetab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"
#include "jstxdialog.h"
#include "jstxlogger.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>


#define ADDRESS_COL             3
#define TOKEN_COL               0
#define SUPPLIED_PRECISION      6
#define LOCAL_APPROVE_FILE       "defi/approve.txt"


// JSApproveTab
JSApproveTab::JSApproveTab(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_table(NULL),
      m_updateTimer(NULL),
      js_running(false)
{
    setObjectName("js_approve_tab");

    //init table
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Supplied amounts information");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Token" << "POS_MANAGER" << "SWAP_ROUTER" << "Address";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#BDFF4F");

    v_splitter->addWidget(m_table);
    initPopupMenu();

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(900);
    m_updateTimer->stop();
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(slotTimer()));

}
void JSApproveTab::setTokens(const QMap<QString, QString> &map, QString chain_name)
{
    m_table->removeAllRows();
    QStringList list(map.keys());
    foreach (const QString &t_name, list)
    {
        if (t_name == SubGraph_CommonSettings::nativeCoinByChain(chain_name))  continue;
        QStringList row_data;
        row_data << t_name.trimmed() << "?" << "?" << map.value(t_name).toLower().trimmed();
        LTable::addTableRow(m_table->table(), row_data);
    }

    loadLocalData();
    syncTableByLocalData();
    m_table->resizeByContents();
}
void JSApproveTab::initPopupMenu()
{
    //prepare menu actions data
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Update", QString("%1/view-refresh.svg").arg(SubGraph_CommonSettings::commonIconsPath()));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Approve asset", TxDialogBase::iconByTXType(txApprove));
    act_list.append(pair2);

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotUpdateApproved())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotSendApprove())); i_menu++;

}
void JSApproveTab::slotUpdateApproved()
{
    //qDebug("JSApproveTab::slotUpdateApproved()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    QTableWidget *t = m_table->table();
    QString token_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    t->setEnabled(false);
    emit signalCheckUpproved(token_addr);
}
void JSApproveTab::slotSendApprove()
{
  //  qDebug("JSApproveTab::slotSendApprove()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}
    QTableWidget *t = m_table->table();
   // qDebug()<<QString("token=%1  addr=%2").arg(t->item(row, TOKEN_COL)->text()).arg(t->item(row, ADDRESS_COL)->text());
    QString token_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    //t->setEnabled(false);

    //prepare dialog params
    TxDialogData approve_data(txApprove);
    approve_data.token_addr = token_addr;
    TxApproveDialog d(approve_data, this);
    d.exec();
    if (d.isApply())
    {
        emit signalMsg(QString("Try send TX approve"));
        emit signalMsg(QString("PARAMS: to_contract=%1  amount=%2").arg(approve_data.dialog_params.value("whom")).arg(approve_data.dialog_params.value("amount")));

        removeRecFromLocalData(token_addr);

        t->setEnabled(false);
        QStringList tx_params;
        tx_params << token_addr << approve_data.dialog_params.value("whom") << approve_data.dialog_params.value("amount");
        emit signalApprove(tx_params);
    }
    else emit signalMsg("operation was canceled!");
}
void JSApproveTab::parseJSResult(const QJsonObject &j_result)
{
   // qDebug("JSApproveTab::parseJSResult");
   // qDebug() << j_result;
    m_table->table()->setEnabled(true);

    QString operation = j_result.value("type").toString().trimmed();
    if (operation == "update") answerUpdate(j_result);
    else if (operation == "tx_approve") answerApprove(j_result);
    else emit signalError(QString("invalid answer type: %1").arg(operation));

    m_table->resizeByContents();
}
void JSApproveTab::answerUpdate(const QJsonObject &j_result)
{
  //  qDebug("JSApproveTab::answerUpdate");
    QTableWidget *t = m_table->table();

    bool ok;
    QString token_addr = j_result.value("token").toString().trimmed();
    float pm_sum = j_result.value("posmanager").toString().toFloat(&ok);
    if (!ok) pm_sum = -1;
    float swap_sum = j_result.value("swaprouter").toString().toFloat(&ok);
    if (!ok) swap_sum = -1;

    int n = t->rowCount();
    for (int i=0; i<n; i++)
    {
        if (t->item(i, ADDRESS_COL)->text().trimmed() == token_addr)
        {
            updateSuppliedCell(i, 1, pm_sum);
            updateSuppliedCell(i, 2, swap_sum);
            addLocalData(token_addr, pm_sum, swap_sum);
            break;
        }
    }

    /////////////////////////////////////
    if (m_updateTimer->isActive())
    {
        js_running = false;
        int row = m_table->curSelectedRow();
        if (row == (n-1)) {t->clearSelection(); return;}
        t->selectRow(row+1);
    }
}
void JSApproveTab::answerApprove(const QJsonObject &j_result)
{
   // qDebug("JSApproveTab::answerApprove");
    QString code = j_result.value("result_code").toString().trimmed();

    if (code == "OK")
    {
        QString hash = j_result.value("tx_hash").toString().trimmed();
        emit signalMsg("TX sended OK!!!");
        emit signalMsg(QString("TX_HASH: %1").arg(hash));

        sendTxRecordToLog(j_result);
    }
    else
    {
        emit signalError(QString("TX sending result: %1").arg(code));
    }

    syncTableByLocalData();
}
void JSApproveTab::updateSuppliedCell(int i, int j, float value)
{
    const QTableWidget *t = m_table->table();
    if (value < 0)
    {
        t->item(i, j)->setText("---");
        t->item(i, j)->setTextColor(Qt::red);
    }
    else if (value == 0)
    {
        t->item(i, j)->setText("0.0");
        t->item(i, j)->setTextColor(Qt::gray);
    }
    else
    {
        t->item(i, j)->setText(QString::number(value, 'f', SUPPLIED_PRECISION));
        t->item(i, j)->setTextColor(Qt::black);
    }
}
void JSApproveTab::slotScriptBroken()
{
    m_table->table()->setEnabled(true);
}
void JSApproveTab::slotResetRecord(const QString &token_addr)
{
    removeRecFromLocalData(token_addr);
    rewriteLocalDataFile();
    syncTableByLocalData();
    m_table->resizeByContents();
}
void JSApproveTab::slotGetApprovedSize(QString whom, const QString &token_addr, float &approved_size)
{
    approved_size = 0;
    whom = whom.trimmed().toLower();
  //  qDebug()<<QString("JSApproveTab::slotGetApprovedSize whom=[%1]").arg(whom);

    QTableWidget *t = m_table->table();
    int n = t->rowCount();
    for (int i=0; i<n; i++)
    {
        if (t->item(i, ADDRESS_COL)->text().trimmed() == token_addr)
        {
            float a = 0;
            bool ok = false;
            if (whom == "swap_router")
            {
                // qDebug()<<QString("cell_text[%1] => float").arg(t->item(i, 2)->text());
                a = t->item(i, 2)->text().toFloat(&ok);
            }
            else if (whom == "pos_manager") a = t->item(i, 1)->text().toFloat(&ok);
            if (ok) approved_size = a;
            else approved_size = -9999;
            break;
        }
    }
}
void JSApproveTab::loadLocalData()
{
    m_locData.clear();
    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(LOCAL_APPROVE_FILE);
    emit signalMsg(QString("try load local file [%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("APPROVE local_file not found");
        return;
    }
    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}

    foreach (const QString &line, fdata)
    {
        QString s = line.trimmed();
        if (s.isEmpty()) continue;
        QStringList list = LString::trimSplitList(s, "/");
        if (list.count() == 3)  m_locData.append(s);
    }
}
void JSApproveTab::rewriteLocalDataFile()
{
    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(LOCAL_APPROVE_FILE);
    emit signalMsg(QString("try rewrite local file [%1].........").arg(fname));

    QString err = LFile::writeFileSL(fname, m_locData);
    if (!err.isEmpty()) emit signalError(err);
    else emit signalMsg(QString("done, wrote %1 records").arg(m_locData.count()));
}
void JSApproveTab::removeRecFromLocalData(const QString &token_addr)
{
  //  qDebug()<<QString("removeRecFromLocalData [%1]").arg(token_addr);
    if (m_locData.isEmpty()) return;

    int n = m_locData.count();
    for (int i=0; i<n; i++)
        if (m_locData.at(i).contains(token_addr))
        {
           // qDebug()<<QString("find, remove index %1").arg(i);
            m_locData.removeAt(i); break;
        }

    rewriteLocalDataFile();
}
void JSApproveTab::addLocalData(const QString &token_addr, float pm_sum, float swap_sum)
{
    removeRecFromLocalData(token_addr.trimmed().toLower());
    QString line = QString("%1 / %2 / %3").arg(token_addr).arg(QString::number(pm_sum, 'f', 6)).arg(QString::number(swap_sum, 'f', 6));
    emit signalMsg(QString("add new local data record: [%1]").arg(line));
    m_locData << line;
    rewriteLocalDataFile();
}
void JSApproveTab::syncTableByLocalData()
{
    if (m_locData.isEmpty()) return;
    float pm_sum = 0;
    float swap_sum = 0;

    QTableWidget *t = m_table->table();
    int n = t->rowCount();
    for (int i=0; i<n; i++)
    {
        QString token_addr = t->item(i, ADDRESS_COL)->text().trimmed();
        getApprovedByLocalData(token_addr, pm_sum, swap_sum);
        updateSuppliedCell(i, 1, pm_sum);
        updateSuppliedCell(i, 2, swap_sum);
    }
}
void JSApproveTab::getApprovedByLocalData(const QString &token_addr, float &pm_sum, float &swap_sum) const
{
    pm_sum = swap_sum = -1;
    if (m_locData.isEmpty()) return;

    foreach (const QString &line, m_locData)
    {
        QString s = line.trimmed();
        if (s.isEmpty()) continue;
        QStringList list = LString::trimSplitList(s, "/");
        if (list.count() == 3)
        {
            if (list.first().trimmed() == token_addr)
            {
                pm_sum = list.at(1).toFloat();
                swap_sum = list.at(2).toFloat();
                break;
            }
        }
    }
}
void JSApproveTab::sendTxRecordToLog(const QJsonObject &j_result)
{
    qDebug("");
    qDebug("JSApproveTab::sendTxRecordToLog");

    QString tx_kind = j_result.value("type").toString().trimmed();
    tx_kind.remove("tx_");
    tx_kind = tx_kind.toLower().trimmed();

    QString cur_chain;
    emit signalGetChainName(cur_chain);
    JSTxLogRecord rec(tx_kind, cur_chain);

    if (j_result.contains("tx_hash"))
        rec.tx_hash = j_result.value("tx_hash").toString().trimmed();
    if (j_result.contains("token"))
    {
        rec.token_address = j_result.value("token").toString().trimmed();
        emit signalGetTokenPrice(rec.token_address, rec.current_price);
    }
    if (j_result.contains("size"))
        rec.token0_size = j_result.value("size").toString().trimmed().toFloat();


    qDebug()<<QString("tx_kind[%1]  token_address[%2]").arg(tx_kind).arg(rec.token_address);
    qDebug()<<QString("current_price[%1]  size[%2]").arg(rec.current_price).arg(rec.token0_size);

    if (j_result.contains("whom")) rec.note = QString("WHOM[%1]").arg(j_result.value("whom").toString().trimmed());
    rec.pool_address = "---";
    emit signalSendTxLog(rec);

}
void JSApproveTab::slotTimer()
{
    if (js_running) {qDebug("js_running"); return;}

    int row = m_table->curSelectedRow();
    if (row < 0)
    {
        m_updateTimer->stop();
        emit signalMsg("Getting approved finished!");
        emit signalEnableControls(true);
        return;
    }
    emit signalMsg(QString("next request approved, TOKEN: %1").arg(m_table->table()->item(row, 0)->text()));
    js_running = true;
    slotUpdateApproved();
}
void JSApproveTab::getAllApprovedVolums()
{
    QTableWidget *t = m_table->table();
    if (t->rowCount() == 0)
    {
        m_updateTimer->stop();
        emit signalMsg("Getting states finished!");
        emit signalEnableControls(true);
        return;
    }

    t->clearSelection();
    t->selectRow(0);
    js_running = false;

    emit signalMsg(QString("Start getting approved volumes ....... "));
    emit signalEnableControls(false);
    m_updateTimer->start();
}

