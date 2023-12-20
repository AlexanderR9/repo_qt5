#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstring.h"
#include "lstatic.h"
#include "lsimplewidget.h"
#include "lsearch.h"
#include "ltable.h"
#include "bb_centralwidget.h"
#include "apiconfig.h"

#include <QDebug>
#include <QDir>
#include <QTest>
#include <QTimer>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QProgressBar>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QTableWidget>
#include <QClipboard>
#include <QGuiApplication>



// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    v_splitter(NULL),
    m_centralWidget(NULL)
{
    setObjectName("bybit_mainwindow");
    qDebug("MainForm::MainFormMainForm()");

    api_config.loadTickers();
}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atLoadData);
    addAction(LMainWidget::atRight);
    addAction(LMainWidget::atRedo);

    addToolBarSeparator();
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);

    this->setActionTooltip(atRedo, "remove carriage return symbols and rewrite file1");
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atStart: {actStart(); break;}
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        default: break;
    }
}
void MainForm::actStart()
{
    m_protocol->addSpace();
    m_protocol->addText("Run test .......", LProtocolBox::ttFile);



    m_protocol->addText("finished!");
}

void MainForm::initWidgets()
{
    qDebug("MainForm::initWidgets()");
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    m_protocol = new LProtocolBox(false, this);
    m_centralWidget = new BB_CentralWidget(this);
    v_splitter->addWidget(m_centralWidget);
    v_splitter->addWidget(m_protocol);

    connect(m_centralWidget, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_centralWidget, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMsg(const QString&)));

}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("file1");
    lCommonSettings.addParam(QString("File 1"), LSimpleDialog::sdtFilePath, key);
    lCommonSettings.setDefValue(key, QString(""));



}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());

    m_centralWidget->save(settings);
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    m_centralWidget->load(settings);
}
void MainForm::slotAppSettingsChanged(QStringList list)
{
    LMainWidget::slotAppSettingsChanged(list);
    /*
    if (list.contains("server_api") || list.contains("print_headers"))
    {
        APIReqPage *req_page = qobject_cast<APIReqPage*>(m_pages.value(aptReq));
        if (req_page)
        {
            req_page->setServerAPI(hptHttps, serverAPI());
            req_page->setPrintHeaders(printHeaders());
        }
        else slotError("req_page is NULL");
    }
    else if (list.contains("expand_level"))
    {
        APIReqPage *req_page = qobject_cast<APIReqPage*>(m_pages.value(aptReq));
        if (req_page) req_page->setExpandLevel(expandLevel());
        else slotError("req_page is NULL");
    }
    */
}

