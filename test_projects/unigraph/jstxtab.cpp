#include "jstxtab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "ltime.h"
#include "subcommonsettings.h"
#include "jstxdialog.h"



#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>

#define JS_TX_FILE      "tx.log"
#define HASH_COL                4
#define RESULT_COL              7
#define FEE_COL                 6


// JSTxTab
JSTxTab::JSTxTab(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_table(NULL)
{
    setObjectName("js_tx_tab");

    //init table
    initTable();

    // init context menu
    initPopupMenu();

}
void JSTxTab::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Transactions");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Number" << "Chain" << "Date" << "Time" << "Hash" << "Kind" << "Fee (native)" << "Result";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#9D9F4F");

    v_splitter->addWidget(m_table);
}
void JSTxTab::loadTxFromFile()
{
    //qDebug("-------------");

    qDebug("-----------------JSTxTab::loadTxFromFile()--------------------------");
    tx_data.clear();
    QString fname = QString("%1%2%3").arg(sub_commonSettings.nodejs_path).arg(QDir::separator()).arg(JS_TX_FILE);
    emit signalMsg(QString("try load transactions list [%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("TX file not found");
        return;
    }
    //qDebug("JSTxTab::loadTxFromFile() 1");

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}

    //qDebug("JSTxTab::loadTxFromFile() 2");

    foreach (const QString &v, fdata)
    {
        QString fline = v.trimmed();
        if (fline.isEmpty()) continue;
        if (fline.left(1) == "#") continue;

        JSTxRecord rec;
        rec.fromFileLine(fline);
        if (!rec.invalid()) tx_data.append(rec);
    }
    emit signalMsg(QString("loaded %1 TX records").arg(tx_data.count()));

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
    }
    m_table->resizeByContents();
    qDebug()<<QString("JSTxTab: TX table rows %1").arg(t->rowCount());
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
void JSTxTab::initPopupMenu()
{
    //prepare menu actions data
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Check TX status", QString("%1/emblem-ok.svg").arg(SubGraph_CommonSettings::commonIconsPath()));
    act_list.append(pair1);

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTxStatus())); i_menu++;

}
void JSTxTab::slotTxStatus()
{
    qDebug("JSTxTab::slotTxStatus()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    QTableWidget *t = m_table->table();
    QString hash = t->item(row, HASH_COL)->text().trimmed();
    qDebug()<<QString("TX_HASH: %1").arg(hash);

    QStringList params;
    params << "qt_tx.js" << hash;
    t->setEnabled(false);
    emit signalCheckTx(params);
}
void JSTxTab::parseJSResult(const QJsonObject &j_result)
{
    qDebug("JSTxTab::parseJSResult");
    qDebug() << j_result;
    m_table->table()->setEnabled(true);

    QString hash = j_result.value("hash").toString().trimmed();
    if (hash.length() < 30) {emit signalError("invalid JS response, it has no HASH"); return;}
    JSTxRecord *rec = recordByHash(hash);
    if (!rec) {emit signalError(QString("not found record by HASH[%1]").arg(hash)); return;}

    QString str_status = j_result.value("status").toString().trimmed();
    QString str_fee = j_result.value("fee").toString().trimmed();
    qDebug()<<QString("str_status(%1)   str_fee(%2)").arg(str_status).arg(str_fee);

   // if (j_result.value("finished").isBool()) qDebug("FINISHED field is bool type");
   // else if (j_result.value("finished").isString()) qDebug("FINISHED field is string type");
   // else     qDebug("FINISHED field is ?? type");

    bool is_finished = (j_result.value("finished").toString().trimmed() == "true");
    qDebug()<<QString("is_finished = %1").arg(is_finished?"YES":"NO");
    rec->setJSResponse(str_status, str_fee, is_finished);
    updateTableRowByRecord(rec);
    m_table->resizeByContents();
}





////////////////JSTxRecord//////////////////////

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
    if (txFault()) return "?";
    if (txOk()) return QString::number(fee, 'f', 6);
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
    if (js_status == "FAULT")  {fee = -2; return;}
    fee = js_fee.toFloat();
}



