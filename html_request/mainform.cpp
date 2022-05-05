#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lhtmlrequester.h"
#include "lfile.h"
#include "htmlparser.h"


#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QApplication>
#include <QSplitter>
#include <QTextEdit>
#include <QWebEngineView>
#include <QProgressBar>
#include <QGroupBox>
#include <QStyleFactory>


#define SAVE_HTML_FOLDER  QString("data")


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    m_req(NULL),
    m_parser(NULL),
    v_splitter(NULL),
    h_splitter(NULL),
    m_textView(NULL),
    m_webView(NULL),
    m_viewProgress(NULL)
{
    setObjectName("main_form_htmlparser");

    m_req = new LHTMLRequester(this);
    connect(m_req, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_req, SIGNAL(signalFinished()), this, SLOT(slotReqFinished()));

    m_parser = new MyHTMLParser(this);
}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atSave);
    addAction(LMainWidget::atLoadData);
    addAction(LMainWidget::atChain);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {startHtmlRequest(); break;}
        case LMainWidget::atSave: {saveHtmlToFile(); break;}
        case LMainWidget::atLoadData: {loadHtmlFile(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        case LMainWidget::atChain: {parseHtml(); break;}

        default: break;
    }
}
void MainForm::initWebView(QGroupBox *&view_box)
{
    m_webView = new QWebEngineView(this);
    connect(m_webView, SIGNAL(loadStarted()), this, SLOT(slotViewStarted()));
    connect(m_webView, SIGNAL(loadProgress(int)), this, SLOT(slotViewProgress(int)));
    connect(m_webView, SIGNAL(loadFinished(bool)), this, SLOT(slotViewFinished(bool)));

    m_viewProgress = new QProgressBar(this);
    m_viewProgress->setTextVisible(true);
    m_viewProgress->setStyle(QStyleFactory::create("NorwegianWood"));

    //QFont font;
    //font.setBold(true);
    //font.setPixelSize(20);
    //m_viewProgress->setFont(font);

    view_box = new QGroupBox("Web view", this);
    if (view_box->layout()) delete view_box->layout();
    view_box->setLayout(new QVBoxLayout(0));
    view_box->layout()->addWidget(m_webView);
    view_box->layout()->addWidget(m_viewProgress);

}
void MainForm::slotViewStarted()
{
    qDebug("MainForm::slotViewStarted()");
    m_viewProgress->setValue(0);
    QPalette p;
    p.setColor(QPalette::Background, Qt::darkYellow);
    m_viewProgress->setPalette(p);
}
void MainForm::slotViewProgress(int p)
{
    qDebug()<<QString("MainForm::slotViewProgress()  p=%1").arg(p);
    m_viewProgress->setValue(p);

}
void MainForm::slotViewFinished(bool ok)
{
    qDebug()<<QString("MainForm::slotViewFinished()  ok=%1").arg(ok);
    QPalette p;
    //p.setColor(QPalette::Background, Qt::green);
    p.setBrush(m_viewProgress->backgroundRole(), Qt::green);
    m_viewProgress->setPalette(p);

    if (ok) m_protocol->addText(QString("url loaded ok!  title: %1").arg(m_webView->title()));
    else slotError("fault.");


    m_webView->page()->toPlainText([this](const QString &result){functorToPlaneText(result);});
    //m_webView->page()->toHtml([this](const QString &result){functorToPlaneText(result);});
    //protected slots:    void handleHtml(QString sHtml);signals:
    //void html(QString sHtml); void MainWindow::SomeFunction() {    connect(this, SIGNAL(html(QString)), this, SLOT(handleHtml(QString)));
    //view->page()->toHtml([this](const QString& result) mutable {emit html(result);}); }void MainWindow::handleHtml(QString sHtml){      qDebug()<<"myhtml"<< sHtml;}
}
void MainForm::functorToPlaneText(const QString &s)
{
    qDebug("MainForm::functorToPlaneText");
    m_textView->clear();
    m_textView->setPlainText(s);

}
void MainForm::functorToHtml(const QString &s)
{
    qDebug("MainForm::functorToPlaneText");
    m_textView->clear();
    m_textView->setHtml(s);
}
void MainForm::initWidgets()
{
    QGroupBox *view_box = NULL;
    initWebView(view_box);

    v_splitter = new QSplitter(Qt::Vertical, this);
    h_splitter = new QSplitter(Qt::Horizontal, this);
    m_textView = new QTextEdit(this);
    m_protocol = new LProtocolBox(false, this);
    m_textView->setReadOnly(true);

    QTextDocument *doc = m_textView->document();
    if (doc) qDebug("doc ok!");
    else qDebug("doc is null!");
    m_textView->update();

    QGroupBox *html_box = new QGroupBox(QString("View HTML"), this);
    if (html_box->layout()) delete html_box->layout();
    html_box->setLayout(new QHBoxLayout(0));
    html_box->layout()->addWidget(m_textView);

    v_splitter->addWidget(html_box);
    v_splitter->addWidget(m_protocol);
    h_splitter->addWidget(v_splitter);

    if (view_box)
        h_splitter->addWidget(view_box);

    addWidget(h_splitter, 0, 0);
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=0; i<=5; i++)
        combo_list.append(QString::number(i));
    
    QString key = QString("url");
    lCommonSettings.addParam(QString("URL page (example: https://ya.ru)"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString("https://yandex.ru"));

    key = QString("savefile");
    lCommonSettings.addParam(QString("Save HTML file (example: data.html)"), LSimpleDialog::sdtFilePath, key);
    lCommonSettings.setDefValue(key, QString("html.txt"));
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());
    settings.setValue(QString("%1/h_splitter/state").arg(objectName()), h_splitter->saveState());
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    ba.clear();
    ba = settings.value(QString("%1/h_splitter/state").arg(objectName()), QByteArray()).toByteArray();
    if (!ba.isEmpty()) h_splitter->restoreState(ba);
}
void MainForm::parseHtml()
{
    if (!m_parser) return

    m_parser->reset();
    m_parser->tryParseHtmlText(m_textView->toPlainText());

    m_protocol->addText(QString("--- parsing result ----"));
    m_protocol->addText(QString("head_node size: %1").arg(m_parser->headData().length()));
    m_protocol->addText(QString("body_node size: %1").arg(m_parser->bodyData().length()));

    m_textView->clear();
    //m_textView->setPlainText(parser.bodyData());
    m_textView->setHtml(m_parser->bodyData());

}
void MainForm::loadHtmlFile()
{
    m_protocol->addSpace();
    m_textView->clear();

    QString err;
    QString key = QString("savefile");
    QString f_name = lCommonSettings.paramValue(key).toString().trimmed();
    if (f_name.isEmpty())
    {
        err = QString("Saving filename is empty, enter normal value to add settings.");
        slotError(err);
        return;
    }

    err.clear();
    if (!f_name.contains(QDir::separator()))
        f_name = QString("%1%2%3").arg(SAVE_HTML_FOLDER).arg(QDir::separator()).arg(f_name);
    m_protocol->addText(QString("Try load HTML file [%1] .........").arg(f_name), LProtocolBox::ttOk);

    QString data;
    err = LFile::readFileStr(f_name, data);
    if (!err.isEmpty())
    {
        slotError(err);
        return;
    }

    m_textView->setPlainText(data);
    m_protocol->addText(QString("Ok!  Readed %1 bytes.").arg(data.size()));

    if (!m_webView->page()) qDebug()<<QString("page is NULL");

    m_webView->setHtml(data);
}
void MainForm::saveHtmlToFile()
{
    m_protocol->addSpace();
    QString err;

    QDir dir(SAVE_HTML_FOLDER);
    if (!dir.exists())
    {
        err = QString("Saving folder [%1] not found.").arg(SAVE_HTML_FOLDER);
        slotError(err);
        return;
    }

    QString key = QString("savefile");
    QString f_name = lCommonSettings.paramValue(key).toString().trimmed();
    if (f_name.isEmpty())
    {
        err = QString("Saving filename is empty, enter normal value to add settings.");
        slotError(err);
        return;
    }
    if (f_name.length() < 5)
    {
        err = QString("Saving filename [%1] too hort, enter normal value to add settings.").arg(f_name);
        slotError(err);
        return;
    }

    err.clear();
    if (!f_name.contains(QDir::separator()))
        f_name = QString("%1%2%3").arg(SAVE_HTML_FOLDER).arg(QDir::separator()).arg(f_name);
    m_protocol->addText(QString("Try save HTML data to file [%1] .........").arg(f_name), LProtocolBox::ttOk);

    QString data;
    m_req->getHtmlData(data);
    err = LFile::writeFile(f_name, data);
    if (!err.isEmpty()) m_protocol->addText(err, LProtocolBox::ttErr);
    else m_protocol->addText(QString("Ok!  Was writed %1 bytes.").arg(data.size()));
}
void MainForm::startHtmlRequest()
{
    qDebug("press start ..........");
    m_protocol->addSpace();

    QString msg = QString("Start request [URL=%1] .........").arg(currentUrl());
    m_protocol->addText(msg, LProtocolBox::ttOk);

    m_textView->clear();
    m_textView->setDocumentTitle(QString("------------- HTML of url(%1) ------------------").arg(m_req->currentUrl()));


    ///////////////for m_webView////////////////////////////////////
    m_webView->load(currentUrl());
    m_webView->show();


    ///////////////for m_req////////////////////////////////////
    /*
    if (m_req) 
    {
        m_req->setUrl(currentUrl());
        m_req->startRequest();
    }
    */
}
QString MainForm::currentUrl() const
{
    return lCommonSettings.paramValue("url").toString();
}
void MainForm::slotReqFinished()
{
    m_protocol->addText(QString("getted bytes size: %1").arg(m_req->replySize()));
    m_protocol->addText("Finished!");

    QString data;
    m_req->getHtmlData(data);
    m_textView->setPlainText(data);

}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}


