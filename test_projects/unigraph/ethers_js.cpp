#include "ethers_js.h"
#include "ug_apistruct.h"
#include "processobj.h"


#include <QDebug>

#define SCRIPTS_PATH    QString("/home/roman/work/my/github_repos/repo_html/nodejs/crypto")

//EthersPage
EthersPage::EthersPage(QWidget *parent)
    :UG_BasePage(parent, 10, rtEthers),
      m_procObj(NULL)
{
    setObjectName("ethers_page");
    //initWidgets();
    //initQueryBox();

    m_procObj = new LProcessObj(this);
    connect(m_procObj, SIGNAL(signalFinished()), this, SLOT(slotScriptFinished()));

    m_procObj->setCommand("node");
    m_procObj->setDebugLevel(5);
}
void EthersPage::slotScriptFinished()
{
    emit signalMsg("-------------slotScriptFinished----------------");
    qDebug("-------------slotScriptFinished----------------");

    QStringList out(m_procObj->bufferList());
    qDebug()<<QString("BUFF:")<<m_procObj->buffer();
    foreach (const QString &v, out)
        emit signalMsg(v);

    emit signalMsg("\n script finished!");

}
void EthersPage::startUpdating(quint16 t)
{
    qDebug("EthersPage::startUpdating");

    m_procObj->setCommand("run_script");


    QStringList list;
    //QString script_name("balance_at");
    QString script_name("gas.js");
    //list << QString("%1/%2.js").arg(SCRIPTS_PATH).arg(script_name);
    list << script_name;
    m_procObj->setArgs(list);
    emit signalMsg("start process ....");
    emit signalMsg(QString("command [%1]").arg(m_procObj->fullCommand()));
    m_procObj->startCommand();


    qDebug()<<QString("STATE: %1").arg(m_procObj->strProcessState());
    qDebug()<<QString("CMD: %1").arg(m_procObj->fullCommand());

}
