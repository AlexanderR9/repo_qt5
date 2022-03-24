 #ifndef LMAINWIDGET_H
 #define LMAINWIDGET_H

 #include <QWidget>

class QToolBar;


// LMainWidget
class LMainWidget: public QWidget
{
    Q_OBJECT
public:
    enum AcitonType {atSettings = 0, atExit, atStart, atStop, atPause, atAdd, atRemove, atOk, atCancel, atUp, atDown, atLeft, atRight, 
			atLoadData, atClear, atRedo, atUndo, atChart, atClock, atOpen, atSave, atSearch, atMonitoring, atRefresh, atData,
            atBuy, atSell, atBag, atBScale, atRScale, atSScale, atCoin, atChain, atHouse, atPerimetr, atSendMsg};

    LMainWidget(QWidget *parent);
    virtual ~LMainWidget() {}

    void addAction(int);
    void addWidget(QWidget*, int, int, int = -1, int = -1);
    QAction* getAction(int) const;
    void addToolBarSeparator();

    virtual void init();
    
    static QString actionText(int);
    static QString actionIconName(int);
    static int defIconSize() {return 40;}

protected:
    virtual void closeEvent(QCloseEvent*) {save();}
    virtual void save();
    virtual void load();
    virtual void actCommonSettings();

    virtual void initCommonSettings() = 0;
    virtual void initActions() = 0;
    virtual void initWidgets() = 0;

    virtual QString projectName() const = 0;
    virtual QString companyName() const {return "my_apps";}
    virtual QString mainTitle() const {return QString("Main widget title!!!");}

    virtual void updateWindowTitle();
    virtual void updateIconSize();
    
    QToolBar 	*m_toolBar;
    QWidget	*m_widget;

    int m_appDialogCaptionWidth;

signals:
    void signalAction(int);
    void signalAppSettingsChanged(QStringList);


protected slots:
    virtual void slotAction(int) = 0;
    virtual void slotTriggered();
    virtual void slotAppSettingsChanged(QStringList);

};



 #endif



