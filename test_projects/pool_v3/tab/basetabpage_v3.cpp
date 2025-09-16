#include "basetabpage_v3.h"
#include "deficonfig.h"
#include "appcommonsettings.h"
#include "txdialog.h"
#include "nodejsbridge.h"

#include <QSettings>
#include <QSplitter>
#include <QByteArray>
#include <QDebug>
#include <QJsonObject>



//BaseTabPage_V3
BaseTabPage_V3::BaseTabPage_V3(QWidget *parent, int t_lay, int page_kind)
    :LSimpleWidget(parent, t_lay),
      m_table(NULL),
      m_integratedTable(NULL)
{
    setObjectName("v3_chain_tab_page");
    m_userSign = page_kind;
}
void BaseTabPage_V3::setChain(int chain_id)
{
    m_userData = "none";
    int pos = defi_config.chainIndexOf(chain_id);
    if (pos >= 0) m_userData = defi_config.chains.at(pos).name.trimmed();
}
void BaseTabPage_V3::load(QSettings &settings)
{
    //qDebug()<<QString("BaseTabPage_V3::load parent[%1]  self[%2]").arg(userData()).arg(objectName());
    QByteArray ba = settings.value(QString("%1/%2/v_spltitter_state").arg(userData()).arg(objectName()), QByteArray()).toByteArray();
    if (v_splitter && !ba.isEmpty()) v_splitter->restoreState(ba);

    ba.clear();
    ba = settings.value(QString("%1/%2/h_spltitter_state").arg(userData()).arg(objectName()), QByteArray()).toByteArray();
    if (h_splitter && !ba.isEmpty()) h_splitter->restoreState(ba);

}
void BaseTabPage_V3::save(QSettings &settings)
{
    //qDebug()<<QString("BaseTabPage_V3::save parent[%1]  self[%2]").arg(parent()->objectName()).arg(objectName());
    if (v_splitter)
        settings.setValue(QString("%1/%2/v_spltitter_state").arg(userData()).arg(objectName()), v_splitter->saveState());

    if (h_splitter)
        settings.setValue(QString("%1/%2/h_spltitter_state").arg(userData()).arg(objectName()), h_splitter->saveState());
}
void BaseTabPage_V3::sendReadNodejsRequest(const QJsonObject &j_params)
{
    emit signalRewriteJsonFile(j_params, AppCommonSettings::readParamsNodeJSFile()); //rewrite params json-file

    QStringList args;
    args << "reader.js" << AppCommonSettings::readParamsNodeJSFile();
    emit signalRunNodejsBridge(j_params.value(AppCommonSettings::nodejsReqFieldName()).toString(), args);
}
void BaseTabPage_V3::sendTxNodejsRequest(const TxDialogData &tx_data /*const QJsonObject &j_params*/)
{
    emit signalMsg(QString("Try send TX [%1]").arg(NodejsBridge::jsonCommandValue(tx_data.tx_kind)));
    if (tx_data.dialog_params.contains("error")) {emit signalError(tx_data.dialog_params.value("error")); return;}

    QJsonObject j_params;
    QStringList keys(tx_data.dialog_params.keys());
    foreach (const QString &v, keys)
        j_params.insert(v, tx_data.dialog_params.value(v));

    // second part
    emit signalRewriteJsonFile(j_params, AppCommonSettings::txParamsNodeJSFile()); //rewrite params json-file

    QStringList args;
    args << "tx_writer.js" << AppCommonSettings::txParamsNodeJSFile();
    emit signalRunNodejsBridge(j_params.value(AppCommonSettings::nodejsReqFieldName()).toString(), args);
}
