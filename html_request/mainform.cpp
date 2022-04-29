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
    m_webView(NULL)
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
void MainForm::initWidgets()
{
    v_splitter = new QSplitter(Qt::Vertical, this);
    h_splitter = new QSplitter(Qt::Horizontal, this);
    m_textView = new QTextEdit(this);
    m_webView = new QWebEngineView(this);
    m_protocol = new LProtocolBox(false, this);
    m_textView->setReadOnly(true);



    //m_textView->setDocumentTitle(QString("------------- HTML text ------------------"));

    QTextDocument *doc = m_textView->document();
    if (doc) qDebug("doc ok!");
    else qDebug("doc is null!");
    m_textView->update();

    QGroupBox *html_box = new QGroupBox(QString("View HTML"), this);
    if (html_box->layout()) delete html_box->layout();
    html_box->setLayout(new QHBoxLayout(0));
    html_box->layout()->addWidget(m_textView);
    //html_box->layout()->setMargin(2);

    v_splitter->addWidget(html_box);
    v_splitter->addWidget(m_protocol);
    h_splitter->addWidget(v_splitter);
    h_splitter->addWidget(m_webView);


    addWidget(h_splitter, 0, 0);


    //QString html_text("<h1 color=\"red\" align=\"center\">Заголовок первого уровня</h1>");
    //m_textView->setHtml(html_text);


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
    QString msg = QString("Start request [URL=%1] .........").arg(m_req->currentUrl());
    m_protocol->addText(msg, LProtocolBox::ttOk);

    m_textView->clear();
    m_textView->setDocumentTitle(QString("------------- HTML of url(%1) ------------------").arg(m_req->currentUrl()));

    if (m_req) 
    {
        m_req->setUrl(lCommonSettings.paramValue("url").toString());
        m_req->startRequest();
        //m_webView->load(m_req->currentUrl());
    }    
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


