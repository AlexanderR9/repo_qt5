#ifndef BASEPAGE_H
#define BASEPAGE_H

#include <QWidget>

class QSettings;

// BasePage
class BasePage : public QWidget
{
    Q_OBJECT
public:
    enum PageType {ptCFDStat = 81, ptLog, ptConfig, ptHtml};

    BasePage(QWidget*);
    virtual ~BasePage() {}

    virtual QString iconPath() const = 0;
    virtual QString caption() const = 0;
    virtual void save(QSettings&) {}
    virtual void load(QSettings&) {}
    virtual void updatePage() = 0;

signals:
    void signalError(const QString&);
    void signalMsg(const QString&);

};


#endif //BASEPAGE_H


