#include "bb_shadowexplorer.h"
#include "apiconfig.h"
#include "lhttp_types.h"
#include "bb_apistruct.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

#define API_KLINE_URI           QString("v5/market/mark-price-kline")


//BB_ShadowExplorer
BB_ShadowExplorer::BB_ShadowExplorer(QWidget *parent)
    :BB_BasePage(parent, 32, rtShadows)
{
    setObjectName("shadows_page");
    /*
    init();
    loadTickers();
    initChart();
    initSearch();


    m_reqData->params.insert("category", "linear");
    m_reqData->uri = API_CHART_URI;

    connect(w_listAll->listWidget(), SIGNAL(clicked(QModelIndex)), this, SLOT(slotTickerChanged()));
    connect(w_listFavor->listWidget(), SIGNAL(clicked(QModelIndex)), this, SLOT(slotTickerChanged()));
    */

}
void BB_ShadowExplorer::updateDataPage(bool forcibly)
{
    if (!forcibly) return;

    /*
    QString ticker = selectedTicker();
    if (ticker.length() < 2) emit signalError(QString("invalid ticker: %1").arg(ticker));
    else requestByTicker(ticker.append("USDT"));
    */
}
void BB_ShadowExplorer::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != userSign()) return;

    //w_chart->clearChartPoints();

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_ChartPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_ChartPage: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_ChartPage: j_arr QJsonArray is empty"); return;}

    /*
    QList<QPointF> points;
    fillPoints(points, j_arr);
    repaintChart(points);

    if (WRITE_TO_FILE) writePricesFile(selectedTicker().trimmed(), points);
    */
}



