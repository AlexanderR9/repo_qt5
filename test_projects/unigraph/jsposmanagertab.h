#ifndef JSPOSMANAGER_TAB_H
#define JSPOSMANAGER_TAB_H

#include "ug_basepage.h"


class QJsonArray;
class QJsonObject;


//JSPosTab
class JSPosManagerTab : public LSimpleWidget
{
    Q_OBJECT
public:
    JSPosManagerTab(QWidget*);
    virtual ~JSPosManagerTab() {}

    void updatePidList(); //запросить из сети список всех PID (поз)
    void parseJSResult(const QJsonObject&); //проверить ответ полученный от  скриптов node_js


protected:
    LSearchTableWidgetBox     *m_tablePos;
    LSearchTableWidgetBox     *m_tableLog;

    void initTables();
    void reloadPidListToTable(const QJsonArray&);
    void jsonPidListReceived(const QJsonObject&);
    void jsonPosFileDataReceived(const QJsonObject&);

signals:
    void signalPosManagerAction(const QStringList&);


};




#endif // JSPOSMANAGER_TAB_H
