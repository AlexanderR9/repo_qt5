#include "bb_chartpage.h"
#include "lchart.h"
#include "apiconfig.h"
#include "lhttp_types.h"
#include "bb_apistruct.h"


#include <QSplitter>
#include <QSettings>
#include <QDebug>
#include <QDateTime>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QPointF>



#define API_CHART_URI       QString("v5/market/mark-price-kline")

//BB_ChartPage
BB_ChartPage::BB_ChartPage(QWidget *parent)
    :BB_BasePage(parent, 32, rtCandles),
      w_listAll(NULL),
      w_listFavor(NULL),
      w_chart(NULL)
{
    setObjectName("chart_page");
    init();
    loadTickers();
    initChart();

    m_reqData->params.insert("category", "linear");
    m_reqData->uri = API_CHART_URI;

    connect(w_listAll->listWidget(), SIGNAL(currentRowChanged(int)), this, SLOT(slotTickerChanged(int)));

}
void BB_ChartPage::slotTickerChanged(int i)
{
    if (i < 0 || i > w_listAll->listWidget()->count()) return;

    int row = w_listAll->listWidget()->currentRow();
    if (row < 0)
    {
        emit signalError("You must select API source.");
        return;
    }

    QString ticker = QString("%1%2").arg(w_listAll->listWidget()->item(row)->text()).arg("USDT");
    m_reqData->params.insert("symbol", ticker);
    m_reqData->params.insert("interval", api_config.candle.size);
    sendRequest(api_config.candle.number, ticker);
}
void BB_ChartPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    BB_BasePage::slotJsonReply(req_type, j_obj);
    if (req_type != rtCandles) return;

    w_chart->clearChartPoints();

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_ChartPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_ChartPage: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_ChartPage: j_arr QJsonArray is empty"); return;}

    bool ok;
    QList<QPointF> points;
    for (int i=0; i<j_arr.count(); i++)
    {
        const QJsonArray &j_el = j_arr.at(i).toArray();
        if (j_el.isEmpty()) {emit signalError(QString("BB_ChartPage: j_el QJsonArray is empty, i=%1").arg(i)); return;}
        //qDebug()<<QString("j_el.count(%1)  %2").arg(i).arg(j_el.count());

        float a = j_el.at(4).toString().toFloat(&ok);
        if (!ok) {emit signalError(QString("BB_ChartPage: invalid convert QJsonValue(%1) to float , i=%2").arg(j_el.at(4).toString()).arg(i)); return;}

        qint64 dt = qint64(j_el.at(0).toString().toDouble(&ok));
        if (!ok) {emit signalError(QString("BB_ChartPage: invalid convert QJsonValue(%1) to int64 , i=%2").arg(j_el.at(0).toString()).arg(i)); return;}

        points.append(QPointF(QDateTime::fromMSecsSinceEpoch(dt).toSecsSinceEpoch(), a));
    }

    repaintChart(points);
}
void BB_ChartPage::repaintChart(const QList<QPointF> &points)
{
    if (points.isEmpty())
    {
        int row = w_listAll->listWidget()->currentRow();
        QString err = QString("Not found chart data for %1").arg(w_listAll->listWidget()->item(row)->text());
        emit signalError(err);
    }
    else
    {
        w_chart->addChartPoints(points, 0);
        emit signalMsg(w_chart->strMinMax());
    }
    w_chart->updateAxis();
}
void BB_ChartPage::init()
{
    w_listAll = new LListWidgetBox(this);
    w_listAll->setTitle("Tickers");
    w_listAll->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    w_listFavor = new LListWidgetBox(this);
    w_listFavor->setTitle("Favor");
    w_chart = new LChartWidget(this);

    delete v_splitter;
    v_splitter = NULL;
    h_splitter->addWidget(w_chart);
    v_splitter = new QSplitter(Qt::Vertical, this);
    h_splitter->addWidget(v_splitter);
    v_splitter->addWidget(w_listAll);
    v_splitter->addWidget(w_listFavor);
}
void BB_ChartPage::initChart()
{
    w_chart->setCrossColor(QColor(150, 190, 150));
    w_chart->setAxisXType(LChartAxisParams::xvtDate);
    w_chart->setCrossXAxisTextViewMode(2);
    w_chart->setPointSize(2);

    LChartParams p;
    p.lineColor = QColor(170, 50, 40);
    p.pointsColor = QColor(0, 100, 0);
    p.points.clear();
    w_chart->addChart(p);
    w_chart->updateAxis();

}
void BB_ChartPage::loadTickers()
{
    qDebug()<<QString("api_config.tickers %1").arg(api_config.tickers.count());
    foreach (const QString &v, api_config.tickers)
    {
        w_listAll->addItem(v);
    }

}

