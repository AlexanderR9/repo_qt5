#include "ethers_js.h"
#include "ug_apistruct.h"
#include "processobj.h"
#include "jswallettab.h"
#include "jsapprovetab.h"
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



#define SIGN_START     QString("JSON_RESULT_START")
#define SIGN_END       QString("JSON_RESULT_END")


//EthersPage
EthersPage::EthersPage(QWidget *parent)
    :UG_BasePage(parent, 10, rtEthers),
      m_procObj(NULL),
      m_tab(NULL),
      m_walletPage(NULL),
      m_approvePage(NULL)
{
    setObjectName("ethers_page");
    initWidgets();
    initProcessObj();

    m_walletPage->loadAssetsFromFile();
    m_approvePage->setTokens(m_walletPage->assetsTokens());
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



    connect(m_walletPage, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_walletPage, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));
    connect(m_approvePage, SIGNAL(signalMsg(QString)), this, SIGNAL(signalMsg(QString)));
    connect(m_approvePage, SIGNAL(signalError(QString)), this, SIGNAL(signalError(QString)));

}
void EthersPage::slotScriptFinished()
{
    emit signalMsg(QString("%1  node_js done!").arg(LTime::strCurrentTime()));
//    qDebug()<<QString("%1 .... QProcess finished").arg(LTime::strCurrentTime());
    qDebug()<<QString("BUFF:")<<m_procObj->buffer();

    parseResultBuffer();
}
void EthersPage::startUpdating(quint16 t)
{
    qDebug()<<QString("EthersPage::startUpdating t=%1").arg(t);
    if (walletPageNow())
    {
        tryUpdateBalace();
        return;
    }
}
bool EthersPage::walletPageNow() const
{
    return m_tab->tab()->currentWidget()->objectName().contains("wallet");
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

    s_json = s_json.trimmed();
    s_json = LString::removeLongSpaces(s_json);
    s_json.replace(QChar('\n'), QString());
    s_json.replace(QChar('\''), QString("\""));
    //qDebug("---------------------------");
    //qDebug()<<s_json;



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
        slotJsonReply(-1, resultObj);
    }
}
void EthersPage::slotJsonReply(int code, const QJsonObject &j_result)
{
    Q_UNUSED(code);
    if (j_result.keys().contains("error"))
    {
        emit signalError(j_result.value("error").toString());
        return;
    }

    if (walletPageNow())
    {
        m_walletPage->updateBalances(j_result);
    }
}

