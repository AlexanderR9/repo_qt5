#include "txpage.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "ltime.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "txlogger.h"
#include "nodejsbridge.h"
#include "txdialog.h"



#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimerEvent>

#define HASH_COL                    0
#define TX_KIND_COL                 3
#define GAS_USED_COL                4
#define RESULT_COL                  7
#define FEE_NATIVE_COL              5
#define FEE_CENT_COL                6


#define FEE_COIN_PRECISION          8
#define FEE_CENT_PRECISION          4
#define TIMER_DELAY_AFTER_STATUS    1700


// DefiTxTabPage
DefiTxTabPage::DefiTxTabPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkTx),
    m_logger(NULL),
    m_autoMode(false)
{
    setObjectName("defi_tx_tab_page");

    //init table
    initTable();

    //init context menu
    initPopupMenu();
}
void DefiTxTabPage::slotNewTx(const TxLogRecord &rec)
{
    m_logger->addNewRecord(rec);

    QTableWidget *t = m_table->table();
    QStringList row_data;
    row_data << rec.tx_hash << rec.strDate() << rec.strTime() << rec.tx_kind;
    row_data << QString::number(rec.status.gas_used) << QString::number(rec.status.fee_coin) << QString::number(rec.status.fee_cent);
    row_data << rec.status.result;

    LTable::insertTableRow(0, t, row_data);   
    setRecordIcon(0, rec);
    updateRowColor(0);
    m_table->resizeByContents();

    emit signalStartTXDelay();
}
void DefiTxTabPage::setChain(int cid)
{
    BaseTabPage_V3::setChain(cid);

    m_logger = new DefiTxLogger(curChainName(), this);
    connect(m_logger, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_logger, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));

    m_logger->reloadLogFiles();
    emit signalMsg(QString("Loaded %1 TX records").arg(m_logger->logSize()));
    reloadTables();

    m_table->resizeByContents();
    m_integratedTable->resizeByContents();
}
void DefiTxTabPage::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Transactions");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Hash" << "Date" << "Time" << "Kind" << "Gas used" << "Fee (native)" << "Fee (cents)" << "Result";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#D3D3D3");
    h_splitter->addWidget(m_table);

    /////////////////////////////////////////

    m_integratedTable = new LTableWidgetBox(this);
    m_integratedTable->setTitle("Total state");
    headers.clear();
    headers << "Value";
    LTable::setTableHeaders(m_integratedTable->table(), headers);
    headers.clear();
    headers << "Total TX" << "Unknown status" << "Fault status" <<  "Fee paid (native)" << "Fee paid (cents)";
    LTable::setTableHeaders(m_integratedTable->table(), headers, Qt::Vertical);
    m_integratedTable->setSelectionMode(QAbstractItemView::NoSelection, QAbstractItemView::NoSelection);
    LTable::createAllItems(m_integratedTable->table());
    h_splitter->addWidget(m_integratedTable);
}
void DefiTxTabPage::reloadTables()
{
    m_table->removeAllRows();
    if (m_logger->logEmpty()) return;

    QTableWidget *t = m_table->table();
    int n = m_logger->logSize();
    //qDebug()<<QString("logSize %1").arg(n);
    QStringList row_data;
    int last_row = 0;
    for (int i=n-1; i>=0; i--)
    {
        //qDebug()<<QString("i %1").arg(1);
        row_data.clear();
        const TxLogRecord &rec = m_logger->recordAt(i);
        row_data << rec.tx_hash << rec.strDate() << rec.strTime() << rec.tx_kind;
        row_data << QString::number(rec.status.gas_used) << QString::number(rec.status.fee_coin) << QString::number(rec.status.fee_cent);
        row_data << rec.status.result;

        LTable::addTableRow(t, row_data);
        setRecordIcon(last_row, rec);
        updateRowColor(last_row);
        last_row++;
    }

    updateTotalTable();
}
void DefiTxTabPage::setRecordIcon(int row, const TxLogRecord &rec)
{
    QTableWidget *t = m_table->table();
    if (row < 0 || row >= t->rowCount()) return;

    t->item(row, TX_KIND_COL)->setToolTip(rec.note);

    int tx_code = NodejsBridge::commandCodeByTxKind(rec.tx_kind);
    if (tx_code < 0) return;

    QString path = TxDialogBase::iconByTXType(tx_code);
    t->item(row, TX_KIND_COL)->setIcon(QIcon(path));
}
void DefiTxTabPage::initPopupMenu()
{
    //prepare menu actions data
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Check TX status", QString("%1/emblem-ok.svg").arg(AppCommonSettings::commonIconsPath()));
    act_list.append(pair1);

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTxStatus())); i_menu++;
}
void DefiTxTabPage::slotTxStatus()
{
  //  qDebug("JSTxTab::slotTxStatus()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    QString hash = m_table->table()->item(row, HASH_COL)->text().trimmed();
    qDebug()<<QString("DefiTxTabPage::slotTxStatus()  row %1  hash[%2]").arg(row).arg(hash);
    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcTXStatus));
    j_params.insert(AppCommonSettings::nodejsTxHashFieldName(), hash);
    sendReadNodejsRequest(j_params);
}
void DefiTxTabPage::slotNodejsReply(const QJsonObject &js_reply)
{
    QString req = js_reply.value(AppCommonSettings::nodejsReqFieldName()).toString();
    qDebug()<<QString("DefiTxTabPage::slotNodejsReply - req kind: [%1]").arg(req);

    if (req == NodejsBridge::jsonCommandValue(nrcTXStatus))
    {
        if (js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
        {
            // TX was finished, now we can look status
            QString hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString();
            quint32 gas_used = js_reply.value("gas_used").toString().toUInt();
            float fee_native = js_reply.value("fee").toString().toFloat();
            QString status = js_reply.value("status").toString();
            m_logger->updateRecStatus(hash, status, fee_native, gas_used);

            updateTableRowByRecord(hash);
            updateTotalTable();
            analyzeStatusLastTx();
        }
        else emit signalError("DefiTxTabPage: nodejs reply has't field <tx_hash>");
    }
    m_autoMode = false;
}
void DefiTxTabPage::updateTableRowByRecord(const QString &hash)
{
    selectRowByCellData(hash, HASH_COL);
    const TxLogRecord *rec = m_logger->recByHash(hash);
    if (!rec) {emit signalError(QString("DefiTxTabPage: TxLogRecord is null by hash [%1]").arg(hash)); return;}

    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError(QString("DefiTxTabPage: can't select row by hash [%1]").arg(hash)); return;}

    QTableWidget *t = m_table->table();
    t->item(row, GAS_USED_COL)->setText(QString::number(rec->status.gas_used));
    t->item(row, FEE_NATIVE_COL)->setText(QString::number(rec->status.fee_coin, 'f', FEE_COIN_PRECISION));
    t->item(row, FEE_CENT_COL)->setText(QString::number(rec->status.fee_cent, 'f', FEE_CENT_PRECISION));
    t->item(row, RESULT_COL)->setText(rec->status.result);

    updateRowColor(row);
}
void DefiTxTabPage::updateTotalTable()
{
    QTableWidget *t = m_integratedTable->table();

    int n = m_logger->logSize();
    t->item(0, 0)->setText(QString::number(n));

    if (m_logger->logEmpty())
    {
        t->item(0, 3)->setText("?");
        t->item(0, 4)->setText("?");
        return;
    }

    int n_fault = 0;
    int n_unknow = 0;
    float fee_native = 0;
    float fee_cent = 0;
    for (int i=0; i<n; i++)
    {
        const TxLogRecord &rec = m_logger->recordAt(i);
        if (rec.isFinishedStatus())
        {
            if (rec.resultFault()) n_fault++;
            fee_native += rec.status.fee_coin;
            fee_cent += rec.status.fee_cent;
        }
        else n_unknow++;
    }

    int row = 1;
    t->item(row, 0)->setText(QString::number(n_unknow)); row++;
    t->item(row, 0)->setText(QString::number(n_fault)); row++;
    if (n_fault > 0) t->item(row, 0)->setTextColor(Qt::red);
    t->item(row, 0)->setText(QString("%1 %2").arg(QString::number(fee_native, 'f', FEE_CENT_PRECISION)).arg(defi_config.nativeTokenName(curChainName()))); row++;
    t->item(row, 0)->setText(QString("%1 c.").arg(QString::number(fee_cent, 'f', 2))); row++;

    m_integratedTable->resizeByContents();
}
void DefiTxTabPage::updateRowColor(int row)
{
    QTableWidget *t = m_table->table();
    if (row < 0 || row >= t->rowCount()) return;

    QString status = t->item(row, RESULT_COL)->text().trimmed().toLower();
    QString tx_kind = t->item(row, TX_KIND_COL)->text().trimmed().toLower();
    if (status == "ok") t->item(row, RESULT_COL)->setTextColor(Qt::darkGreen);
    else if (status == "fault") t->item(row, RESULT_COL)->setTextColor(Qt::red);
    else t->item(row, RESULT_COL)->setTextColor(Qt::gray);
    if (tx_kind == "mint") t->item(row, TX_KIND_COL)->setTextColor("#FF8C00");
    if (tx_kind == "burn") t->item(row, TX_KIND_COL)->setTextColor("#AA0000");
}
void DefiTxTabPage::autoCheckStatusLastTx()
{
    qDebug("DefiTxTabPage::checkStatusLastTx()");
    QTableWidget *t = m_table->table();
    t->clearSelection();
    t->selectRow(0);

    m_autoMode = true;
    slotTxStatus();
}
void DefiTxTabPage::analyzeStatusLastTx()
{
    if (m_autoMode)
    {
        const TxLogRecord &rec = m_logger->recordLast();
        if (rec.resultOk())
        {
            qDebug()<<QString("DefiTxTabPage::analyzeStatusLastTx() recordLast[%1] ").arg(rec.tx_hash);
            int x = startTimer(TIMER_DELAY_AFTER_STATUS);
            qDebug() << QString("DefiTxTabPage::analyzeStatusLastTx()  start timer ID=%1").arg(x);
        }
    }
}
void DefiTxTabPage::timerEvent(QTimerEvent *event)
{
    int t_id = event->timerId();
    qDebug() << QString("KILL Timer ID:  %1").arg(t_id);
    this->killTimer(t_id);

    qDebug("DefiTxTabPage::timerEvent: next step [emit signalUpdatePageBack]");
    const TxLogRecord &rec = m_logger->recordLast();
    QString extra_data = QString();
    if (rec.tx_kind == NodejsBridge::jsonCommandValue(txApprove)) extra_data = rec.wallet.token_addr;
    emit signalUpdatePageBack(rec.tx_kind, extra_data);
}




