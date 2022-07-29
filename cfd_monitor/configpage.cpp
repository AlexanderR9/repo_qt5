#include "configpage.h"
#include "ltable.h"
#include "lsearch.h"


ConfigPage::ConfigPage(QWidget *parent)
    :BasePage(parent),
    m_search(NULL)
{
    setupUi(this);

    initSearch();

}
void ConfigPage::slotSetUrlByTicker(const QString &ticker, QString &url)
{
    url.clear();
    for (int i=0; i<cfdTable->rowCount(); i++)
    {
        if (cfdTable->item(i, 1)->text() == ticker)
        {
            url = cfdTable->item(i, 3)->text().trimmed();
            break;
        }
    }
}
void ConfigPage::slotSetChartSource(QStringList &list)
{
    list.clear();
    for (int i=0; i<cfdTable->rowCount(); i++)
        list << cfdTable->item(i, 1)->text();
}
void ConfigPage::setSourses(const QStringList &data)
{
    sourcesListWidget->clear();
    for (int i=0; i<data.count(); i++)
        sourcesListWidget->addItem(data.at(i));
}
void ConfigPage::setTGBotParams(const QMap<QString, QString> &map)
{
    QStringList keys(map.keys());
    for (int i=0; i<keys.count(); i++)
    {
        QStringList row_data;
        row_data << keys.at(i) << map.value(keys.at(i));
        LTable::addTableRow(tgTable, row_data);
    }

}
void ConfigPage::initSearch()
{
    m_search = new LSearch(searchLineEdit, this);
    m_search->addTable(cfdTable, countLabel);
    m_search->exec();
}
void ConfigPage::reinitCFDTable()
{
    LTable::fullClearTable(cfdTable);
    cfdTable->verticalHeader()->hide();

    QStringList headers;
    headers << QString("N") << QString("Ticker") << QString("Name") << QString("Request URL");
    LTable::setTableHeaders(cfdTable, headers);
}
void ConfigPage::reinitTGTable()
{
    LTable::fullClearTable(tgTable);
    tgTable->verticalHeader()->hide();

    QStringList headers;
    headers << QString("Parameter") << QString("Value");
    LTable::setTableHeaders(tgTable, headers);
}
void ConfigPage::addCFDObject(const QStringList &row_data)
{
    LTable::addTableRow(cfdTable, row_data);
}
void ConfigPage::updatePage()
{
    m_search->exec();
    tgTable->resizeColumnsToContents();
    tgTable->resizeRowsToContents();
    cfdTable->resizeRowsToContents();
}

