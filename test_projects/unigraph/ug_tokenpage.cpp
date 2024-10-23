#include "ug_tokenpage.h"
#include "subcommonsettings.h"
#include "ltable.h"

#include <QTableWidget>
#include <QSplitter>
#include <QDebug>



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
    headers << "Address" << "Tiker" << "Chain";
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

    //clearPage();

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
    updateTableData();


}
void UG_TokenPage::startUpdating(quint16 t)
{
    UG_BasePage::startUpdating(t);
    clearPage();
    m_reqData->query.clear();
    updateDataPage(true);
    //emit signalGetFilterParams(m_reqLimit, m_minTVL);
    //m_skip = 0;
    emit signalStopUpdating();
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
