#include "bb_bagstatepage.h"
#include "ltable.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

#define API_WALLET_URI          QString("v5/account/wallet-balance")


//BB_BagStatePage
BB_BagStatePage::BB_BagStatePage(QWidget *parent)
    :BB_BasePage(parent, 20, rtBag),
      m_table(NULL)
{
    setObjectName("bag_state_page");
    init();

    m_reqData->params.insert("accountType", "UNIFIED");
    m_reqData->params.insert("coin", "USDT");
    m_reqData->uri = API_WALLET_URI;

}
void BB_BagStatePage::init()
{
    m_table = new LTableWidgetBox(this);
    m_table->setObjectName("bag_table");
    m_table->setTitle("Current state");
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    QStringList headers;
    headers.append("Value");
    m_table->setHeaderLabels(headers);

    headers.clear();
    headers << "Wallet (total/free)" << "Positions" << "Orders" << "Freezed sum (pos/order)" << "Freezed sum (total)" << "Current result (opened pos)";
    m_table->setHeaderLabels(headers, Qt::Vertical);
    for (int i=0; i<m_table->table()->rowCount(); i++)
        LTable::createTableItem(m_table->table(), i, 0, "-");

    h_splitter->addWidget(m_table);
    m_table->resizeByContents();
}
void BB_BagStatePage::updateDataPage(bool force)
{
    if (updateTimeOver(force))
    {
        m_state.reset();
        emit signalGetPosState(m_state);
        sendRequest();
    }
    else
    {
        emit signalGetPosState(m_state);
        updateTable();
        m_table->resizeByContents();
    }
}
void BB_BagStatePage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    BB_BasePage::slotJsonReply(req_type, j_obj);
    if (req_type != m_userSign) return;

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_BagStatePage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_BagStatePage: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_BagStatePage: j_arr QJsonArray is empty"); return;}
    QJsonValue j_coin = j_arr.at(0).toObject().value("coin");
    if (j_coin.isNull()) {emit signalError("BB_BagStatePage: coin QJsonElement not found"); return;}
    const QJsonArray &j_arr_coin = j_coin.toArray();
    if (j_arr_coin.isEmpty())  {emit signalError("BB_BagStatePage: j_arr_coin QJsonArray is empty"); return;}

    QJsonObject j_wallet = j_arr.at(0).toObject();
    m_state.balance = j_wallet.value("totalEquity").toString().toFloat();
    m_state.balance_free = j_wallet.value("totalAvailableBalance").toString().toFloat();

    updateTable();
    m_table->resizeByContents();
}
void BB_BagStatePage::updateTable()
{
    QTableWidget *t = m_table->table();
    int row = 0;
    t->item(row, 0)->setText(QString("%1/%2").arg(QString::number(m_state.balance, 'f', 1)).arg(QString::number(m_state.balance_free, 'f', 1))); row++;
    t->item(row, 0)->setText(QString::number(m_state.n_pos)); row++;
    t->item(row, 0)->setText(QString::number(m_state.n_order)); row++;
    t->item(row, 0)->setText(QString("%1/%2").arg(QString::number(m_state.freezed_pos, 'f', 1)).arg(QString::number(m_state.freezed_order, 'f', 1))); row++;
    t->item(row, 0)->setText(QString::number(m_state.sumFreezed(), 'f', 1)); row++;
    t->item(row, 0)->setText(QString::number(m_state.pos_result, 'f', 1));
    if (m_state.pos_result < -0.5) t->item(row, 0)->setTextColor(Qt::red);
    else if (m_state.pos_result > 0.5) t->item(row, 0)->setTextColor(Qt::blue);
    else t->item(row, 0)->setTextColor(Qt::gray);

}

