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
    void reloadISOList(const QString&);
    QString seletedISOFile() const;
    void resetColors();
    void findMD5(const QString&);

    static QString md5File() {return QString("md5_iso.txt");}

protected:
    QStringList headerLabels() const;
    void initTable();
    void updateMD5Column(const QString&);

signals:
    void signalError(const QString&);


};


#endif //LOGPAGE_H


