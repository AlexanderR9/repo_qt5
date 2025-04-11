#include "jsapprovetab.h"
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
#include <QJsonValue>


#define ADDRESS_COL             3
#define TOKEN_COL               0
#define SUPPLIED_PRECISION      6

// JSApproveTab
JSApproveTab::JSApproveTab(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_table(NULL)
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
}
void JSApproveTab::setTokens(const QMap<QString, QString> &map)
{
    m_table->removeAllRows();
    QStringList list(map.keys());
    foreach (const QString &t_name, list)
    {
        if (t_name == SubGraph_CommonSettings::nativeTokenByChain("polygon"))  continue;
        QStringList row_data;
        row_data << t_name.trimmed() << "?" << "?" << map.value(t_name).toLower().trimmed();
        LTable::addTableRow(m_table->table(), row_data);
    }
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
    qDebug("JSApproveTab::slotUpdateApproved()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}

    QTableWidget *t = m_table->table();
    qDebug()<<QString("token=%1  addr=%2").arg(t->item(row, TOKEN_COL)->text()).arg(t->item(row, ADDRESS_COL)->text());

    QString token_addr = t->item(row, ADDRESS_COL)->text().trimmed();
    t->setEnabled(false);
    emit signalCheckUpproved(token_addr);
}
void JSApproveTab::slotSendApprove()
{
    qDebug("JSApproveTab::slotSendApprove()");
    int row = m_table->curSelectedRow();
    if (row < 0) {emit signalError("You must select row"); return;}
    QTableWidget *t = m_table->table();
    qDebug()<<QString("token=%1  addr=%2").arg(t->item(row, TOKEN_COL)->text()).arg(t->item(row, ADDRESS_COL)->text());
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

        t->setEnabled(false);
        QStringList tx_params;
        tx_params << token_addr << approve_data.dialog_params.value("whom") << approve_data.dialog_params.value("amount");
        emit signalApprove(tx_params);
    }
    else emit signalMsg("operation was canceled!");
}
void JSApproveTab::parseJSResult(const QJsonObject &j_result)
{
    qDebug("JSApproveTab::parseJSResult");
    qDebug() << j_result;
    m_table->table()->setEnabled(true);

    QString operation = j_result.value("type").toString().trimmed();
    if (operation == "update") answerUpdate(j_result);
    else if (operation == "tx_approve") answerApprove(j_result);
    else emit signalError(QString("invalid answer type: %1").arg(operation));

    m_table->resizeByContents();
}
void JSApproveTab::answerUpdate(const QJsonObject &j_result)
{
    qDebug("JSApproveTab::answerUpdate");
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
            break;
        }
    }
}
void JSApproveTab::answerApprove(const QJsonObject &j_result)
{
    qDebug("JSApproveTab::answerApprove");
    QString code = j_result.value("result_code").toString().trimmed();

    if (code == "OK")
    {
        QString hash = j_result.value("tx_hash").toString().trimmed();
        emit signalMsg("TX sended OK!!!");
        emit signalMsg(QString("TX_HASH: %1").arg(hash));
    }
    else
    {
        emit signalError(QString("TX sending result: %1").arg(code));
    }
}
void JSApproveTab::updateSuppliedCell(int i, int j, float value)
{
    const QTableWidget *t = m_table->table();
    if (value < 0)
    {
        t->item(i, j)->setText("-1");
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




