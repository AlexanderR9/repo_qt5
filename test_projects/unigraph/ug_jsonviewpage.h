#ifndef UG_JSON_VIEWPAGE_H
#define UG_JSON_VIEWPAGE_H

#include "ug_basepage.h"

class QJsonObject;
class QLineEdit;
class QSettings;


//UG_JSONViewPage
class UG_JSONViewPage : public UG_BasePage
{
    Q_OBJECT
public:
    UG_JSONViewPage(QWidget*);
    virtual ~UG_JSONViewPage() {}

    virtual void load(QSettings&);
    virtual void save(QSettings&);

    QString iconPath() const {return QString(":/icons/images/tree.png");}
    QString caption() const {return QString("JSON viewer");}
    void setExpandLevel(int);

    void updateDataPage(bool) {}
    inline void setPrecision(quint8 p) {m_precision = p;}
    virtual void saveData() {};
    virtual void loadData() {};

    QString freeQueryData() const;

protected:
    LTreeWidgetBox      *m_replyBox;
    quint8               m_precision;
    QLineEdit           *m_reqEdit;

    void initWidgets();
    void initQueryBox();
    void cutPrecision(QTreeWidgetItem*);
    virtual void clearPage();

public slots:
    void slotJsonReply(int, const QJsonObject&);
    void slotReqBuzyNow() {}


};



#endif // UG_JSON_VIEWPAGE_H
