 #ifndef MAINFORM_H
 #define MAINFORM_H 

 #include "lmainwidget.h"


class LProtocolBox;
class ModBusObj;
class QByteArray;
class FileWorker;

// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
protected:
    QString projectName() const {return "modbustester";}
    QString mainTitle() const {return QString("Test ModBus (Qt5)!");}
    void save();
    void load();

    void openPort();
    void closePort();
    void writeToPort();
    void updatePortParams();
//    void parseReceivedData(const QByteArray&);

    void initActions();
    void initWidgets();
    void initCommonSettings();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMessage(const QString&);
    void slotAppSettingsChanged(QStringList);


private:
    LProtocolBox    *m_protocol;
    ModBusObj       *m_modbusObj;
    //FileWorker      *f_worker;

};



 #endif

