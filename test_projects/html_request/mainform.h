#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class LHTMLPageRequester;
class QSplitter;
class QTextEdit;
//class MyHTMLParser;
//class QWebEngineView;
class QProgressBar;
class QGroupBox;


// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
protected:
    LProtocolBox        *m_protocol;
    LHTMLPageRequester  *m_req;
    QSplitter           *v_splitter;
    QSplitter           *h_splitter;
    QTextEdit           *m_textView;
    QProgressBar        *m_viewProgress;
    QStringList         m_tickers;


    QString projectName() const {return "htmlparser";}
    QString mainTitle() const {return QString("Html requester (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();
    void save();
    void load();

    //void initWebView(QGroupBox*&);

    void startHtmlRequest(); //отправить http запрос для получения html страницы
    void saveHtmlToFile();
    void loadHtmlFile();
    void parseHtml();
    void loadtickers();
    void loadtickers_insta();

    QString currentUrl() const;

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotReqFinished();
    void slotTimer();

    //for m_webView
    //void slotViewStarted();
    void slotViewProgress(int);
    void slotViewFinished(bool);

private:
    void functorToPlaneText(const QString&);
    void functorToHtml(const QString&);

};




#endif

