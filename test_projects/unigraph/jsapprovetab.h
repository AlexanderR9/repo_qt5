#ifndef JSAPPROVE_TAB_H
#define JSAPPROVE_TAB_H

#include "ug_basepage.h"

class LTableWidgetBox;
class QJsonObject;

//JSApproveTab
class JSApproveTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSApproveTab(QWidget*);
    virtual ~JSApproveTab() {}

    void setTokens(const QMap<QString, QString>&);
    void parseJSResult(const QJsonObject&);

    inline QString scriptName() const {return QString("qt_approve.js");}

protected:
    LTableWidgetBox     *m_table;

    void initPopupMenu(); //инициализировать элементы всплывающего меню
    void updateSuppliedCell(int i, int j, float value);
    void answerUpdate(const QJsonObject&);
    void answerApprove(const QJsonObject&);

protected slots:
    void slotUpdateApproved();
    void slotSendApprove();

signals:
    void signalCheckUpproved(QString);
    void signalApprove(const QStringList&);

};



#endif // JSAPPROVE_TAB_H
