#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lhtmlpagerequester.h"


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
#include <QTabWidget>

//#define SAVE_HTML_FOLDER  QString("data")


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_tab(NULL)
  //  m_textView(NULL),
    //m_timer(NULL),
    //m_pageRequester(NULL)
{
/*
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));


    m_pageRequester = new LHTMLPageRequester(this);
    connect(m_pageRequester, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_pageRequester, SIGNAL(signalDataReady()), this, SLOT(slotDataReady()));
    connect(m_pageRequester, SIGNAL(signalFinished(bool)), this, SLOT(slotFinished(bool)));


    m_couples.append("VZ");
    m_couples.append("T");
    m_couples.append("LSTR");
    m_couples.append("CBRL");
    m_couples.append("INTC");
    m_couples.append("GIS");
    m_couples.append("ET");
    m_couples.append("KO");
    m_couples.append("RPM");
    m_couples.append("MRK");
    m_couples.append("MET");
*/

}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::initWidgets()
{
    v_splitter = new QSplitter(Qt::Vertical, this);
    m_textView = new QTextEdit(this);
    m_protocol = new LProtocolBox(false, this);
    m_textView->setReadOnly(true);

    v_splitter->addWidget(m_textView);
    v_splitter->addWidget(m_protocol);
    addWidget(v_splitter, 0, 0);

    stop();
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("url");
    lCommonSettings.addParam(QString("URL page (example: https://ya.ru)"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString("https://yandex.ru"));

    key = QString("req_interval");
    lCommonSettings.addParam(QString("Request interval, sec"), LSimpleDialog::sdtIntCombo, key);
    for (int i=1; i<=20; i++) combo_list.append(QString::number(i*3));
    lCommonSettings.setComboList(key, combo_list);

    key = QString("view_type");
    lCommonSettings.addParam(QString("Show page content type"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "HTML code" << "Plain text";
    lCommonSettings.setComboList(key, combo_list);

}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {start(); break;}
        case LMainWidget::atStop: {stop(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        default: break;
    }
}
void MainForm::slotTimer()
{
    QString elm = m_couples.takeFirst();
    m_couples.append(elm);

    if (!m_pageRequester->isBuzy())
    {
        m_protocol->addSpace();
        QString msg = QString("Start request [URL=%1] .........").arg(currentUrl());
        m_protocol->addText(msg, LProtocolBox::ttOk);
        m_textView->clear();
    }

    tryRequest();
}
void MainForm::tryRequest()
{
    if (m_pageRequester->isBuzy())
    {
        m_protocol->addText("requester is buzy!!!", LProtocolBox::ttWarning);
        return;
    }

    m_pageRequester->setUrl(currentUrl());
    m_pageRequester->startRequest();
}
void MainForm::parsePageData()
{
    m_protocol->addText(QString("Page title: [%1]").arg(m_pageRequester->title()));
    if (!m_pageRequester->title().contains(QString("(%1)").arg(m_couples.first())))
    {
        m_protocol->addText("page data incorrect", LProtocolBox::ttWarning);
        return;
    }

    QStringList list = m_pageRequester->plainData().split("\n");
    int n = list.count();
    for (int i=n-1; i>=0; i--)
        if (list.at(i).trimmed().isEmpty()) list.removeAt(i);

    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        if (s.left(1) == "$" && s.right(1) == "%")
        {
            m_protocol->addText(QString("%1: %2").arg(m_couples.first()).arg(s), LProtocolBox::ttFile);
            break;
        }
    }

}
void MainForm::slotDataReady()
{
    if (m_pageRequester->badRequest()) return;

    m_protocol->addText("ok!");
    m_protocol->addText(QString("html data size %1,  plain data size %2").arg(m_pageRequester->htmlDataSize()).arg(m_pageRequester->plainDataSize()));

    if (viewType().toLower().contains("html")) m_textView->setPlainText(m_pageRequester->htmlData());
    else m_textView->setPlainText(m_pageRequester->plainData());

    parsePageData();
}
void MainForm::slotFinished(bool ok)
{
    if (!ok) slotError("fault");
}
void MainForm::updateActionsEnable(bool stoped)
{
    getAction(LMainWidget::atStop)->setEnabled(!stoped);
    getAction(LMainWidget::atStart)->setEnabled(stoped);
    getAction(LMainWidget::atSettings)->setEnabled(stoped);
    getAction(LMainWidget::atExit)->setEnabled(stoped);
}
void MainForm::start()
{
    m_protocol->addText(QString("Monitoring started, request interval: %1 sec.").arg(reqInterval()/1000), 5);
    updateActionsEnable(false);

    //m_timer->setInterval(reqInterval());
    //m_timer->start();
}
void MainForm::stop()
{
//    m_timer->stop();
    m_protocol->addText("Monitoring stoped!", 5);
    updateActionsEnable(true);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());
    //settings.setValue(QString("%1/h_splitter/state").arg(objectName()), h_splitter->saveState());
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    //ba.clear();
    //ba = settings.value(QString("%1/h_splitter/state").arg(objectName()), QByteArray()).toByteArray();
    //if (!ba.isEmpty()) h_splitter->restoreState(ba);
}



/*
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


}
void MainForm::slotReqFinished()
{
    m_protocol->addText(QString("getted bytes size: %1").arg(m_req->replySize()));
    m_protocol->addText("Finished!");

    QString data;
    m_req->getHtmlData(data);
    m_textView->setPlainText(data);

}
*/
int MainForm::reqInterval() const
{
    return (lCommonSettings.paramValue("req_interval").toInt() * 1000);
}
QString MainForm::viewType() const
{
    return lCommonSettings.paramValue("view_type").toString();
}
QString MainForm::currentUrl() const
{
    return QString("%1/%2").arg(lCommonSettings.paramValue("url").toString()).arg(m_couples.first());
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}

