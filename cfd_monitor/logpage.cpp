#include "logpage.h"
#include "ltable.h"


#include <QDebug>

//LogPage
LogPage::LogPage(QWidget *parent)
    :BasePage(parent)
{
    setupUi(this);

    initModulesList();
    initLogTable();

}
void LogPage::initModulesList()
{
    modulesListWidget->addItem("Calc object");
    modulesListWidget->addItem("TG bot");
    modulesListWidget->addItem("HTML page");
    modulesListWidget->addItem("Main window");

}
void LogPage::initLogTable()
{
    logTable->verticalHeader()->hide();

    LTable::fullClearTable(logTable);
    LTable::setTableHeaders(logTable, headerLabels());
    LTable::resizeTableContents(logTable);

}
void LogPage::slotNewLog(const LogStruct &log)
{
    if (log.invalid())
    {
        return;
        qWarning()<<QString("LogPage::slotNewLog - received invalid log!");
    }

    QStringList row_data;
    row_data.append(log.strDate());
    row_data.append(log.strTime());
    row_data.append(log.strModule());
    row_data.append(log.msg);
    row_data.append(log.strStatus());
    LTable::addTableRow(logTable, row_data);
    logTable->item(logTable->rowCount()-1, logTable->columnCount()-1)->setTextColor(log.logColor());

    LTable::resizeTableContents(logTable);
}
QStringList LogPage::headerLabels() const
{
    QStringList list;
    list << "Date" << "Time" << "Module" << "Message" << "Status";
    return list;
}



//LogStruct
LogStruct::LogStruct(int m, int code)
    :module(m),
    status(code)
{
    dt = QDateTime::currentDateTime();
    msg.clear();
    desc.clear();
}
LogStruct::LogStruct()
    :module(-1),
    status(-1)
{
    dt = QDateTime();
    msg.clear();
    desc.clear();
}
bool LogStruct::invalid() const
{
    if (dt.isNull() || !dt.isValid()) return true;
    if (msg.isEmpty()) return true;
    if (status < 0 || status > 2) return true;
    if (module < amtMainWindow || module > amtCalcObj) return true;
    return false;
}
QString LogStruct::strDate() const
{
    return dt.date().toString("dd.MM.yyyy");
}
QString LogStruct::strTime() const
{
    return dt.time().toString("hh:mm:ss");
}
QString LogStruct::strModule() const
{
    switch (module)
    {
        case amtMainWindow: return "main_window";
        case amtCalcObj: return "calc_object";
        case amtHtmlPage: return "html_page";
        case amtTGBot: return "tg_bot";
        default: break;
    }
    return "??";
}
QString LogStruct::strStatus() const
{
    switch (status)
    {
        case 0: return "OK";
        case 1: return "ERROR";
        case 2: return "WARNIMG";
        default: break;
    }
    return "??";
}
QColor LogStruct::logColor() const
{
    switch (status)
    {
        case 0: return Qt::blue;
        case 1: return Qt::darkRed;
        case 2: return Qt::darkYellow;
        default: break;
    }
    return "??";
}


