#ifndef PARAMSPAGE_H
#define PARAMSPAGE_H

#include "ui_paramspage.h"

class QSettings;

//ParamsPage
class ParamsPage : public QWidget, public Ui::ParamsPage
{
    Q_OBJECT
public:
    ParamsPage(QWidget*);
    virtual ~ParamsPage() {}

    QString commandName() const;
    QStringList getArgs() const;
    bool isSudo() const;

    void save(QSettings&);
    void load(QSettings&);
    void loadISOList(const QString&);
    QString seletedISOFile() const;

protected:
    QStringList headerLabels() const;
    void initTable();

signals:
    void signalError(const QString&);


};


#endif //LOGPAGE_H


