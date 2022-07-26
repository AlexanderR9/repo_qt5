#include "logpage.h"
#include "ltable.h"
#include "lfile.h"


#include <QDebug>
#include <QDir>
#include <QFile>


#define CFD_LOG_FILETYPE   "log"
#define CFD_LOG_FOLDER     "log"


//LogPage
LogPage::LogPage(QWidget *parent)
    :BasePage(parent),
      n_err(0),
      n_maxSize(1000)
{
    setupUi(this);

    initModulesList();
    initLogTable();

}
QString LogPage::filePathByModule(int module_type) const
{
    QString path = QString("%1%2%3").arg(QApplication::applicationDirPath()).arg(QDir::separator()).arg(CFD_LOG_FOLDER);

    QString f_name;
    switch (module_type)
    {
        case amtMainWindow:     {f_name = "main_window"; break;}
        case amtCalcObj:        {f_name = "calc_object"; break;}
        case amtHtmlPage:       {f_name = "html_page"; break;}
        case amtTGBot:          {f_name = "tg_bot"; break;}
        default: return QString();
    }

    return QString("%1%2%3.%4").arg(path).arg(QDir::separator()).arg(f_name).arg(CFD_LOG_FILETYPE);
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
void LogPage::addLogToFile(const LogStruct &log)
{
    QString fname(filePathByModule(log.module));
    if (fname.isEmpty())
    {
        qWarning()<<QString("LogPage::addLogToFile - log_filename is empty!");
        return;
    }

    QString fline = QString("%1 %2 STATUS(%3) - %4 \n").arg(log.strDate()).arg(log.strTime()).arg(log.strStatus()).arg(log.msg);
    QString err = LFile::appendFile(fname, fline);
    if (!err.isEmpty())
    {
        signalError(err);
    }
}
void LogPage::updatePage()
{
    if (logTable->rowCount() > n_maxSize)
    {
        for (int i=0; i<100; i++)
        {
            logTable->removeRow(0);
        }
    }

    QString s = QString("Log (count %1/%2)").arg(logTable->rowCount()).arg(n_err);
    logBox->setTitle(s);
}
void LogPage::slotNewLog(const LogStruct &log)
{
    if (log.invalid())
    {
        qWarning()<<QString("LogPage::slotNewLog - received invalid log!");
        return;
    }

    addLogToFile(log);
    if (log.status != 0) n_err++;

    QStringList row_data;
    row_data.append(log.strDate());
    row_data.append(log.strTime());
    row_data.append(log.strModule());
    row_data.append(log.msg);
    row_data.append(log.strStatus());
    LTable::addTableRow(logTable, row_data);
    logTable->item(logTable->rowCount()-1, logTable->columnCount()-1)->setTextColor(log.logColor());
    LTable::resizeTableContents(logTable);

    updatePage();
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


