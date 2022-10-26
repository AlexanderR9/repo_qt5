 #ifndef MAINFORM_H
 #define MAINFORM_H 

 #include "lmainwidget.h"


class LProtocolBox;
class ComObj;
class QByteArray;
class FileWorker;

// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm(); 
    
protected:
    QString projectName() const {return "comtester";}
    QString mainTitle() const {return QString("Test COM (Qt5)!");}
    void save();
    void load();

    void openPort();
    void closePort();
    void writeToPort();
    void updatePortParams();
    void parseReceivedData(const QByteArray&);
    void tryAddCRCBuff(QByteArray&);
    quint16 calcCRC(const QByteArray&) const;
    quint16 MB_CRC16_cs(const unsigned char*, quint16) const;



    void initActions();
    void initWidgets();
    void initCommonSettings();


    static void getTestBA(QByteArray&);

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotMessage(const QString&);
    void slotAppSettingsChanged(QStringList);


private:
    LProtocolBox    *m_protocol;
    ComObj          *m_comObj;
    FileWorker      *f_worker;


};



 #endif

