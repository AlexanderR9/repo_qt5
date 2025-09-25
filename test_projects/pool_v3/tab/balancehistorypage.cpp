#include "balancehistorypage.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "appcommonsettings.h"
#include "deficonfig.h"
#include "nodejsbridge.h"
#include "walletbalancelogger.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QDebug>
#include <QJsonObject>

#define DEVIATION_COL       2
#define AMOUNT_COL          3
#define TOKEN_COL           1
#define DT_MASK             QString("dd.MM.yyyy / hh:mm:ss")
#define COLOR_DAY_EVEN      QString("#FFFFEE")
#define COLOR_DAY_ODD       QString("#EEFFFF")


// BalanceHistoryPage
BalanceHistoryPage::BalanceHistoryPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkBalance),
      m_balanceObj(NULL)
{
    setObjectName("balances_history_tab_page");

    //init table
    initTable();

    //init balance history object
    m_balanceObj = new WalletBalanceLogger(m_tokenList, this);
    connect(m_balanceObj, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_balanceObj, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_balanceObj, SIGNAL(signalAddNewRecord(float)), this, SLOT(slotAddNewRecord(float)));


}
void BalanceHistoryPage::setChain(int cid)
{
    BaseTabPage_V3::setChain(cid);
    initTokenList(cid);
    m_balanceObj->loadLogFile(curChainName());

    reloadLogToTable();
    m_table->resizeByContents();
    startSearch();

    //qDebug()<<QString("BalanceHistoryPage: loaded balance records %1, chain[%2]").arg(m_balanceObj->logSize()).arg(curChainName());
}
void BalanceHistoryPage::initTable()
{
    m_table = new LSearchTableWidgetBox(this);
    m_table->setTitle("Wallet assets events");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Date / time" << "Asset" << "Deviation" << "Current amount";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#87CEEB");

    h_splitter->addWidget(m_table);
    m_table->resizeByContents();
    startSearch();
}
void BalanceHistoryPage::initTokenList(int cid)
{
    m_tokenList.clear();
    foreach (const DefiToken &v, defi_config.tokens)
    {
        if (v.chain_id == cid)
            m_tokenList.append(v.name);
    }
    m_tokenList.removeAll(defi_config.nativeTokenName(curChainName()));
    //qDebug()<<QString("BalanceHistoryPage::initTokenList  tracking tokens %1 in chain[%2]").arg(m_tokenList.count()).arg(curChainName());
}
void BalanceHistoryPage::startSearch()
{
    LSearchTableWidgetBox *st = qobject_cast<LSearchTableWidgetBox*>(m_table);
    if (st) st->searchExec();
}
void BalanceHistoryPage::slotNodejsReply(const QJsonObject &js_reply)
{
    QString req = js_reply.value(AppCommonSettings::nodejsReqFieldName()).toString();
   // qDebug()<<QString("BalanceHistoryPage::slotNodejsReply - req kind: [%1]").arg(req);

    if (req == NodejsBridge::jsonCommandValue(nrcBalance)) checkCurrentBalances(js_reply);
    else return;

    startSearch();
    m_table->resizeByContents();
}
void BalanceHistoryPage::slotAddNewRecord(float deviation)
{
  //  qDebug()<<QString("BalanceHistoryPage::slotAddNewRecord deviation=%2").arg(deviation);
  //  qDebug()<<QString("total balance records %1, chain[%2]").arg(m_balanceObj->logSize()).arg(curChainName());

    int i = m_balanceObj->logSize() - 1;
    const WalletBalanceLogger::TokenBalanceRecord &rec = m_balanceObj->recAt(i);


    QTableWidget *t = m_table->table();
    QStringList row_data;
    row_data <<  rec.toDT().toString(DT_MASK) << rec.name;
    row_data << QString::number(deviation, 'f', AppCommonSettings::interfacePricePrecision(deviation));
    row_data << QString::number(rec.balance, 'f', AppCommonSettings::interfacePricePrecision(rec.balance));
    LTable::insertTableRow(0, t, row_data);
    t->item(0, 0)->setData(Qt::UserRole, rec.toDT().date());
}
void BalanceHistoryPage::reloadLogToTable()
{
    m_table->removeAllRows();
    int n = m_balanceObj->logSize();
    if (n == 0) return;

    QTableWidget *t = m_table->table();

    QDate cur_date;
    QString row_color = COLOR_DAY_EVEN;
    QStringList row_data;
    for (int i=0; i<n; i++)
    {
        row_data.clear();
        const WalletBalanceLogger::TokenBalanceRecord &rec = m_balanceObj->recAt(i);
        row_data <<  rec.toDT().toString(DT_MASK) << rec.name << QString("---");
        row_data << QString::number(rec.balance, 'f', AppCommonSettings::interfacePricePrecision(rec.balance));
        LTable::insertTableRow(0, t, row_data);
        t->item(0, 0)->setData(Qt::UserRole, rec.toDT().date());
    }

    recalcDeviationCol();
    updateTableColors();
}
void BalanceHistoryPage::recalcDeviationCol()
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    if (n_rows <= 0) return;

    for (int i=0; i<n_rows; i++)
    {
        int k = n_rows - i - 1;
        float deviation = m_balanceObj->recBalanceDeviation(k);
        if (deviation == 0) continue;

        t->item(i, DEVIATION_COL)->setText(QString::number(deviation, 'f', AppCommonSettings::interfacePricePrecision(deviation)));
    }
}
void BalanceHistoryPage::updateTableColors()
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    if (n_rows <= 0) return;

    bool ok;
    QDate cur_date;
    QString row_color = COLOR_DAY_EVEN;
    for (int i=0; i<n_rows; i++)
    {
        //set row color
        QDate row_date = t->item(i, 0)->data(Qt::UserRole).toDate();
        if (!cur_date.isValid())  cur_date = row_date;
        if (row_date != cur_date)
        {
            if (row_color == COLOR_DAY_EVEN) row_color = COLOR_DAY_ODD;
            else row_color = COLOR_DAY_EVEN;
            cur_date = row_date;
        }
        LTable::setTableRowColor(t, i, row_color);

        //set deviation color
        float d = t->item(i, DEVIATION_COL)->text().toFloat(&ok);
        if (ok) t->item(i, DEVIATION_COL)->setTextColor((d>0) ? QString("#008000") : QString("#A52A2A"));
        else t->item(i, DEVIATION_COL)->setTextColor(Qt::lightGray);
    }
}
void BalanceHistoryPage::checkCurrentBalances(const QJsonObject &js_reply)
{
    QStringList keys = js_reply.keys();
   // qDebug()<<QString("BalanceHistoryPage::checkBalances  js_reply keys %1").arg(keys.count());
   // qDebug()<<"KEYS: "<<LString::uniteList(keys, " / ");

    bool ok;
    foreach (const QString &token, m_tokenList)
    {
        if (keys.contains(token))
        {
            QString s_value = js_reply.value(token).toString();
            float a = s_value.toFloat(&ok);
            if (ok)
            {
                m_balanceObj->receiveNewValue(token, a);
            }
            else qWarning()<<QString("BalanceHistoryPage::checkBalances WARNING can't convert value[%1] to float").arg(s_value);
        }
    }

    updateTableColors();
}

