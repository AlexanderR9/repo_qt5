#include "jswallettab.h"
#include "ltable.h"
#include "lfile.h"
#include "lstring.h"
#include "subcommonsettings.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDir>
#include <QJsonObject>
#include <QJsonValue>


#define JS_ASSETS_FILE      "token.txt"
#define ADDRESS_COL         2
#define TOKEN_COL           1
#define BALANCES_COL        4

// JSWalletTab
JSWalletTab::JSWalletTab(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_table(NULL)
{
    setObjectName("js_wallet_tab");

    //init table
    m_table = new LTableWidgetBox(this);
    m_table->setTitle("Assets list");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Number" << "Token" << "Address" << "Decimal" << "Amount";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_table->setSelectionColor("#BDFF4F");

    v_splitter->addWidget(m_table);
}
void JSWalletTab::getBalacesArgs(QStringList &nodejs_args)
{
    nodejs_args.clear();
    if (assetsCount() == 0)
    {
        emit signalError("Assets list is empty");
        return;
    }

    nodejs_args << "get_balance.js" << QString::number(2);
}
void JSWalletTab::initNativeToken()
{
    QStringList token_data;
    token_data << SubGraph_CommonSettings::nativeTokenAddress();
    token_data << SubGraph_CommonSettings::nativeTokenByChain("polygon");
    token_data << "polygon" << QString::number(18);

    addTokenToTable(token_data);
}
int JSWalletTab::assetsCount() const
{
    return m_table->table()->rowCount();
}
QStringList JSWalletTab::assetsTokens() const
{
    QStringList list;
    QTableWidget *t = m_table->table();

    int n = assetsCount();
    for (int i=0; i<n; i++)
    {
        list.append(t->item(i, TOKEN_COL)->text().trimmed());
    }
    return list;
}
void JSWalletTab::loadAssetsFromFile()
{
    m_table->removeAllRows();

    QString fname = QString("%1%2%3").arg(sub_commonSettings.nodejs_path).arg(QDir::separator()).arg(JS_ASSETS_FILE);
    emit signalMsg(QString("try load wallet assets[%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("tokens file not found");
        return;
    }

    initNativeToken();

    QStringList fdata;
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {emit signalError(err); return;}

    foreach (const QString &v, fdata)
    {
        QString fline = v.trimmed();
        if (fline.isEmpty()) continue;
        if (fline.left(1) == "#") continue;

        QStringList token_data = LString::trimSplitList(fline, "/");
        addTokenToTable(token_data);
    }

    m_table->resizeByContents();
}
void JSWalletTab::addTokenToTable(const QStringList &token_data)
{
    if (token_data.count() != 4)
    {
        emit signalError(QString("token file line is invalid [%1]").arg(LString::uniteList(token_data, " / ")));
        return;
    }

    QTableWidget *t = m_table->table();
    QStringList row_data;
    QString t_addr = token_data.first().trimmed().toLower();
    row_data << QString::number(t->rowCount()+1) << token_data.at(1) << t_addr;
    row_data << token_data.last() << "?";
    LTable::addTableRow(t, row_data);

    t->item(t->rowCount()-1, ADDRESS_COL)->setTextColor("#4682B4");
    t->item(t->rowCount()-1, BALANCES_COL)->setTextColor(Qt::gray);
}
void JSWalletTab::updateBalances(const QJsonObject &j_result)
{
    QStringList keys(j_result.keys());
    foreach (const QString &t_addr, keys)
    {
        QString value = j_result.value(t_addr).toString();
        updateTokenBalance(t_addr, value);
    }
    m_table->resizeByContents();
}
void JSWalletTab::updateTokenBalance(const QString &t_addr, const QString &value)
{
    QTableWidget *t = m_table->table();
    int n = assetsCount();
    for (int i=0; i<n; i++)
    {
        QString row_addr = t->item(i, ADDRESS_COL)->text();
        if (row_addr == t_addr.trimmed().toLower())
        {
            bool ok;
            float fb = value.toFloat(&ok);
            if (!ok || fb < 0)
            {
                t->item(i, BALANCES_COL)->setText("invalid");
                t->item(i, BALANCES_COL)->setTextColor(Qt::red);
            }
            else
            {
                t->item(i, BALANCES_COL)->setText(value);
                t->item(i, BALANCES_COL)->setTextColor((fb > 0.1) ? Qt::darkGreen : Qt::gray);
            }
            break;
        }
    }
}


