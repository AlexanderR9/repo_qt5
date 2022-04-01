#ifndef MAINFORM_H
#define MAINFORM_H

#include "lmainwidget.h"

class LProtocolBox;
class LHTMLRequester;
class QSplitter;
class QTextEdit;

// MainForm
class MainForm : public LMainWidget
{
    Q_OBJECT
public:
    MainForm(QWidget *parent = 0);
    virtual ~MainForm() {}
    
protected:
    LProtocolBox        *m_protocol;
    LHTMLRequester      *m_req;
    QSplitter           *v_splitter;
    QTextEdit           *m_textView;

    QString projectName() const {return "htmlparser";}
    QString mainTitle() const {return QString("Html requester (Qt5)!");}
    void initActions();
    void initWidgets();
    void initCommonSettings();
    void save();
    void load();

    void startHtmlRequest(); //отправить http запрос для получения html страницы
    void saveHtmlToFile();
    void loadHtmlFile();

protected slots:
    void slotAction(int); //virtual slot from parent
    void slotError(const QString&);
    void slotReqFinished();

};




#endif

