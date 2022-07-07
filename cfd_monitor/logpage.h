#ifndef LOGPAGE_H
#define LOGPAGE_H

#include "basepage.h"
#include "ui_logpage.h"

#include <QDateTime>

enum AppModuleType {amtMainWindow = 820, amtTGBot, amtHtmlPage, amtCalcObj};

//LogStruct
struct LogStruct
{
    LogStruct();
    LogStruct(int, int);


    QDateTime dt;
    int module;
    QString msg;
    QString desc;
    int status; //0-ok, 1-err, 2-warning

    bool invalid() const;
    QString strDate() const;
    QString strTime() const;
    QString strModule() const;
    QString strStatus() const;
    QColor logColor() const;



};



//LogPage
class LogPage : public BasePage, Ui::LogPage
{
    Q_OBJECT
public:
    LogPage(QWidget*);
    virtual ~LogPage() {}

    QString iconPath() const {return QString(":/icons/images/log.png");}
    QString caption() const {return QString("Log");}

    void updatePage() {}

protected:
    void initModulesList();
    void initLogTable();
    QStringList headerLabels() const;

public slots:
    void slotNewLog(const LogStruct&);


};


#endif //LOGPAGE_H


