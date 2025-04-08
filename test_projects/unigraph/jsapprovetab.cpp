#include "jsapprovetab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QJsonObject>
#include <QJsonValue>

/*
#define JS_ASSETS_FILE      "token.txt"
#define ADDRESS_COL         2
#define BALANCES_COL        4
*/

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
    headers << "Token" << "POS_MANAGER" << "SWAP_ROUTER";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_table->setSelectionColor("#BDFF4F");

    v_splitter->addWidget(m_table);
}
void JSApproveTab::setTokens(const QStringList &list)
{
    m_table->removeAllRows();
    foreach (const QString &t_name, list)
    {
        if (t_name == SubGraph_CommonSettings::nativeTokenByChain("polygon"))  continue;
        QStringList row_data;
        row_data << t_name << "?" << "?";
        LTable::addTableRow(m_table->table(), row_data);
    }
    m_table->resizeByContents();
}


