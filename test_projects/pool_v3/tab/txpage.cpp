#include "txpage.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "ltime.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "txlogger.h"
#include "nodejsbridge.h"



#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSplitter>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>

#define HASH_COL                0
#define TX_KIND_COL             3
#define GAS_USED_COL            4
#define RESULT_COL              7
#define FEE_NATIVE_COL          5
#define FEE_CENT_COL            6


//#define CHECK_STATE_TIMER_INTERVAL  700
#define FEE_COIN_PRECISION          8
#define FEE_CENT_PRECISION          4


// DefiTxTabPage
DefiTxTabPage::DefiTxTabPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkTx),
    m_logger(NULL)
     // m_checkStateTimer(NULL),
     // js_running(false)
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
    //return;
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
    m_table->setSelectionColor("#B7F8F9");
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
        //qDebug()<<QString("2");

        if (rec.resultOk()) t->item(last_row, RESULT_COL)->setTextColor(Qt::darkGreen);
        else if (rec.resultFault()) t->item(last_row, RESULT_COL)->setTextColor(Qt::red);
        else t->item(last_row, RESULT_COL)->setTextColor(Qt::gray);

        if (rec.tx_kind == "mint") t->item(last_row, TX_KIND_COL)->setTextColor("#FF8C00");
        if (rec.tx_kind == "burn") t->item(last_row, TX_KIND_COL)->setTextColor("#AA0000");
        last_row++;
        //qDebug()<<QString("3");
    }
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
        }
    }
}
void DefiTxTabPage::updateTableRowByRecord(const QString &hash)
{
    selectRowByHash(hash);
    const TxLogRecord *rec = m_logger->recByHash(hash);
    if (!rec) {emit signalError(QString("DefiTxTabPage: TxLogRecord is null by hash [%1]").arg(hash)); return;}

    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError(QString("DefiTxTabPage: can't select row by hash [%1]").arg(hash)); return;}

    QTableWidget *t = m_table->table();
    t->item(row, GAS_USED_COL)->setText(QString::number(rec->status.gas_used));
    t->item(row, FEE_NATIVE_COL)->setText(QString::number(rec->status.fee_coin, 'f', FEE_COIN_PRECISION));
    t->item(row, FEE_CENT_COL)->setText(QString::number(rec->status.fee_cent, 'f', FEE_CENT_PRECISION));
    t->item(row, RESULT_COL)->setText(rec->status.result);
}
void DefiTxTabPage::selectRowByHash(const QString &v)
{
    QTableWidget *t = m_table->table();
    t->clearSelection();
    int n_row = t->rowCount();
    for (int i=0; i<n_row; i++)
    {
        QString t_hash = t->item(i, HASH_COL)->text().trimmed();
        if (t_hash == v.trimmed())
        {
            t->selectRow(i);
            break;
        }
    }
}

/*
void JSTxTab::loadTxFromFile(QString chain_name)
{
    qDebug("-----------------JSTxTab::loadTxFromFile()--------------------------");
    m_locData.clear();
    tx_data.clear();
    m_table->removeAllRows();
    chain_name = chain_name.trimmed().toUpper();

    QString fname = QString("%1%2%3").arg(sub_commonSettings.nodejs_path).arg(QDir::separator()).arg(JS_TX_FILE);
    emit signalMsg(QString("try load transactions list [%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("TX file not found");
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

        JSTxRecord rec;
        rec.fromFileLine(fline);
        if (!rec.invalid())
        {
            if (rec.chain == chain_name)
                tx_data.append(rec);
        }
    }
    emit signalMsg(QString("loaded %1 TX records").arg(tx_data.count()));
    qDebug()<<QString("loaded %1 TX records").arg(tx_data.count());

    loadLocalData();
    emit signalMsg(QString("loaded %1 lines local data").arg(m_locData.count()));
    qDebug()<<QString("loaded %1 lines local data").arg(m_locData.count());


    applyLocalData();
    reloadTable();
}
void JSTxTab::reloadTable()
{
    QTableWidget *t = m_table->table();

    m_table->removeAllRows();
    QStringList row_data;
    for (int i=0; i<tx_data.count(); i++)
    {
        row_data.clear();
        const JSTxRecord &rec = tx_data.at(i);
        row_data << QString::number(i+1) << rec.chain << rec.strDate() << rec.strTime();
        row_data << rec.hash << rec.kind << rec.strFee() << rec.strResult();
        LTable::addTableRow(t, row_data);

        if (tx_data.at(i).txOk()) t->item(i, RESULT_COL)->setTextColor(Qt::darkGreen);
        else if (tx_data.at(i).txFault()) t->item(i, RESULT_COL)->setTextColor(Qt::red);
        else t->item(i, RESULT_COL)->setTextColor(Qt::gray);

        if (tx_data.at(i).kind == "mint") t->item(i, TX_KIND_COL)->setTextColor("#FF8C00");
        if (tx_data.at(i).kind == "burn") t->item(i, TX_KIND_COL)->setTextColor("#AA0000");
    }
    m_table->resizeByContents();
    t->scrollToBottom();

    //qDebug()<<QString("JSTxTab: TX table rows %1").arg(t->rowCount());
}
void JSTxTab::loadLocalData()
{
    m_locData.clear();
    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(LOCAL_TX_FILE);
    emit signalMsg(QString("try load local file [%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("TX local_file not found");
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
void JSTxTab::applyLocalData()
{
    if (m_locData.isEmpty() || tx_data.isEmpty()) return;

    int n_rows = tx_data.count();
    foreach (const QString &line, m_locData)
    {
        QString s = line.trimmed();
        QStringList list = LString::trimSplitList(s, "/");
        if (list.count() != 3) {qWarning()<<QString("LocalRecord: WARNING list.count(%1)!=3").arg(list.count()); continue;}

        float fee = list.at(1).toFloat();
        for (int i=0; i<n_rows; i++)
        {
            if (tx_data.at(i).hash == list.first().trimmed())
            {
                tx_data[i].fee = fee;
                tx_data[i].finished_fault = (list.last().trimmed().toLower() == "fault");
            }
        }
    }
}
void JSTxTab::updateTableRowByRecord(const JSTxRecord *rec)
{
    if (!rec) return;
    if (rec->invalid()) return;
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    for (int i=0; i<n_rows; i++)
    {
        if (t->item(i, HASH_COL)->text() != rec->hash) continue;

        t->item(i, FEE_COL)->setText(rec->strFee());
        t->item(i, RESULT_COL)->setText(rec->strResult());
        if (rec->txOk()) t->item(i, RESULT_COL)->setTextColor(Qt::darkGreen);
        else if (rec->txFault()) t->item(i, RESULT_COL)->setTextColor(Qt::red);
        break;
    }
}
JSTxRecord* JSTxTab::recordByHash(QString hash_value)
{
    hash_value = hash_value.trimmed().toLower();
    if (hash_value.isEmpty() || tx_data.isEmpty()) return NULL;

    for (int i=0; i<tx_data.count(); i++)
    {
        if (tx_data.at(i).hash == hash_value)
            return &tx_data[i];
    }
    return NULL;
}
*/



/*
void JSTxTab::parseJSResult(const QJsonObject &j_result)
{
  //  qDebug("JSTxTab::parseJSResult");
  //  qDebug() << j_result;
    m_table->table()->setEnabled(true);

    QString hash = j_result.value("hash").toString().trimmed();
    if (hash.length() < 30) {emit signalError("invalid JS response, it has no HASH"); return;}
    JSTxRecord *rec = recordByHash(hash);
    if (!rec) {emit signalError(QString("not found record by HASH[%1]").arg(hash)); return;}

    QString str_status = j_result.value("status").toString().trimmed();
    QString str_fee = j_result.value("fee").toString().trimmed();
  //  qDebug()<<QString("str_status(%1)   str_fee(%2)").arg(str_status).arg(str_fee);

    bool is_finished = (j_result.value("finished").toString().trimmed() == "true");
  //  qDebug()<<QString("is_finished = %1").arg(is_finished?"YES":"NO");
    rec->setJSResponse(str_status, str_fee, is_finished);
    updateTableRowByRecord(rec);
    m_table->resizeByContents();

    addRecToLocalFile(rec);

//--------------------------------------------------

    emit signalMsg("\n\n\n");
    js_running = false;
}
void JSTxTab::addRecToLocalFile(const JSTxRecord *rec)
{
    if (!rec) return;
    QString fline = rec->toLocalFileLine().trimmed();
  //  qDebug()<<QString("LFile::appendFile  line[%1]").arg(fline);

    if (fline.isEmpty()) return;
    if (m_locData.contains(fline)) {qWarning("local file line already contains this line"); return;}

    QString err;
    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(LOCAL_TX_FILE);
  //  qDebug()<<fname;
    if (LFile::fileExists(fname))
    {
      //  qDebug("appendFile");
        LFile::appendFile(fname, QString("%1 \n").arg(fline));
    }
    else
    {
      //  qDebug("writeFile");
        LFile::writeFile(fname, QString("%1 \n").arg(fline));
    }
    if (!err.isEmpty()) emit signalError(QString("can't append line to local file: %1").arg(fname));
}
void JSTxTab::slotCheckTxResult(const QString &tx_hash, bool &ok)
{
    ok = false;
    foreach (const JSTxRecord &rec, tx_data)
    {
        if (rec.hash == tx_hash)
        {
            ok = rec.txOk();
            break;
        }
    }
}
void JSTxTab::getAllWaitingStates()
{
    qDebug("JSTxTab::getAllWaitingStates()");
    if (tx_data.isEmpty())
    {
        emit signalError("TX list is empty");
        emit signalEnableControls(false);
        return;
    }

    js_running = false;
    m_checkStateTimer->start();
}
void JSTxTab::slotCheckStateTimer()
{
    if (js_running) {qDebug("js_running"); return;}

    QString next_hash = QString();
    foreach (const JSTxRecord &rec, tx_data)
    {
        if (rec.txUnknown()) {next_hash = rec.hash; break;}
    }
    if (next_hash.isEmpty())
    {
        m_checkStateTimer->stop();
        emit signalMsg("Getting TX states finished!");
        emit signalEnableControls(true);
        return;
    }

    emit signalMsg(QString("next TX_HASH: %1").arg(next_hash));
    selectRowByHash(next_hash);
    slotTxStatus();
}
*/


/*

////////////////JSTxRecord//////////////////////
QString JSTxRecord::toLocalFileLine() const
{
    if (invalid()) return QString();
    if (txUnknown())  return QString();
    return QString("%1 / %2 / %3").arg(hash).arg(strFee()).arg(strResult());
}
void JSTxRecord::fromFileLine(const QString &fline)
{
   // qDebug()<<QString("fromFileLine [%1]").arg(fline);
    reset();
    QString s = fline.trimmed();
    if (s.isEmpty()) return;

    QStringList list = LString::trimSplitList(s, "/");
    if (list.count() != 7) {qWarning()<<QString("JSTxRecord::fromFileLine WARNING list.count(%1)!=7").arg(list.count()); return;}
   // qDebug()<<QString("fromFileLine list.count()[%1]").arg(list.count());


    int i = 0;
    s = list.at(i).trimmed();
    dt.setDate(QDate::fromString(s, "dd.MM.yyyy")); i++;
    s = list.at(i).trimmed();
    dt.setTime(QTime::fromString(s, "hh:mm:ss")); i++;
    hash = list.at(i).trimmed().toLower();  i++;
    chain = list.at(i).trimmed().toUpper();  i++;
    kind = list.at(i).trimmed().toLower();  i++;
}
bool JSTxRecord::invalid() const
{
    if (hash.length() < 30) return true;
    if (hash.left(2) != "0x") return true;
    if (!dt.isValid() || dt.isNull()) return true;
    if (kind.isEmpty() || kind == "none") return true;
    return false;
}
QString JSTxRecord::strTime() const
{
    return LTime::strDate(dt.date());
}
QString JSTxRecord::strDate() const
{
    return LTime::strTime(dt.time(), "hh:mm:ss");
}
QString JSTxRecord::strFee() const
{
    //if (txFault()) return "?";
    if (txFault()) return QString::number(fee, 'f', 4);
    if (txOk()) return QString::number(fee, 'f', 8);
    return "---";
}
QString JSTxRecord::strResult() const
{
    if (txFault()) return "FAULT";
    if (txOk()) return "OK";
    return "wait...";
}
void JSTxRecord::setJSResponse(QString js_status, QString js_fee, bool js_finished)
{
    if (!js_finished) {fee = -1; return;}

    fee = js_fee.toFloat();
    finished_fault = (js_status == "FAULT");
}
*/




