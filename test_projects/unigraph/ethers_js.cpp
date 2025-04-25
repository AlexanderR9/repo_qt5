#include "ethers_js.h"
#include "ug_apistruct.h"
#include "processobj.h"
#include "jswallettab.h"
#include "jsapprovetab.h"
#include "jstxtab.h"
#include "jspooltab.h"
#include "subcommonsettings.h"
#include "ltime.h"
#include "lfile.h"
#include "lstring.h"

#include <QDebug>
#include <QSplitter>
#include <QTabWidget>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QIcon>
#include <QSettings>



#define SIGN_START      QString("JSON_RESULT_START")
#define SIGN_END        QString("JSON_RESULT_END")
#define QUTE_SYMBOL1    QChar('\"')
#define QUTE_SYMBOL2    QChar('\'')


//EthersPage
EthersPage::EthersPage(QWidget *parent)
    :UG_BasePage(parent, 10, rtEthers),
      m_procObj(NULL),
      m_tab(NULL),
      m_walletPage(NULL),
      m_approvePage(NULL),
      m_txPage(NULL),
      m_poolPage(NULL)
{
    setObjectName("ethers_page");
    initWidgets();
    initProcessObj();

    m_walletPage->loadAssetsFromFile();
    m_approvePage->setTokens(m_walletPage->assetsTokens());
    m_txPage->loadTxFromFile();
    m_poolPage->loadPoolsFromFile();
}
void EthersPage::initProcessObj()
{
    m_procObj = new LProcessObj(this);
    connect(m_procObj, SIGNAL(signalFinished()), this, SLOT(slotScriptFinished()));

    m_procObj->setCommand("node");
    m_procObj->setProcessDir(sub_commonSettings.nodejs_path);
    m_procObj->setDebugLevel(5);
}
void EthersPage::initWidgets()
{
    m_tab = new LTabWidgetBox(this);
    m_tab->setTitle("DEFI data");
    m_tab->removeAllPages();
    v_splitter->addWidget(m_tab);

    m_walletPage = new JSWalletTab(this);
    QString icon_path = SubGraph_CommonSettings::iconPathByChain("polygon").trimmed();
    if (LFile::fileExists(icon_path)) m_tab->tab()->addTab(m_walletPage, QIcon(icon_path), "Wallet");
    else m_tab->tab()->addTab(m_walletPage, "Wallet");

    m_approvePage = new JSApproveTab(this);
    m_tab->tab()->addTab(m_approvePage, "Approve");

    m_txPage = new JSTxTab(this);
    m_tab->tab()->addTab(m_txPage, "Transactions");

    m_poolPage = new JSPoolTab(this);
    m_tab->tab()->addTab(m_poolPage, "Pools");

    connect(m_walletPage, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_walletPage, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    connect(m_walletPage, SIGNAL(signalWalletTx(const QStringList&)), this, SLOT(slotWalletTx(const QStringList&)));
    connect(m_approvePage, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_approvePage, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    connect(m_approvePage, SIGNAL(signalCheckUpproved(QString)), this, SLOT(slotCheckUpproved(QString)));
    connect(m_approvePage, SIGNAL(signalApprove(const QStringList&)), this, SLOT(slotApprove(const QStringList&)));
    connect(this, SIGNAL(signalScriptBroken()), m_walletPage, SLOT(slotScriptBroken()));
    connect(this, SIGNAL(signalScriptBroken()), m_approvePage, SLOT(slotScriptBroken()));
    connect(m_txPage, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_txPage, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    connect(m_txPage, SIGNAL(signalCheckTx(const QStringList&)), this, SLOT(slotCheckTx(const QStringList&)));
    connect(m_poolPage, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_poolPage, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    connect(m_poolPage, SIGNAL(signalPoolAction(const QStringList&)), this, SLOT(slotPoolAction(const QStringList&)));
    connect(m_poolPage, SIGNAL(signalResetApproved(const QString&)), m_approvePage, SLOT(slotResetRecord(const QString&)));
    connect(m_poolPage, SIGNAL(signalGetApprovedSize(QString, const QString&, float&)), m_approvePage, SLOT(slotGetApprovedSize(QString, const QString&, float&)));

}
void EthersPage::slotScriptFinished()
{
    emit signalMsg(QString("%1  node_js done!").arg(LTime::strCurrentTime()));
  //  qDebug()<<QString("BUFF:")<<m_procObj->buffer();
    parseResultBuffer();
    emit signalStopUpdating();
}
void EthersPage::startUpdating(quint16 t)
{
    qDebug()<<QString("EthersPage::startUpdating t=%1").arg(t);
    if (walletPageNow())
    {
        tryUpdateBalace();
        return;
    }
    if (txPageNow())
    {
        m_txPage->loadTxFromFile();
        emit signalStopUpdating();
        return;
    }

    emit signalError("wrong current page");
    emit signalStopUpdating();
}
bool EthersPage::walletPageNow() const
{
    return m_tab->tab()->currentWidget()->objectName().contains("wallet");
}
bool EthersPage::approvePageNow() const
{
    return m_tab->tab()->currentWidget()->objectName().contains("approve");
}
bool EthersPage::txPageNow() const
{
    return m_tab->tab()->currentWidget()->objectName().contains("tx_tab");
}
bool EthersPage::poolsPageNow() const
{
    return m_tab->tab()->currentWidget()->objectName().contains("pools");
}



void EthersPage::tryUpdateBalace()
{
    qDebug()<<QString("%1 .... tryUpdateBalace").arg(LTime::strCurrentTime());
    QStringList args;
    m_walletPage->getBalacesArgs(args);
    m_procObj->setArgs(args);
    emit signalMsg(QString("%1  start node_js process %2").arg(LTime::strCurrentTime()).arg(LString::symbolString('.', 50)));
    emit signalMsg(QString("command [%1]").arg(m_procObj->fullCommand()));
    m_procObj->startCommand();
}
void EthersPage::parseResultBuffer()
{
    QString result(m_procObj->buffer().trimmed());
    if (!result.contains(SIGN_START)) {emit signalError(QString("invalid NODE_JS result, not found sign [%1]").arg(SIGN_START)); return;}
    if (!result.contains(SIGN_END)) {emit signalError(QString("invalid NODE_JS result, not found sign [%1]").arg(SIGN_END)); return;}

    QString s_json = LString::strBetweenStr(result, SIGN_START, SIGN_END);
    emit signalMsg("---------NODE_JS result----------");
    emit signalMsg(s_json);
    s_json = transformJsonResult(s_json);
    emit signalMsg("--------- transformed result----------");
    emit signalMsg(s_json);
    //return;

    // Парсим JSON-ответ
    QJsonParseError parseError;
    QJsonDocument outputDoc = QJsonDocument::fromJson(s_json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        emit signalError(QString("Ошибка парсинга JSON: %1").arg(parseError.errorString()));
    }
    else
    {
        QJsonObject resultObj = outputDoc.object();
        emit signalMsg("JSON result was parsed!");
        slotJsonReply(rtEthers, resultObj);
    }
}
void EthersPage::startProcessObj()
{
    emit signalMsg(QString("%1  start node_js process %2").arg(LTime::strCurrentTime()).arg(LString::symbolString('.', 50)));
    emit signalMsg(QString("command [%1]").arg(m_procObj->fullCommand()));
    emit signalEnableControls(false);
    m_procObj->startCommand();
}
void EthersPage::slotCheckUpproved(QString token_addr)
{
    qDebug()<<QString("%1 .... tryUpdateApproved sum for token_addr (%2)").arg(LTime::strCurrentTime()).arg(token_addr);

    QStringList args;
    args << m_approvePage->scriptName() << token_addr;// << "pos_manager";
    m_procObj->setArgs(args);

    startProcessObj();
}
void EthersPage::slotApprove(const QStringList &tx_params)
{
    qDebug()<<QString("%1 .... slotApprove sum for token_addr (%2)").arg(LTime::strCurrentTime()).arg(tx_params.first());

    QStringList args;
    args << m_approvePage->scriptName();
    args.append(tx_params);
    m_procObj->setArgs(args);

    startProcessObj();
}
void EthersPage::slotCheckTx(const QStringList &args)
{
    qDebug()<<QString("%1 .... slotCheckTx, args %2").arg(LTime::strCurrentTime()).arg(args.count());
    m_procObj->setArgs(args);
    startProcessObj();
}
void EthersPage::slotPoolAction(const QStringList &args)
{
    qDebug()<<QString("%1 .... slotPoolAction, args %2").arg(LTime::strCurrentTime()).arg(args.count());
    m_procObj->setArgs(args);
    startProcessObj();
}
void EthersPage::slotWalletTx(const QStringList &args)
{
    qDebug()<<QString("%1 .... slotWalletTx, args %2").arg(LTime::strCurrentTime()).arg(args.count());
    m_procObj->setArgs(args);
    startProcessObj();
}
void EthersPage::slotJsonReply(int req_type, const QJsonObject &j_result)
{
    if (req_type != userSign()) return;

    if (j_result.keys().contains("error"))
    {
        emit signalError(j_result.value("error").toString());
        emit signalScriptBroken();
        return;
    }

    if (walletPageNow())
    {
        m_walletPage->parseJSResult(j_result);
    }
    else if (approvePageNow())
    {
        m_approvePage->parseJSResult(j_result);
    }
    else if (txPageNow())
    {
        m_txPage->parseJSResult(j_result);
    }
    else if (poolsPageNow())
    {
        m_poolPage->parseJSResult(j_result);
    }
}
void EthersPage::load(QSettings &settings)
{
    UG_BasePage::load(settings);
    int i_tab = settings.value(QString("%1/tab_index").arg(objectName()), -1).toInt();
    if (i_tab >= 0) m_tab->tab()->setCurrentIndex(i_tab);
}
void EthersPage::save(QSettings &settings)
{
    UG_BasePage::save(settings);
    settings.setValue(QString("%1/tab_index").arg(objectName()), m_tab->tab()->currentIndex());
}

//private
QString EthersPage::transformJsonResult(const QString &str_json) const
{
    QString s = str_json.trimmed();
    qDebug()<<QString("transformJsonResult STAGE1: %1").arg(s);
    if (s.isEmpty()) return s;
    int len = s.length();
    if (s.at(0) != '{' || s.at(len-1) != '}') return QString("invalid_json");

    s = LString::strBetweenStr(s, "{", "}");
    s = LString::removeLongSpaces(s);
    s = LString::removeSymbol(s, QChar('\n'));
    s = LString::removeSymbol(s, QUTE_SYMBOL1);
    s = LString::removeSymbol(s, QUTE_SYMBOL2);
    s = s.trimmed();
    len = s.length();
    if (s.at(len-1) == ',') {s = LString::strTrimRight(s, 1); s = s.trimmed(); len = s.length();}

  //  qDebug()<<QString("transformJsonResult STAGE2: %1").arg(s);

    QStringList list = LString::trimSplitList(s, ",");
    qDebug()<<QString("----------json pairs %1-------------").arg(list.count());
    s.clear();
    for(int i=0; i<list.count(); i++)
    {
        qDebug()<<QString("PAIR_%1  [%2]").arg(i+1).arg(list.at(i));
        if (!s.isEmpty()) s.append(QString(", "));

        QString field = list.at(i).trimmed();
        int pos = field.indexOf(QChar(':'));
        if (pos > 0)
        {
            //qDebug()<<QString("pos = %1").arg(pos);
            QString f_name = field.left(pos).trimmed();
            QString f_value = LString::strTrimLeft(field, pos+1).trimmed();
            QString j_line = QString("%1%2%1 : %1%3%1").arg(QUTE_SYMBOL1).arg(f_name).arg(f_value);
           // qDebug()<<QString("     JLINE:  [%1]").arg(j_line);
            s.append(j_line);
        }
        else s.append(field);
    }

    s.prepend(QChar('{'));
    s.append(QChar('}'));
    return s;
}


