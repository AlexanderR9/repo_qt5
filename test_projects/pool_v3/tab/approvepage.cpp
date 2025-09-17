#include "approvepage.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "ltable.h"
#include "txdialog.h"
#include "txlogrecord.h"
#include "nodejsbridge.h"


#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>


#define ADDRESS_COL             3
#define TOKEN_COL               0


// JSApproveTab
DefiApproveTabPage::DefiApproveTabPage(QWidget *parent)
    :BaseTabPage_V3(parent, 20, dpkApproved)
{
    setObjectName("defi_approve_tab_page");

    //init table
    initTable();

    // init popup
    initPopupMenu();
}
void DefiApproveTabPage::updatePageBack(QString extra_data)
{
    qDebug()<<QString("DefiApproveTabPage::updatePageBack  extra_data[%1]").arg(extra_data);

    m_table->table()->clearSelection();
    selectRowByCellData(extra_data.trimmed(), ADDRESS_COL);
    slotGetApproved();
}
void DefiApproveTabPage::initTable()
{
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Supplied amounts information");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Token" << "POS_MANAGER" << "SWAP_ROUTER" << "Address";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#BDFF4F");

    h_splitter->addWidget(m_table);
    m_table->resizeByContents();
}
void DefiApproveTabPage::setChain(int cid)
{
    BaseTabPage_V3::setChain(cid);
    initTokenList(cid);
}
void DefiApproveTabPage::initTokenList(int cid)
{
    QTableWidget *t = m_table->table();
    m_table->removeAllRows();
    QStringList row_data;
    foreach (const DefiToken &v, defi_config.tokens)
    {
        if (v.chain_id == cid)
        {
            row_data.clear();
            row_data << v.name << "?" << "?" << v.address;
            LTable::addTableRow(t, row_data);
            t->item(t->rowCount()-1, 1)->setTextColor(Qt::red);
            t->item(t->rowCount()-1, 2)->setTextColor(Qt::red);
        }
    }
    m_table->resizeByContents();
}
void DefiApproveTabPage::initPopupMenu()
{
    QString path = AppCommonSettings::commonIconsPath(); // icons path

    //prepare menu actions data for assets table
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Get approved", QString("%1/view-refresh.svg").arg(path));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Approve asset", TxDialogBase::iconByTXType(txApprove));
    act_list.append(pair2);

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetApproved())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTxApprove())); i_menu++;
}
void DefiApproveTabPage::slotGetApproved()
{
    qDebug("DefiApproveTabPage::slotGetApproved()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("DefiWalletTabPage: You must select row"); return;}
    QTableWidget *t = m_table->table();

    QJsonObject j_params;
    j_params.insert(AppCommonSettings::nodejsReqFieldName(), NodejsBridge::jsonCommandValue(nrcApproved));
    j_params.insert("token_address", t->item(row, ADDRESS_COL)->text().trimmed());
    sendReadNodejsRequest(j_params);
}
void DefiApproveTabPage::slotTxApprove()
{
    qDebug("DefiApproveTabPage::slotTxApprove()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("DefiApproveTabPage: You must select row"); return;}
    QTableWidget *t = m_table->table();

    TxDialogData data(txApprove, curChainName());
    data.token_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    TxApproveDialog d(data, this);

    d.exec();
    if (d.isApply()) sendTxNodejsRequest(data);
    else emit signalMsg("operation was canceled!");
}
void DefiApproveTabPage::slotNodejsReply(const QJsonObject &js_reply)
{
    QString req = js_reply.value(AppCommonSettings::nodejsReqFieldName()).toString();
    qDebug()<<QString("DefiApproveTabPage::slotNodejsReply - req kind: [%1]").arg(req);

    if (req == NodejsBridge::jsonCommandValue(nrcApproved)) updateApprovedAmounts(js_reply);
    else if (req == NodejsBridge::jsonCommandValue(txApprove)) checkTxResult(req, js_reply);
    else return;

    m_table->resizeByContents();
}
void DefiApproveTabPage::updateApprovedAmounts(const QJsonObject &js_reply)
{
    qDebug("DefiApproveTabPage::updateApprovedAmounts");
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();

    QString t_addr = js_reply.value("token_address").toString().trimmed();
    for (int i=0; i<n_rows; i++)
    {
        if (t->item(i, ADDRESS_COL)->text().trimmed() == t_addr)
        {
            qDebug()<<QString("find asset row: %1 / %2").arg(i).arg(t->item(i, TOKEN_COL)->text());
            float pm = js_reply.value("pos_manager").toString().toFloat();
            float sr = js_reply.value("swap_router").toString().toFloat();
            t->item(i, 1)->setText(js_reply.value("pos_manager").toString());
            t->item(i, 2)->setText(js_reply.value("swap_router").toString());

            if (pm <= 0.01) t->item(i, 1)->setTextColor(Qt::lightGray);
            else t->item(i, 1)->setTextColor(Qt::darkGreen);
            if (sr <= 0.01) t->item(i, 2)->setTextColor(Qt::lightGray);
            else t->item(i, 2)->setTextColor(Qt::darkGreen);
            break;
        }
    }
}
void DefiApproveTabPage::checkTxResult(QString req, const QJsonObject &js_reply)
{
    bool is_simulate = (js_reply.value(AppCommonSettings::nodejsTxSimulateFieldName()).toString() == "yes");
    emit signalMsg("");
    emit signalMsg(QString("//////////// REPLY form TX request [%1] ///////////////").arg(req));
    int result_code = js_reply.value("code").toString().toInt();
    emit signalMsg(QString("simulate_mode = %1").arg(is_simulate?"YES":"NO"));
    emit signalMsg(QString("result_code = %1").arg(result_code));

    if (is_simulate)
    {
        emit signalMsg(QString("estimated_gas = %1").arg(js_reply.value("estimated_gas").toString()));
    }
    else if (js_reply.contains(AppCommonSettings::nodejsTxHashFieldName()))
    {
        logTxRecord(req, js_reply);
    }
}
void DefiApproveTabPage::logTxRecord(QString req, const QJsonObject &js_reply)
{
    TxLogRecord tx_rec(req, curChainName());
    tx_rec.tx_hash = js_reply.value(AppCommonSettings::nodejsTxHashFieldName()).toString().trimmed().toLower();
    tx_rec.wallet.token_addr = js_reply.value("token_address").toString().trimmed();
    tx_rec.wallet.token_amount = js_reply.value("amount").toString().toFloat();
    tx_rec.wallet.contract_addr = js_reply.value("to_contract").toString();
    //tx_rec.note = QString("approve %1").arg(tickerByAddress(tx_rec.wallet.token_addr));
    tx_rec.formNote(tickerByAddress(tx_rec.wallet.token_addr));


    emit signalNewTx(tx_rec);
}
QString DefiApproveTabPage::tickerByAddress(const QString &t_addr) const
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    for (int i=0; i<n_rows; i++)
    {
        if (t->item(i, ADDRESS_COL)->text().trimmed() == t_addr)
            return t->item(i, TOKEN_COL)->text().trimmed();
    }
    return QString();
}



