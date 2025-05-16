#include "balancehistorytab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"
#include "walletbalancehistory.h"


#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QDebug>

#define JS_BALANCE_FILE          "defi/balance_history.txt"
#define DT_MASK                 QString("dd.MM.yyyy / hh:mm:ss")
#define CELL_PRECISION          4
#define DEVIATION_COL           4
#define AMOUNT_COL              5
#define ADDRESS_COL             2
#define CHAIN_COL               1



// BalanceHistoryTab
BalanceHistoryTab::BalanceHistoryTab(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_table(NULL)
{
    setObjectName("js_balancehistory_tab");

    initTable();
    //loadHistory();
}
void BalanceHistoryTab::initTable()
{
    m_table = new LSearchTableWidgetBox(this);
    m_table->setTitle("Wallet assets events");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Date / time" << "Chain" << "Address" << "Asset" << "Deviation" << "Current amount";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#87CEEB");

    v_splitter->addWidget(m_table);
    m_table->resizeByContents();
    m_table->searchExec();
}
void BalanceHistoryTab::setAssets(const QMap<QString, QString> &assets)
{
    m_walletAssets.clear();
    if (assets.isEmpty()) return;

    QMap<QString, QString>::const_iterator it = assets.constBegin();
    while (it != assets.constEnd())
    {
        m_walletAssets.insert(it.value().trimmed().toLower(), it.key().trimmed());
        it++;
    }
}
void BalanceHistoryTab::reloadHistory(QString chain_name)
{
    qDebug("----BalanceHistoryTab::reloadHistory()----");
    m_table->removeAllRows();
    chain_name = chain_name.trimmed().toLower();


    QString fname = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(JS_BALANCE_FILE);
    emit signalMsg(QString("try load local file [%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("BALANCE local_file not found");
        return;
    }
    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}

    LString::removeEmptyStrings(fdata);        
    qDebug()<<QString("BALANCE local_file size %1").arg(fdata.size());
    if (fdata.isEmpty()) return;

    //remove other chains records
    int n = fdata.count();
    for (int i=n-1; i>=0; i--)
        if (fdata.at(i).trimmed().indexOf(chain_name) != 0) fdata.removeAt(i);

    qDebug()<<QString("BalanceHistoryTab::reloadHistory - CHAIN[%1]  records %2").arg(chain_name).arg(fdata.count());

    if (!fdata.isEmpty())
    {
        parseFileData(fdata);
        calcDeviationColumn();
    }

    m_table->resizeByContents();
    m_table->searchExec();
}
void BalanceHistoryTab::parseFileData(const QStringList &fdata)
{
    if (fdata.isEmpty()) return;

    //////////////////// parse file data //////////////////////////////
    QList<WalletBalanceHistory::SnapshotPointAsset> rec_list;
    foreach (const QString &fline, fdata)
    {
        WalletBalanceHistory::SnapshotPointAsset rec;
        rec.fromFileLine(fline); //парсим текущую строку файла
        if (rec.invalid()) {qWarning()<<QString("BalanceHistoryTab::parseFileData WARNING: can't read file record [%1]").arg(fline); continue;}

        if (rec_list.isEmpty()) {rec_list.append(rec); continue;}

        int n = rec_list.count();
        for (int i=0; i<n; i++)
        {
            if (rec.time >= rec_list.at(i).time)
            {
                rec_list.insert(i, rec);
                break;
            }
        }
        if (n == rec_list.count()) rec_list.append(rec);
    }

    ////////////////////fill table//////////////////////////////
    foreach (const WalletBalanceHistory::SnapshotPointAsset &rec, rec_list)
    {
        QStringList row_data;
        row_data << rec.time.toString(DT_MASK) << rec.chain.toUpper() << rec.token_address;
        row_data << m_walletAssets.value(rec.token_address, "?") << QString("-");
        row_data << QString::number(rec.balance, 'f', CELL_PRECISION);
        LTable::addTableRow(m_table->table(), row_data);
    }
}
void BalanceHistoryTab::calcDeviationColumn()
{
    QStringList addrs(m_walletAssets.keys());
    foreach (const QString &addr, addrs)
    {
        if (addr.trimmed().isEmpty()) continue;
        calcDeviationByAddr(addr);
    }
}
void BalanceHistoryTab::calcDeviationByAddr(QString addr)
{
    //qDebug()<<QString("BalanceHistoryTab::calcDeviationByAddr  addr[%1]").arg(addr);
    QTableWidget *t = m_table->table();
    int n_row = t->rowCount();
    if (n_row == 0) return;

    float amount = -1;
    for (int i=n_row-1; i>=0; i--)
    {
        t->item(i, CHAIN_COL)->setTextColor("#6A5ACD");

        QString t_addr = t->item(i, ADDRESS_COL)->text().trimmed();
        if (t_addr != addr) continue;

        float t_amount = t->item(i, AMOUNT_COL)->text().trimmed().toFloat();
        if (amount < 0) {amount = t_amount; continue;}

        float d = t_amount - amount;
        t->item(i, DEVIATION_COL)->setText(QString::number(d, 'f', CELL_PRECISION));
        amount = t_amount;
        if (d < 0) t->item(i, DEVIATION_COL)->setTextColor(Qt::darkRed);
        else t->item(i, DEVIATION_COL)->setTextColor(Qt::darkGreen);


    }
}

