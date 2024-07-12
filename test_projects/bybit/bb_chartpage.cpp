#include "bb_chartpage.h"
#include "lchart.h"
#include "apiconfig.h"
#include "lhttp_types.h"
#include "bb_apistruct.h"
#include "lsearch.h"
#include "lfile.h"


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
#include <QDir>
#include <QVBoxLayout>



#define API_CHART_URI           QString("v5/market/mark-price-kline")
#define API_FUND_RATE_URI       QString("v5/market/funding/history")
#define WRITE_TO_FILE           false //если этот флаг взведен то цены будут писаться в файл prices.txt (временная мера для сбора данных)


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

    connect(w_listAll->listWidget(), SIGNAL(clicked(QModelIndex)), this, SLOT(slotTickerChanged()));
    connect(w_listFavor->listWidget(), SIGNAL(clicked(QModelIndex)), this, SLOT(slotTickerChanged()));

}
void BB_ChartPage::updateDataPage(bool forcibly)
{
    if (!forcibly) return;

    QString ticker = selectedTicker();
    if (ticker.length() < 2) emit signalError(QString("invalid ticker: %1").arg(ticker));
    else requestByTicker(ticker.append("USDT"));
}
void BB_ChartPage::slotTickerChanged()
{
    QListWidget *lw = qobject_cast<QListWidget*>(sender());
    if (!lw) return;

    if (lw->objectName().contains("all")) {if (w_listFavor) w_listFavor->clearSelection();}
    else {if (w_listAll) w_listAll->clearSelection();}
    if (lw->count() == 0) {emit signalError(QString("view has't items")); return;}

    QString ticker = selectedTicker();
    if (ticker.length() < 2) {emit signalError(QString("invalid ticker: %1").arg(ticker)); return;}

    requestByTicker(ticker.append("USDT"));
}
void BB_ChartPage::requestByTicker(QString ticker)
{
    m_reqData->params.insert("symbol", ticker);
    m_reqData->params.insert("interval", api_config.candle.size);
    sendRequest(api_config.candle.number, ticker);
}
void BB_ChartPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != userSign()) return;

    w_chart->clearChartPoints();

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_ChartPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_ChartPage: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_ChartPage: j_arr QJsonArray is empty"); return;}

    QList<QPointF> points;
    fillPoints(points, j_arr);
    repaintChart(points);

    if (WRITE_TO_FILE) writePricesFile(selectedTicker().trimmed(), points);
}
void BB_ChartPage::writePricesFile(QString ticker, const QList<QPointF> &points)
{

    qDebug()<<QString("writePricesFile ticker=%1, points %2").arg(ticker).arg(points.count());
    if (ticker.length() < 2) return;

    //prepare data
    QString fdata(QString("\n\n"));
    int ls = 5;
    foreach (const QPointF &p, points)
    {
        ls--;
        BB_PricesContainer::PricePoint pp(ticker, p.y(), qint64(p.x()));
        fdata.append(pp.toFileElement());
        fdata.append(QChar(' '));
        if (ls < 0) {fdata.append(QChar('\n')); ls = 5;}
    }
    fdata.append(QChar('\n'));


    //append file
    QString fname(QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(BB_PricesContainer::priceFile()));
    QString err = LFile::appendFile(fname, fdata);
    if (!err.isEmpty()) signalError(err);
    else signalMsg(QString("File prices(%1) was appended OK!").arg(ticker));
}
void BB_ChartPage::fillPoints(QList<QPointF> &points, const QJsonArray &j_arr)
{
    bool ok;
    points.clear();
    for (int i=0; i<j_arr.count(); i++)
    {
        const QJsonArray &j_el = j_arr.at(i).toArray();
        if (j_el.isEmpty()) {emit signalError(QString("BB_ChartPage: j_el QJsonArray is empty, i=%1").arg(i)); return;}

        float a = j_el.at(4).toString().toFloat(&ok);
        if (!ok) {emit signalError(QString("BB_ChartPage: invalid convert QJsonValue(%1) to float , i=%2").arg(j_el.at(4).toString()).arg(i)); return;}

        qint64 dt = qint64(j_el.at(0).toString().toDouble(&ok));
        if (!ok) {emit signalError(QString("BB_ChartPage: invalid convert QJsonValue(%1) to int64 , i=%2").arg(j_el.at(0).toString()).arg(i)); return;}

        points.append(QPointF(QDateTime::fromMSecsSinceEpoch(dt).toSecsSinceEpoch(), a));
    }
}
void BB_ChartPage::repaintChart(const QList<QPointF> &points)
{
    if (points.isEmpty())
    {
        //int row = w_listAll->listWidget()->currentRow();
        QString err = QString("Not found chart data for %1").arg(selectedTicker());
        emit signalError(err);
    }
    else
    {
        float a = 0;
        foreach (const QPointF &p, points) a += p.y();
        a /= points.count();

        w_chart->addChartPoints(points, 0);
        emit signalMsg(w_chart->strMinMax());

        quint8 preq = 1;
        if (a < 10) preq = 2;
        if (a < 2) preq = 4;
        w_chart->setAxisPrecision(-1, preq);
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

    QFrame *frame = new QFrame(this);
    frame->setObjectName("frame_obj");
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
    foreach (const QString &v, api_config.tickers)
        if (!v.trimmed().isEmpty()) w_listAll->addItem(v);

    if (api_config.favor_tickers.isEmpty()) return;
    foreach (const QString &v, api_config.favor_tickers)
        if (!v.trimmed().isEmpty()) w_listFavor->addItem(v);
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
QString BB_ChartPage::selectedTicker() const
{
    if (w_listAll)
    {
        QStringList list(w_listAll->selectedValues());
        if (!list.isEmpty()) return list.first();
    }
    if (w_listFavor)
    {
        QStringList list(w_listFavor->selectedValues());
        if (!list.isEmpty()) return list.first();
    }
    return QString();
}



//BB_FundRatePage
BB_FundRatePage::BB_FundRatePage(QWidget *parent)
    :BB_ChartPage(parent),
      m_endDateEdit(NULL)
{
    setObjectName("fund_rate_page");
    m_userSign = rtFundRate;
    m_reqData->req_type = m_userSign;

    m_reqData->uri = API_FUND_RATE_URI;

    m_endDateEdit = new QLineEdit(this);
    m_endDateEdit->setText(QDate::currentDate().toString(APIConfig::userDateMask()));

    reinitSearchObj();
    w_chart->setAxisPrecision(0, 3);
}
void BB_FundRatePage::reinitSearchObj()
{
    delete m_searchObj; m_searchObj = NULL;
    delete m_searchObj_F; m_searchObj_F = NULL;
    delete m_searchLabel; m_searchLabel = NULL;


    const QObjectList &childs = h_splitter->children();
    foreach (QObject *v, childs)
    {
        if (v->objectName() == "frame_obj")
        {
            qobject_cast<QFrame*>(v)->layout()->removeWidget(m_searchEdit);
            delete m_searchEdit;
            m_searchEdit = NULL;
        }
    }

    delete w_listAll; w_listAll = NULL;

    w_listFavor->layout()->addWidget(m_endDateEdit);

}
void BB_FundRatePage::requestByTicker(QString ticker)
{
    m_reqData->params.insert("symbol", ticker);
    if (m_reqData->params.contains("interval")) m_reqData->params.remove("interval");

    QDate date = endDate();
    qint64 ts = APIConfig::toTimeStamp(date.day(), date.month(), date.year());
    m_reqData->params.insert("endTime", QString::number(ts));

    date = QDate::currentDate().addDays(-100);
    ts = APIConfig::toTimeStamp(date.day(), date.month(), date.year());
    m_reqData->params.insert("startTime", QString::number(ts));

    sendRequest(500, ticker);
}
void BB_FundRatePage::fillPoints(QList<QPointF> &points, const QJsonArray &j_arr)
{
    bool ok;
    points.clear();
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_FundRatePage::fillTable WARNING j_el is empty (index=%1)").arg(i); break;}

        QString s_a(j_el.value("fundingRate").toString());
        float a = s_a.toFloat(&ok);
        if (!ok) {emit signalError(QString("BB_FundRatePage: invalid convert QJsonValue(%1) to float , i=%2").arg(s_a).arg(i)); break;}

        s_a = j_el.value("fundingRateTimestamp").toString();
        qint64 dt = qint64(s_a.toDouble(&ok));
        if (!ok) {emit signalError(QString("BB_FundRatePage: invalid convert QJsonValue(%1) to int64 , i=%2").arg(s_a).arg(i)); break;}

        points.append(QPointF(QDateTime::fromMSecsSinceEpoch(dt).toSecsSinceEpoch(), a*100));
    }
}
QDate BB_FundRatePage::endDate() const
{
    QString s(m_endDateEdit->text().trimmed());
    if (!s.isEmpty())
    {
        QDate d = QDate::fromString(s, APIConfig::userDateMask());
        if (d.isValid()) return d;
    }
    return QDate::currentDate();
}



