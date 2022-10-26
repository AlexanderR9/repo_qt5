#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lhtmlpagerequester.h"
#include "lfile.h"
#include "htmlparser.h"
#include "lstatic.h"
#include "lfile.h"


#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QApplication>
#include <QSplitter>
#include <QTextEdit>
#include <QWebEngineView>
#include <QTimer>
#include <QProgressBar>
#include <QGroupBox>
#include <QStyleFactory>


#define SAVE_HTML_FOLDER  QString("data")


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    m_req(NULL),
    v_splitter(NULL),
    h_splitter(NULL),
    m_textView(NULL),
    m_viewProgress(NULL)
{
    setObjectName("main_form_htmlparser");

    m_req = new LHTMLPageRequester(this);
    connect(m_req, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_req, SIGNAL(signalDataReady()), this, SLOT(slotReqFinished()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    timer->start(30000);
}
void MainForm::slotTimer()
{
    if (m_tickers.isEmpty())
    {
        qobject_cast<QTimer*>(sender())->stop();
        m_protocol->addSpace();
        m_protocol->addText("timer stoped!");
    }

    m_protocol->addSpace();
    m_protocol->addText("timer tick");
    if (m_req->isBuzy())
    {
        m_protocol->addText("requester is buzy.");
        return;
    }

    QString ticker = m_tickers.last();
    QString url = QString("https://finviz.com/quote.ashx?t=%1").arg(ticker);
    m_protocol->addText(QString("try next data ticker: %1").arg(ticker));
    m_protocol->addText(QString("URL: %1").arg(url));
    m_req->setUrl(url);
    m_req->startRequest();

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
    //p.setBrush(m_viewProgress->backgroundRole(), Qt::green);
    //m_viewProgress->setPalette(p);

    //if (ok) m_protocol->addText(QString("url loaded ok!  title: %1").arg(m_webView->title()));
    //else slotError("fault.");


    //m_webView->page()->toPlainText([this](const QString &result){functorToPlaneText(result);});
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
    //initWebView(view_box);

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

    //loadtickers();
    loadtickers_insta();
}
void MainForm::parseHtml()
{
    //if (!m_parser) return
    //m_parser->reset();
    //m_parser->tryParseHtmlText(m_textView->toPlainText());

    //m_protocol->addText(QString("--- parsing result ----"));
    //m_protocol->addText(QString("head_node size: %1").arg(m_parser->headData().length()));
    //m_protocol->addText(QString("body_node size: %1").arg(m_parser->bodyData().length()));

    m_textView->clear();
    //m_textView->setPlainText(parser.bodyData());
    //m_textView->setHtml(m_parser->bodyData());
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

    QString data = m_req->htmlData();
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
    m_textView->setDocumentTitle(QString("------------- HTML of url(%1) ------------------").arg(m_req->url()));

    ///////////////for m_req////////////////////////////////////
    if (m_req) 
    {
        m_req->setUrl(currentUrl());
        m_req->startRequest();
    }
}
QString MainForm::currentUrl() const
{
    return lCommonSettings.paramValue("url").toString();
}
void MainForm::slotReqFinished()
{
    m_protocol->addText(QString("getted bytes size: %1").arg(m_req->plainDataSize()));
    m_protocol->addText("Finished!");

    QString data(m_req->plainData());
    m_textView->setPlainText(data);

    QString fname = QString("%1%2%3").arg(SAVE_HTML_FOLDER).arg(QDir::separator()).arg(QString("config.txt"));
    QStringList list = LStatic::trimSplitList(data);
    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        if (s.indexOf(m_tickers.last()) == 0 && s.contains("[") && s.contains("]"))
        {
            s = list.at(i+1).trimmed();
            m_protocol->addText(QString("company_name = %1").arg(s));
            QString amp("&");
            if (s.contains(amp)) s.replace(amp, QString("&amp;"));

            QString f_line = QString("<cfd ticker=\"%1\" name=\"%2\" source=\"1\" insta=\"true\"").arg(m_tickers.last()).arg(s);
            m_tickers.removeLast();

            QStringList direction = LStatic::trimSplitList(list.at(i+2).trimmed(), "|");
            if (direction.count() == 3)
            {
                direction[1].replace(amp, QString("&amp;"));
                f_line.append(QString(" direction=\"%1\" country=\"%2\"").arg(direction.at(1)).arg(direction.at(2)));
            }
            f_line.append(QString("/>\n"));

            QString err = LFile::appendFile(fname, f_line);
            if (!err.isEmpty()) slotError(err);
            break;
        }
    }

    m_protocol->addText(QString("tickers else %1").arg(m_tickers.count()));
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::loadtickers()
{
    m_protocol->addSpace();
    m_textView->clear();
    QString f_name = "tickers.txt";
    f_name = QString("%1%2%3").arg(SAVE_HTML_FOLDER).arg(QDir::separator()).arg(f_name);
    m_protocol->addText(QString("Try load file [%1] .........").arg(f_name), LProtocolBox::ttOk);

    m_tickers.clear();
    QString err = LFile::readFileSL(f_name, m_tickers);
    if (!err.isEmpty())
    {
        slotError(err);
        return;
    }

    for (int i=m_tickers.count()-1; i>=0; i--)
    {
        QString s = m_tickers.at(i).trimmed();
        s = s.remove("\n");
        s = s.remove("\r");
        if (s.isEmpty())
        {
            m_tickers.removeAt(i);
            continue;
        }
        m_tickers[i] = s;
        qDebug()<<QString("%1.   [%2]").arg(i+1).arg(m_tickers.at(i));
    }
    m_protocol->addText(QString("loaded %1 tickers.").arg(m_tickers.count()), LProtocolBox::ttOk);

}
void MainForm::loadtickers_insta()
{
    m_protocol->addSpace();
    m_textView->clear();
    QString f_name = "tickers_insta.txt";
    f_name = QString("%1%2%3").arg(SAVE_HTML_FOLDER).arg(QDir::separator()).arg(f_name);
    m_protocol->addText(QString("Try load file [%1] .........").arg(f_name), LProtocolBox::ttOk);

    m_tickers.clear();
    QString err = LFile::readFileSL(f_name, m_tickers);
    if (!err.isEmpty())
    {
        slotError(err);
        return;
    }


    int space_pos = -1;
    for (int i=m_tickers.count()-1; i>=0; i--)
    {
        QString s = m_tickers.at(i).trimmed();
        s = s.remove("\n");
        s = s.remove("\r");
        if (s.isEmpty()) {m_tickers.removeAt(i); continue;}
        if (s.indexOf("#") == 0) s = LStatic::strTrimLeft(s, 1);
        space_pos = s.indexOf(LStatic::spaceSymbol());
        if (space_pos <= 0) {m_tickers.removeAt(i); continue;}
        m_tickers[i] = s.left(space_pos);

        qDebug()<<QString("%1.   [%2]").arg(i+1).arg(m_tickers.at(i));
    }
    m_protocol->addText(QString("loaded %1 tickers.").arg(m_tickers.count()), LProtocolBox::ttOk);

}


