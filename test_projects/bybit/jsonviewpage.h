#ifndef JSON_VIEWPAGE_H
#define JSON_VIEWPAGE_H

#include "bb_basepage.h"

class QJsonObject;

//JSONViewPage
class JSONViewPage : public BB_BasePage
{
    Q_OBJECT
public:
    JSONViewPage(QWidget*);
    virtual ~JSONViewPage() {}

    QString iconPath() const {return QString(":/icons/images/tree.png");}
    QString caption() const {return QString("JSON viewer");}
    void setExpandLevel(int);

    void updateDataPage(bool) {}

protected:
    LTreeWidgetBox      *m_replyBox;

    void initWidgets();

public slots:
    void slotJsonReply(int, const QJsonObject&);


};



#endif // JSON_VIEWPAGE_H
