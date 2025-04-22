#include "jspooltab.h"
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

#define JS_POOL_FILE          "pools.txt"
#define FEE_COL                 3
#define POOL_ADDR_COL           1


// JSPoolTab
JSPoolTab::JSPoolTab(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_table(NULL)
{
    setObjectName("js_pools_tab");

    initTable();
    initPopupMenu();
}
void JSPoolTab::initPopupMenu()
{
    //prepare menu actions data
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Get state", QString("%1/r_scale.svg").arg(SubGraph_CommonSettings::commonIconsPath()));
    act_list.append(pair1);
    QPair<QString, QString> pair2("Swap assets", TxDialogBase::iconByTXType(txSwap));
    act_list.append(pair2);

    //init popup menu actions
    m_table->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotGetPoolState())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotTrySwapAssets())); i_menu++;

}
void JSPoolTab::slotGetPoolState()
{
    qDebug("JSPoolTab::slotGetPoolState()");
}
void JSPoolTab::slotTrySwapAssets()
{
    qDebug("JSPoolTab::slotTrySwapAssets()");

}
void JSPoolTab::initTable()
{
    m_table = new LSearchTableWidgetBox(this);
    m_table->setTitle("Pools list");
    m_table->vHeaderHide();
    QStringList headers;
    headers << "Chain" << "Pool address" << "Assets" << "Fee" << "Tick space" << "Current state";
    LTable::setTableHeaders(m_table->table(), headers);
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    m_table->setSelectionColor("#9D9F4F");

    v_splitter->addWidget(m_table);
}
void JSPoolTab::loadPoolsFromFile()
{
    qDebug("-----------------JSPoolTab::loadPoolsFromFile()--------------------------");
    m_poolData.clear();
    QString fname = QString("%1%2%3").arg(sub_commonSettings.nodejs_path).arg(QDir::separator()).arg(JS_POOL_FILE);
    emit signalMsg(QString("try load pools list [%1].........").arg(fname));
    if (!LFile::fileExists(fname))
    {
        emit signalError("POOL file not found");
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

        JSPoolRecord rec;
        rec.fromFileLine(fline);
        if (!rec.invalid()) m_poolData.append(rec);
    }
    emit signalMsg(QString("loaded %1 POOL records").arg(m_poolData.count()));
    qDebug()<<QString("loaded %1 POOL records").arg(m_poolData.count());

    reloadTable();
}
void JSPoolTab::reloadTable()
{
    m_table->removeAllRows();
    if (m_poolData.isEmpty()) {m_table->searchExec(); return;}

    QTableWidget *t = m_table->table();
    QStringList row_data;
    for (int i=0; i<m_poolData.count(); i++)
    {
        row_data.clear();
        const JSPoolRecord &rec = m_poolData.at(i);
        row_data << rec.chain << rec.address << rec.assets << rec.strFee();
        row_data << QString::number(rec.tickSpace()) << "?";
        LTable::addTableRow(t, row_data);

        if (rec.fee == 100) t->item(i, FEE_COL)->setTextColor(Qt::gray);
        else if (rec.fee == 500) t->item(i, FEE_COL)->setTextColor(Qt::darkGreen);
        else if (rec.fee == 3000) t->item(i, FEE_COL)->setTextColor(Qt::blue);
        else t->item(i, FEE_COL)->setTextColor(Qt::red);

        t->item(i, POOL_ADDR_COL)->setTextColor("#4682B4");
    }

    m_table->searchExec();
    m_table->resizeByContents();
    qDebug()<<QString("JSPoolTab: POOL table rows %1").arg(t->rowCount());
}
///////////////////////////JSPoolRecord//////////////////////////////

void JSPoolRecord::reset()
{
    address.clear();
    fee = 0;
    token0_addr.clear();
    token1_addr.clear();
    chain = QString("POLYGON");
    assets.clear();
}
bool JSPoolRecord::invalid() const
{
    if (address.isEmpty() || token0_addr.isEmpty() || token1_addr.isEmpty()) return true;
    if (address.length()<30 || token0_addr.length()<30 || token1_addr.length()<30) return true;
    if (fee != 100 && fee != 500 && fee != 3000 && fee != 10000) return true;
    if (assets.isEmpty() || !assets.contains("/")) return true;
    return false;
}
void JSPoolRecord::fromFileLine(const QString &fline)
{
    qDebug()<<QString("fromFileLine [%1]").arg(fline);
    reset();
    QString s = fline.trimmed();
    if (s.isEmpty()) return;

    QStringList list = LString::trimSplitList(s, "/");
    if (list.count() != 5) {qWarning()<<QString("JSPoolRecord::fromFileLine WARNING list.count(%1)!=5").arg(list.count()); return;}
   // qDebug()<<QString("fromFileLine list.count()[%1]").arg(list.count());


    int i = 0;
    address = list.at(i).trimmed().toLower(); i++;
    assets =  list.at(i).trimmed(); i++;
    QStringList desc_list = LString::trimSplitList(assets, ":");
    if (desc_list.count() == 3) assets = QString("%1 / %2").arg(desc_list.at(0).trimmed()).arg(desc_list.at(1).trimmed());
    token0_addr = list.at(i).trimmed().toLower(); i++;
    token1_addr = list.at(i).trimmed().toLower(); i++;
    fee = list.at(i).trimmed().toUInt(); i++;
}
QString JSPoolRecord::strFee() const
{
    switch (fee)
    {
        case 100: return QString("0.01%");
        case 500: return QString("0.05%");
        case 3000: return QString("0.3%");
        case 10000: return QString("1.0%");
        default: break;
    }
    return QString("?");
}
int JSPoolRecord::tickSpace() const
{
    switch (fee)
    {
        case 100: return 2;
        case 500: return 10;
        case 3000: return 60;
        case 10000: return 200;
        default: break;
    }
    return -1;
}
