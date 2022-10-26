#ifndef FX_MAINFORM_H
#define FX_MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class QSplitter;
class ViewWidget;



// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}

protected:
    ViewWidget  *m_centralWidget;


    QString projectName() const {return "xmlpacketviewer";}
    QString mainTitle() const {return QString("XML packet viewer (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();

    void save();
    void load();

    QString packDir() const;
    void reloadPackets();
    void toLeft(); //переслать пакет из правой вьюхи в левую
    void toRight(); //переслать пакет из левой вьюхи в правую


protected slots:
    void slotAction(int); //virtual slot from parent
    void slotAppSettingsChanged(QStringList);

signals:
    void signalError(const QString&);
    void signalMsg(const QString&);

private:
    bool singleFloating() const;
    int byteOrder() const;
    quint8 doublePrecision() const;


};



#endif

