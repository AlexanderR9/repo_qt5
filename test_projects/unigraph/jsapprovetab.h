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

    //информация загруженная из локального файла, содержит информацию только о значениях апрувнутых токенов.
    //вид строки: token_addr / approved_value for POS_MANAGER / approved_value for SWAP_ROUTER
    QStringList          m_locData;

    void initPopupMenu(); //инициализировать элементы всплывающего меню
    void updateSuppliedCell(int i, int j, float value);
    void answerUpdate(const QJsonObject&);
    void answerApprove(const QJsonObject&);

    //working by local data
    void loadLocalData(); //from local defi file
    void rewriteLocalDataFile(); //to local defi file
    void removeRecFromLocalData(const QString&); //в момент апрува необходимо удалить запись из локальных данных для текущего токена, т.к. значение поменяется
    void addLocalData(const QString&, float, float); //в момент получения ответа о текущих апувнутых значения добавить обновить/добавить соответствующую запись в m_locData
    void syncTableByLocalData();
    void getApprovedByLocalData(const QString&, float&, float&) const; //получить из локальных данных текущие апрувнутые значения для указанного токена

protected slots:
    void slotUpdateApproved();
    void slotSendApprove();

public slots:
    void slotScriptBroken();

signals:
    void signalCheckUpproved(QString);
    void signalApprove(const QStringList&);

};



#endif // JSAPPROVE_TAB_H
