#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lhtmlpagerequester.h"
#include "cfdpage.h"
#include "logpage.h"
#include "htmlpage.h"
#include "configpage.h"
#include "cfdconfigobj.h"
#include "tgbot.h"
#include "cfdcalcobj.h"

#include <QDebug>
#include <QTimer>
#include <QIcon>
#include <QSettings>
#include <QApplication>
#include <QSplitter>
#include <QTextEdit>
#include <QWebEngineView>
#include <QProgressBar>
#include <QGroupBox>
#include <QStyleFactory>
#include <QTabWidget>


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_tab(NULL),
    m_configObj(NULL),
    m_bot(NULL),
    m_timer(NULL),
    m_calcObj(NULL)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

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
    qDeleteAll(m_pages);
    m_pages.clear();

    initPages();
    initTab();
    initSplitter();

    updateActionsEnable(true);
}
void MainForm::initPages()
{
    CFDPage *cfd_page = new  CFDPage(this);
    m_pages.insert(BasePage::ptCFDStat, cfd_page);

    ConfigPage *config_page = new  ConfigPage(this);
    m_pages.insert(BasePage::ptConfig, config_page);

    LogPage *log_page = new  LogPage(this);
    m_pages.insert(BasePage::ptLog, log_page);

    HtmlPage *html_page = new  HtmlPage(this);
    m_pages.insert(BasePage::ptHtml, html_page);
    connect(html_page, SIGNAL(signalGetUrlByTicker(const QString&, QString&)), config_page, SLOT(slotSetUrlByTicker(const QString&, QString&)));


    foreach (BasePage *page, m_pages)
    {
        connect(page, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
        connect(page, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));
    }
}
void MainForm::initTab()
{
    if (m_tab) {delete m_tab; m_tab = NULL;}
    m_tab = new QTabWidget(this);
    m_tab->setIconSize(QSize(28, 28));
    //m_tab->setTabShape(QTabWidget::Triangular);
    m_tab->setTabPosition(QTabWidget::South);

    int n = m_tab->count();
    for (int i=0; i<n; i++)
    {
        QWidget *w = m_tab->widget(0);
        m_tab->removeTab(0);
        if (w) delete w;
    }

    foreach (BasePage *page, m_pages)
    {
        QIcon icon(page->iconPath());
        m_tab->addTab(page, icon, page->caption());
    }
}
void MainForm::initSplitter()
{
    m_protocol = new LProtocolBox(false, this);
    if (v_splitter) {delete v_splitter; v_splitter = NULL;}
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    v_splitter->addWidget(m_tab);
    v_splitter->addWidget(m_protocol);
}
void MainForm::initConfigObj()
{
    if (m_configObj) {delete m_configObj; m_configObj = NULL;}
    m_configObj = new CFDConfigObject(lCommonSettings.paramValue("config").toString(), this);

    connect(m_configObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_configObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

    m_protocol->addText("Try load CFD configuration .....", LProtocolBox::ttOk);
    m_configObj->tryLoadConfig();
}
void MainForm::initCalcObj()
{
    if (m_calcObj) {delete m_calcObj; m_calcObj = NULL;}
    m_calcObj = new CFDCalcObj(this);

    connect(m_calcObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_calcObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

    m_protocol->addText("Calculate object ready!", LProtocolBox::ttOk);
    m_protocol->addSpace();

    HtmlPage *page = qobject_cast<HtmlPage*>(m_pages.value(BasePage::ptHtml));
    if (!page)
    {
        qWarning()<<QString("MainForm::fillConfigPage() ERR: invalid convert to HtmlPage from m_pages");
        return;
    }
    connect(page, SIGNAL(signalNewPrice(QString, double)), m_calcObj, SLOT(slotNewPrice(QString, double)));

    CFDPage *cfd_page = qobject_cast<CFDPage*>(m_pages.value(BasePage::ptCFDStat));
    if (!cfd_page)
    {
        qWarning()<<QString("MainForm::fillConfigPage() ERR: invalid convert to CFDPage from m_pages");
        return;
    }
    connect(m_calcObj, SIGNAL(signalUpdateCFDTable(const QStringList&)), cfd_page, SLOT(slotNewPrice(const QStringList&)));
}
void MainForm::initBotObj()
{
    if (m_bot) {delete m_bot; m_bot = NULL;}
    m_bot = new TGBot(this);

    connect(m_bot, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_bot, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

    m_protocol->addSpace();
    m_protocol->addText("Try bot configuration .....", LProtocolBox::ttOk);
    m_bot->loadConfig(lCommonSettings.paramValue("config").toString());
    if (m_bot->invalid())
    {
        slotError(QString("invalid loaded bot parameters"));
        return;
    }
}
void MainForm::fillConfigPage()
{
    ConfigPage *page = qobject_cast<ConfigPage*>(m_pages.value(BasePage::ptConfig));
    if (!page)
    {
        qWarning()<<QString("MainForm::fillConfigPage() ERR: invalid convert to ConfigPage from m_pages");
        return;
    }

    page->reinitCFDTable();
    page->reinitTGTable();

    page->setTGBotParams(m_bot->getParams());

    QStringList data(m_configObj->getSources());
    page->setSourses(data);

    int n = m_configObj->cfdCount();
    for (int i=0; i<n; i++)
    {
        QStringList row_data(m_configObj->getCFDObjectData(i));
        page->addCFDObject(row_data);
    }

    page->updatePage();
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("config");
    lCommonSettings.addParam(QString("Config file"), LSimpleDialog::sdtFilePath, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("req_interval");
    lCommonSettings.addParam(QString("Request interval, sec"), LSimpleDialog::sdtIntCombo, key);
    for (int i=1; i<=20; i++) combo_list.append(QString::number(i*3));
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
    m_protocol->addSpace();

    QString next_ticker;
    m_configObj->getNextTicker(next_ticker);
    QString msg = QString("next request [%1] .........").arg(next_ticker);
    m_protocol->addText(msg, LProtocolBox::ttFile);

    HtmlPage *page = qobject_cast<HtmlPage*>(m_pages.value(BasePage::ptHtml));
    if (!page)
    {
        qWarning()<<QString("MainForm::slotTimer() ERR: invalid convert to HtmlPage from m_pages");
        return;
    }

    page->tryRequest(next_ticker);
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
    m_protocol->addSpace();
    m_protocol->addText(QString("Monitoring started, request interval: %1 sec.").arg(reqInterval()/1000), 5);
    updateActionsEnable(false);

    m_timer->setInterval(reqInterval());
    m_timer->start();
}
void MainForm::stop()
{
    m_timer->stop();
    m_protocol->addText("Monitoring stoped!", 5);
    updateActionsEnable(true);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());
    settings.setValue(QString("%1/tab/page_index").arg(objectName()), m_tab->currentIndex());

}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    m_tab->setCurrentIndex(settings.value(QString("%1/tab/page_index").arg(objectName()), 0).toInt());

    initCalcObj();
    initConfigObj();
    initBotObj();
    fillConfigPage();
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
/*
QString MainForm::viewType() const
{
    return lCommonSettings.paramValue("view_type").toString();
}
QString MainForm::currentUrl() const
{
    return QString("%1/%2").arg(lCommonSettings.paramValue("url").toString()).arg(m_couples.first());
}
*/
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMessage(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}

