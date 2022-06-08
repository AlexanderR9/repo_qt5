#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include "basepage.h"

#include "ui_configpage.h"

class LSearch;

//ConfigPage
class ConfigPage : public BasePage, Ui::ConfigPage
{
    Q_OBJECT
public:
    ConfigPage(QWidget*);
    virtual ~ConfigPage() {}

    QString iconPath() const {return QString(":/icons/images/xml.png");}
    QString caption() const {return QString("Configuration");}

    void setSourses(const QStringList&);
    void setTGBotParams(const QMap<QString, QString>&);
    void reinitCFDTable();
    void reinitTGTable();
    void addCFDObject(const QStringList&);
    void updatePage();

protected:
    LSearch     *m_search;


    void initSearch();

public slots:
    void slotSetUrlByTicker(const QString&, QString&);

};


#endif //CONFIGPAGE_H


