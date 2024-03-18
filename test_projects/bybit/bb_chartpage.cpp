#include "bb_chartpage.h"
#include "lchart.h"
#include "apiconfig.h"
#include "lhttp_types.h"
#include "bb_apistruct.h"
#include "lsearch.h"


#include <QSplitter>
#include <QSettings>
#include <QDebug>
#include <QDateTime>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QPointF>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>



#define API_CHART_URI       QString("v5/market/mark-price-kline")

//BB_ChartPage
BB_ChartPage::BB_ChartPage(QWidget *parent)
    :BB_BasePage(parent, 32, rtCandles),
      w_listAll(NULL),
      w_listFavor(NULL),
      w_chart(NULL),
      m_searchObj(NULL),
      m_searchObj_F(NULL),
      m_searchEdit(NULL),
      m_searchLabel(NULL),
      m_searchLabel_F(NULL)
{
    setObjectName("chart_page");
    init();
    loadTickers();
    initChart();
    initSearch();


    m_reqData->params.insert("category", "linear");
    m_reqData->uri = API_CHART_URI;

    connect(w_listAll->listWidget(), SIGNAL(clicked(QModelIndex)), this, SLOT(slotListClicked()));
    connect(w_listFavor->listWidget(), SIGNAL(clicked(QModelIndex)), this, SLOT(slotListClicked()));
    connect(w_listAll->listWidget(), SIGNAL(currentRowChanged(int)), this, SLOT(slotTickerChanged(int)));
    connect(w_listFavor->listWidget(), SIGNAL(currentRowChanged(int)), this, SLOT(slotTickerChanged(int)));

}
void BB_ChartPage::slotTickerChanged(int i)
{
    QListWidget *lw = qobject_cast<QListWidget*>(sender());
    if (!lw) return;
    if (i < 0 || i > lw->count()) return;

    int row = lw->currentRow();
    if (row < 0)
    {
        emit signalError("You must select API source.");
        return;
    }

    QString ticker = QString("%1%2").arg(lw->item(row)->text()).arg("USDT");
    m_reqData->params.insert("symbol", ticker);
    m_reqData->params.insert("interval", api_config.candle.size);
    qDebug()<<QString("send req by ticker %1").arg(ticker);
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
    w_listAll->listWidget()->setObjectName("all_list");
    w_listAll->setTitle("Tickers");
    w_listAll->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    w_listFavor = new LListWidgetBox(this);
    w_listFavor->listWidget()->setObjectName("favor_list");
    w_listFavor->setTitle("Favor");
    w_listFavor->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    w_chart = new LChartWidget(this);

    m_searchEdit = new QLineEdit(this);
    m_searchLabel = new QLabel("Count: ", this);
    w_listAll->layout()->addWidget(m_searchLabel);
    m_searchLabel_F = new QLabel("Count: ", this);
    w_listFavor->layout()->addWidget(m_searchLabel_F);

    delete v_splitter;
    v_splitter = NULL;

    QFrame *frame = new QFrame();
    if (frame->layout()) {delete frame->layout();}
    frame->setLayout(new QVBoxLayout(0));
    h_splitter->addWidget(w_chart);
    h_splitter->addWidget(frame);

    v_splitter = new QSplitter(Qt::Vertical, this);
    h_splitter->addWidget(v_splitter);
    v_splitter->addWidget(w_listAll);
    v_splitter->addWidget(w_listFavor);

    frame->layout()->addWidget(m_searchEdit);
    frame->layout()->addWidget(v_splitter);
}
void BB_ChartPage::initSearch()
{
    m_searchObj = new LSearch(m_searchEdit, this);
    m_searchObj->addList(w_listAll->listWidget(), m_searchLabel);
    m_searchObj->exec();

    m_searchObj_F = new LSearch(m_searchEdit, this);
    m_searchObj_F->addList(w_listFavor->listWidget(), m_searchLabel_F);
    m_searchObj_F->exec();

    //connect(m_searchObj, SIGNAL(signalSearched()), this, SIGNAL(signalSearched()));

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
    //qDebug()<<QString("api_config.tickers %1").arg(api_config.tickers.count());
    foreach (const QString &v, api_config.tickers)
        if (!v.trimmed().isEmpty()) w_listAll->addItem(v);

    if (api_config.favor_tickers.isEmpty()) return;
    foreach (const QString &v, api_config.favor_tickers)
        if (!v.trimmed().isEmpty()) w_listFavor->addItem(v);
}
void BB_ChartPage::slotListClicked()
{
    //qDebug()<<sender()->objectName();
    if (sender()->objectName().contains("all"))
    {
        //qDebug("slotListClicked all");
        w_listFavor->clearSelection();
    }
    else
    {
        //qDebug("slotListClicked favor");
        w_listAll->clearSelection();
    }
}
void BB_ChartPage::addFavorToken()
{
    QStringList list(w_listAll->selectedValues());
    if (list.isEmpty()) return;

    w_listAll->clearSelection();
    w_listFavor->clearSelection();

    foreach (const QString &v, list)
    {
        if (!w_listFavor->valueContain(v))
        {
            w_listFavor->addItem(v);
            if (!api_config.favor_tickers.contains(v)) api_config.favor_tickers.append(v);
        }
    }

    m_searchObj_F->exec();
}
void BB_ChartPage::removeFavorToken()
{
    QStringList list(w_listFavor->selectedValues());
    if (list.isEmpty()) return;

    w_listAll->clearSelection();
    w_listFavor->clearSelection();
    foreach (const QString &v, list)
    {
        w_listFavor->removeItemByValue(v);
        int pos = api_config.favor_tickers.indexOf(v);
        if (pos >= 0) api_config.favor_tickers.removeAt(pos);
    }
    m_searchObj_F->exec();

}

