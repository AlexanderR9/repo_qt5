#include "ug_tokenpage.h"
#include "subcommonsettings.h"
#include "ltable.h"
#include "lfile.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>



//UG_PoolPage
UG_TokenPage::UG_TokenPage(QWidget *parent)
    :UG_BasePage(parent, 10, rtTokens),
      m_tableBox(NULL)
      //m_reqLimit(70),
      //m_minTVL(2000),
      //m_skip(0)
{
    setObjectName("ug_token_page");

    initTable();

}
void UG_TokenPage::initTable()
{
    m_tableBox = new LSearchTableWidgetBox(this);

    QStringList headers;
    headers << "Address" << "Tiker" << "Full name" << "Chain" << "Supply" << "TVL, M (usd)" << "All fees, M (usd)";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Token list");
    m_tableBox->setObjectName("table_box");
    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    v_splitter->addWidget(m_tableBox);

    /*
    m_tableBox->sortingOn();
    m_tableBox->addSortingData(1, LTableWidgetBox::sdtNumeric);
    m_tableBox->addSortingData(2, LTableWidgetBox::sdtNumeric);
    m_tableBox->addSortingData(3, LTableWidgetBox::sdtString);
    m_tableBox->addSortingData(4, LTableWidgetBox::sdtString);
    */

}
void UG_TokenPage::clearPage()
{
    qDebug("UG_TokenPage::clearPage");

    m_tokens.clear();
    m_tableBox->removeAllRows();
    m_tableBox->resizeByContents();
}
void UG_TokenPage::updateDataPage(bool forcibly)
{
    qDebug("UG_TokenPage::updateDataPage");
    qDebug()<<QString("forcibly: %1").arg(forcibly?"true":"false");
    if (!forcibly) return;


    //get tokens from pool page
    /*
    QHash<QString, QString> map;
    emit signalGetTokensFromPoolPage(map);

    QString cur_chain = sub_commonSettings.factories.at(sub_commonSettings.cur_factory).chain.trimmed();
    QHash<QString, QString>::const_iterator it = map.constBegin();
    while (it != map.constEnd())
    {
        UG_TokenInfo t;
        t.address = it.key();
        t.ticker = it.value();
        t.chain = cur_chain;
        if (!t.invalid()) m_tokens.append(t);
        it++;
    }
    */

    //get tokens from all pool_files
    loadAllPoolFiles();

}
void UG_TokenPage::startUpdating(quint16 t)
{    
    UG_BasePage::startUpdating(t);
    //clearPage();
    m_reqData->query.clear();
    findCurChainTokens();
    qDebug()<<QString("found cur_chain tokens %1").arg(m_updatedTokens.count());
    if (m_updatedTokens.isEmpty())
        emit signalError("Not found tokens for updating");

    //updateDataPage(true);
    //updateTableData();
    //emit signalStopUpdating();


}
void UG_TokenPage::findCurChainTokens()
{
    m_updatedTokens.clear();
    if (m_tokens.isEmpty()) return;

    QString cur_chain = sub_commonSettings.curChain().trimmed().toLower();
    foreach (const UG_TokenInfo &t, m_tokens)
        if (t.chain.trimmed().toLower() == cur_chain) m_updatedTokens.append(t.address);

    emit signalMsg(QString("finded %1 tokens for chain %2").arg(m_updatedTokens.count()).arg(cur_chain.toUpper()));
}
void UG_TokenPage::updateTableData()
{
    QTableWidget *tw = m_tableBox->table();
    int start_index = tw->rowCount();
    if (m_tokens.isEmpty())
    {
        qDebug()<<QString("WARNIG tokens data is empty");
        return;
    }

    if (m_tokens.count() > start_index)
    {
        for (int i=start_index; i<m_tokens.count(); i++)
        {
            QStringList row_data;
            m_tokens.at(i).toTableRow(row_data);
            LTable::addTableRow(tw, row_data);
        }
        m_tableBox->resizeByContents();
        m_tableBox->searchExec();
    }
    qDebug()<<QString("m_tokens size %1, table row count %2").arg(m_tokens.count()).arg(tw->rowCount());
}
void UG_TokenPage::loadAllPoolFiles()
{
    QString fname("pools");
    for (int i=0; i<sub_commonSettings.factories.count(); i++)
    {
        QString chain = sub_commonSettings.factories.at(i).chain.toLower().trimmed();
        QString postfix = QString("%1").arg((chain.length()>4) ? chain.left(3) : chain);
        QString full_name = QString("%1_%2.txt").arg(fname).arg(postfix);

        emit signalMsg(QString());
        emit signalMsg(QString("try load tokens from: %1").arg(full_name));
        full_name = QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(full_name);

        QStringList fdata;
        QString err = LFile::readFileSL(full_name, fdata);
        if (err.isEmpty()) loadFromFileData(fdata, chain);
        else signalError(err);
    }
}
void UG_TokenPage::loadFromFileData(const QStringList &fdata, const QString &chain)
{
    int n = 0;
    foreach (const QString v, fdata)
    {
        if (v.trimmed().isEmpty()) continue;

        UG_PoolInfo pool;
        pool.fromFileLine(v);
        if (pool.invalid()) continue;

        UG_TokenInfo t0(pool.token0_id, pool.token0, chain.toUpper());
        if (!t0.invalid() && !tokenContains(t0.address)) {m_tokens.append(t0); n++;}
        UG_TokenInfo t1(pool.token1_id, pool.token1, chain.toUpper());
        if (!t1.invalid() && !tokenContains(t1.address)) {m_tokens.append(t1); n++;}
    }
    emit signalMsg(QString("loaded %1 records").arg(n));
}
bool UG_TokenPage::tokenContains(QString addr) const
{
    if (m_tokens.isEmpty()) return false;

    foreach (const UG_TokenInfo t, m_tokens)
        if (t.address == addr) return true;
    return false;
}
void UG_TokenPage::saveData()
{
    if (m_tokens.isEmpty())
    {
        emit signalError("Tokens container is empty.");
        return;
    }

    QString err;
    QString fname(dataFile());
    QStringList fdata;
    fdata << QString("TOKENS INFO:");

    foreach (const UG_TokenInfo &t, m_tokens)
        fdata << t.toFileLine();
    fdata.append(QString());

    emit signalMsg(QString("filiname[%1]  datasize[%2]").arg(fname).arg(fdata.count()));
    err = LFile::writeFileSL(fname, fdata);
    if (!err.isEmpty()) emit signalError(err);
    else emit signalMsg("Ok!");
}
void UG_TokenPage::loadData()
{
    //qDebug("UG_TokenPage::loadData()");
    clearPage();
    emit signalMsg(QString("try load pools data from file [%1] ........").arg(dataFile()));

    QStringList fdata;
    QString err = LFile::readFileSL(dataFile(), fdata);
    if (!err.isEmpty())
    {
        emit signalError(err);
        return;
    }

    foreach (const QString v, fdata)
    {
        if (v.trimmed().isEmpty()) continue;

        UG_TokenInfo t;
        t.fromFileLine(v);
        if (!t.invalid()) m_tokens.append(t);
    }
    emit signalMsg(QString("loaded %1 records").arg(m_tokens.count()));

    updateTableData();
}
QString UG_TokenPage::dataFile() const
{
    QString fname("tokens.txt");
    return QString("%1%2%3").arg(SubGraph_CommonSettings::appDataPath()).arg(QDir::separator()).arg(fname);
}
void UG_TokenPage::slotTimer()
{
    if (m_updatedTokens.isEmpty())
    {
        emit signalMsg("Updating tokens data finished!");
        emit signalStopUpdating();
        m_tableBox->removeAllRows();
        updateTableData();
        return;
    }

    UG_BasePage::slotTimer();
    emit signalMsg(QString("try next query[%1] ......").arg(m_updatedTokens.first()));
    prepareQuery();
    sendRequest();
}
void UG_TokenPage::prepareQuery()
{
    QString s_fields = QString();
    s_fields = QString("%1 totalSupply").arg(s_fields);
    s_fields = QString("%1 totalValueLockedUSD").arg(s_fields);
    s_fields = QString("%1 name").arg(s_fields);
    s_fields = QString("%1 feesUSD").arg(s_fields);

    QString s_tag = QString("token(id: \"%1\")").arg(m_updatedTokens.first());
    m_reqData->query = QString("{%1 {%2}}").arg(s_tag).arg(s_fields);
}
void UG_TokenPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != userSign()) return;
    qDebug()<<QString("UG_TokenPage::slotJsonReply  req_type=%1, OK!").arg(req_type);

    //m_updatedTokens.clear();
    const QJsonValue &j_data = j_obj.value("data").toObject().value("token");
    if (j_data.isNull()) {emit signalError("UG_TokenPage: result QJsonValue <data/token> not found"); return;}

    updateTokenData(m_updatedTokens.first(), j_data.toObject());
    m_updatedTokens.removeFirst();
}
void UG_TokenPage::updateTokenData(const QString &addr, const QJsonObject &j_obj)
{
    for (int i=0; i<m_tokens.count(); i++)
    {
        UG_TokenInfo &t = m_tokens[i];
        if (t.address == addr)
        {
            t.name = j_obj.value("name").toString();
            t.total_supply = j_obj.value("totalSupply").toString().toDouble();
            t.tvl = j_obj.value("totalValueLockedUSD").toString().toDouble();
            t.collected_fees = j_obj.value("feesUSD").toString().toDouble();
            break;
        }
    }
}

