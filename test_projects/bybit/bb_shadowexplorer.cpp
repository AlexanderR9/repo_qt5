#include "bb_shadowexplorer.h"
#include "apiconfig.h"
#include "lhttp_types.h"
#include "ltable.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>
#include <QSplitter>
#include <QLineEdit>
#include <QLayout>


#define API_KLINE_URI           QString("v5/market/mark-price-kline")



//BB_ShadowExplorer
BB_ShadowExplorer::BB_ShadowExplorer(QWidget *parent)
    :BB_BasePage(parent, 32, rtShadows),
      m_tickerTable(NULL),
      m_pivotTable(NULL),
      m_resultTable(NULL)
{
    setObjectName("shadows_page");
    init();
    loadTickers();

    resetPrivotData();

    m_reqData->params.insert("category", "linear");
    m_reqData->uri = API_KLINE_URI;

    connect(m_tickerTable->table(), SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(slotTickerChanged(QTableWidgetItem*)));

}
void BB_ShadowExplorer::slotTickerChanged(QTableWidgetItem *item)
{
    resetPrivotData();
    if (!item) return;

    //qDebug()<<QString("slotTickerChanged  ticker=%1").arg(item->text());
    if (api_config.favor_tickers.contains(item->text()))
    {
        m_pivotTable->table()->item(0, 0)->setText(item->text());
        updateDataPage(true);
    }
}
void BB_ShadowExplorer::resetPrivotData()
{
    m_pivotData.reset();
    if (!m_pivotTable) return;

    for (int i=0; i<m_pivotTable->table()->rowCount(); i++)
        LTable::createTableItem(m_pivotTable->table(), i, 0, "-");

    m_pivotTable->resizeByContents();

    m_resultTable->removeAllRows();
    m_resultTable->resizeByContents();
}
void BB_ShadowExplorer::init()
{
    initTickersTable();
    initPivotTable();
    initResultTable();

    h_splitter->addWidget(m_pivotTable);
    v_splitter->addWidget(m_tickerTable);
    v_splitter->addWidget(m_resultTable);
}
void BB_ShadowExplorer::initTickersTable()
{
    //ticker table
    m_tickerTable = new LTableWidgetBox(this);
    m_tickerTable->setObjectName("ticker_table");
    m_tickerTable->setTitle("Symbol list");
    m_tickerTable->vHeaderHide();
    m_tickerTable->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    //headers
    QStringList headers;
    headers << "Ticker" <<  "Last price" << "RSI, %" << "Testing result";
    m_tickerTable->setHeaderLabels(headers);
    m_tickerTable->resizeByContents();

}
void BB_ShadowExplorer::initPivotTable()
{
    //ticker table
    m_pivotTable = new LTableWidgetBox(this);
    m_pivotTable->setObjectName("pivot_table");
    m_pivotTable->setTitle("Pivot information by symbol");
    m_pivotTable->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::NoSelection);
    m_pivotTable->setHeaderLabels(QStringList()<<"Value");

    QStringList headers;
    headers << "Ticker" <<  "Period" << "Candles count" << "Time interval" << "Prices span";
    headers << "Min/Max price" << "Current RSI, %" << "Average shadow size (over/under)";
    headers << "Max over shadow size" << "Max under shadow size";
    headers << "User limit size, %" << "Has over limit shadow candles" << "Has under limit shadow candles";
    headers << "Testing total result";
    m_pivotTable->setHeaderLabels(headers, Qt::Vertical);
    m_pivotTable->resizeByContents();

    QLineEdit *edit = new QLineEdit(this);
    m_pivotTable->layout()->addWidget(edit);

}
void BB_ShadowExplorer::initResultTable()
{
    m_resultTable = new LTableWidgetBox(this);
    m_resultTable->setObjectName("result_table");
    m_resultTable->setTitle("Testing result");
    m_resultTable->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    //headers
    QStringList headers;
    headers << "Open price" <<  "Previos shadows (over/under)" << "Candle result, %" << "Total result";
    m_resultTable->setHeaderLabels(headers);
    m_resultTable->resizeByContents();
}
void BB_ShadowExplorer::loadTickers()
{
    m_tickerTable->removeAllRows();
    if (api_config.favor_tickers.isEmpty()) return;

    QTableWidget *t = m_tickerTable->table();
    foreach (const QString &v, api_config.favor_tickers)
    {
        QStringList row_data;
        row_data << v << "---" << "-1" << "0.0%";
        LTable::addTableRow(t, row_data);
    }
    m_tickerTable->resizeByContents();
}
void BB_ShadowExplorer::updateDataPage(bool forcibly)
{
    if (!forcibly) return;

    QString ticker = selectedTicker();
    if (ticker.length() < 2)
    {
        emit signalError(QString("invalid ticker: %1").arg(ticker));
        return;
    }

    ticker.append("USDT");
    m_reqData->params.insert("symbol", ticker);
    m_reqData->params.insert("interval", api_config.candle.size);
    //qDebug()<<api_config.candle.toStr();
    sendRequest(api_config.candle.number, ticker);
}
QString BB_ShadowExplorer::selectedTicker() const
{
    if (!m_pivotTable) return "?";
    return m_pivotTable->table()->item(0, 0)->text().trimmed();
}
int BB_ShadowExplorer::selectedRow() const
{
    QList<int> list(LTable::selectedRows(m_tickerTable->table()));
    return (list.isEmpty() ? -1 : list.first());
}
void BB_ShadowExplorer::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != userSign()) return;

    //w_chart->clearChartPoints();

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_ShadowExplorer: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (j_list.isNull()) {emit signalError("BB_ShadowExplorer: list QJsonValue not found"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    if (j_arr.isEmpty())  {emit signalError("BB_ShadowExplorer: j_arr QJsonArray is empty"); return;}


    emit signalGetLimitSize(m_pivotData.limit_shadow_size);

    receivedData(j_arr);

    updateResultTable();
    updatePivotTable();
    updateTickerRow();

}
void BB_ShadowExplorer::receivedData(const QJsonArray &j_arr)
{

    //qDebug()<<QString("BB_ShadowExplorer::recivedData  size %1").arg(j_arr.count());
    if (j_arr.count() < 10)
    {
        emit signalError(QString("BB_ShadowExplorer: j_arr size too small(%1)").arg(j_arr.count()));
        return;
    }

    for (int i=j_arr.count()-1; i>=0; i--)
    {
        const QJsonArray &j_el = j_arr.at(i).toArray();
        if (j_el.isEmpty()) {emit signalError(QString("BB_ShadowExplorer: j_el QJsonArray is empty, i=%1").arg(i)); return;}

        m_pivotData.addBar(j_el);
    }
}
void BB_ShadowExplorer::updateTickerRow()
{
    int row = selectedRow();
    if (row < 0) return;

    QTableWidget *t = m_tickerTable->table();
    t->item(row, 1)->setText(QString::number(m_pivotData.bars.last().p_close, 'f', 3));
    t->item(row, 2)->setText(QString::number(m_pivotData.currentRsi(), 'f', 1));
    t->item(row, 3)->setText(QString("%1%").arg(QString::number(m_pivotData.testing_result, 'f', 1)));

    m_tickerTable->resizeByContents();
}
void BB_ShadowExplorer::updatePivotTable()
{
    QTableWidget *t = m_pivotTable->table();
    int i = 1;
    t->item(i, 0)->setText(api_config.candle.size); i++;
    t->item(i, 0)->setText(QString::number(m_pivotData.bars.count())); i++;
    t->item(i, 0)->setText(m_pivotData.strTimeSpan()); i++;
    t->item(i, 0)->setText(m_pivotData.strPriceSpan()); i++;
    t->item(i, 0)->setText(m_pivotData.strMinMaxPrice()); i++;
    t->item(i, 0)->setText(QString::number(m_pivotData.currentRsi(), 'f', 1)); i++;
    t->item(i, 0)->setText(m_pivotData.strAverageShadows()); i++;
    t->item(i, 0)->setText(m_pivotData.strMinMaxShadows(true)); i++;
    t->item(i, 0)->setText(m_pivotData.strMinMaxShadows(false)); i++;
    t->item(i, 0)->setText(QString::number(m_pivotData.limit_shadow_size, 'f', 1)); i++;
    t->item(i, 0)->setText(m_pivotData.strHasLimitCandles(true)); i++;
    t->item(i, 0)->setText(m_pivotData.strHasLimitCandles(false)); i++;
    t->item(i, 0)->setText(QString("%1%").arg(QString::number(m_pivotData.testing_result, 'f', 1))); i++;

    m_pivotTable->resizeByContents();
}
void BB_ShadowExplorer::updateResultTable()
{
    int n_ok = 0;
    QTableWidget *t = m_resultTable->table();
    for (int i=0; i<m_pivotData.bars.count(); i++)
    {
        QStringList row_data(m_pivotData.candleResult(i, n_ok));
        LTable::addTableRow(t, row_data);

        QString s_res = row_data.at(2).trimmed();
        QTableWidgetItem *res_item = t->item(i, t->columnCount()-2);
        if (s_res.contains("OK")) res_item->setTextColor(Qt::darkGreen);
        else if (s_res == "none") res_item->setTextColor("#FF8C00");
        else if (s_res.contains("+")) res_item->setTextColor("#87CEEB");
        else res_item->setTextColor(Qt::darkRed);
    }
    m_resultTable->resizeByContents();
}


///////////////////////////////////////////////////////
void ShadowPrivotData::reset()
{
    bars.clear();
    volatility_factor = -1;
    limit_shadow_size = -1;
    testing_result = 0;
}
void ShadowPrivotData::addBar(const QJsonArray &b_arr)
{
    if (b_arr.count() != 5)
    {
        qWarning()<<QString("ShadowPrivotData::addBar - WARNING JSON BAR ARR size invalid(%1) != 5").arg(b_arr.count());
        return;
    }

    bool ok;
    BB_Bar bar;

    bar.start_time = qint64(b_arr.at(0).toString().toDouble(&ok));
    if (!ok) return;

    bar.p_open = b_arr.at(1).toString().toFloat(&ok);
    if (!ok) return;
    bar.p_close = b_arr.at(4).toString().toFloat(&ok);
    if (!ok) return;
    bar.p_high = b_arr.at(2).toString().toFloat(&ok);
    if (!ok) return;
    bar.p_low = b_arr.at(3).toString().toFloat(&ok);
    if (!ok) return;

    //Bar JSON data OK!
    bars.append(bar);
}
QString ShadowPrivotData::strTimeSpan() const
{
    QString s("---");
    if (bars.isEmpty()) return s;
    s = APIConfig::fromTimeStamp(bars.first().start_time, "dd.MM.yyyy");
    if (bars.count() == 1) return QString("%1 / ---").arg(s);
    s = QString("%1 / %2").arg(s).arg(APIConfig::fromTimeStamp(bars.last().start_time, "dd.MM.yyyy"));
    //return s;
    int days = QDateTime::fromMSecsSinceEpoch(bars.first().start_time).daysTo(QDateTime::fromMSecsSinceEpoch(bars.last().start_time));
    return QString("%1 (%2 days)").arg(s).arg(days);
}
QString ShadowPrivotData::strPriceSpan() const
{
    QString s("---");
    if (bars.isEmpty()) return s;

    quint8 prec = (bars.first().p_open < 1.5 ? 3 : 2);
    s = QString::number(bars.first().p_open, 'f', prec);
    if (bars.count() == 1) return QString("%1 / ---").arg(s);
    s = QString("%1 / %2 ").arg(s).arg(QString::number(bars.last().p_open, 'f', prec));

    float d = (bars.last().p_open - bars.first().p_open)/bars.first().p_open;
    s.append(QString("(%1%)").arg(QString::number(d*100, 'f', 1)));
    return s;

}
float ShadowPrivotData::minPrice() const
{
    float p = -1;
    foreach (const BB_Bar &v, bars)
        if (p < 0 || p > v.p_low) p = v.p_low;
    return p;
}
float ShadowPrivotData::maxPrice() const
{
    float p = -1;
    foreach (const BB_Bar &v, bars)
        if (p < v.p_high) p = v.p_high;
    return p;
}
QString ShadowPrivotData::strMinMaxPrice() const
{
    QString s("---");
    if (bars.isEmpty()) return s;

    float min = minPrice();
    float max = maxPrice();

    quint8 prec = (bars.first().p_open < 1.5 ? 3 : 2);
    s = QString::number(min, 'f', prec);
    s = QString("%1 / %2 ").arg(s).arg(QString::number(max, 'f', prec));


    float f = -1;
    if (min > 0 && max > 0) f = max/min;
    s.append(QString("(factor=%1)").arg(QString::number(f, 'f', 1)));
    return s;

}
float ShadowPrivotData::currentRsi() const
{
    float min = minPrice();
    float max = maxPrice();
    float rsi = -1;
    if (min > 0 && max > 0)
    {
        float a = bars.last().p_close - min;
        float b = max - min;
        rsi = float(100)*(a/b);
    }
    return rsi;
}
float ShadowPrivotData::averageOverShadow() const
{
    float a = 0;
    int n = 0;
    foreach (const BB_Bar &v, bars)
    {
        a += v.overShadow();
        n++;
    }
    return (n>0 ? a/n : a);
}
float ShadowPrivotData::averageUnderShadow() const
{
    float a = 0;
    int n = 0;
    foreach (const BB_Bar &v, bars)
    {
        a += v.underShadow();
        n++;
    }
    return (n>0 ? a/n : a);
}
QString ShadowPrivotData::strAverageShadows() const
{
    QString s("---");
    if (bars.isEmpty()) return s;

    quint8 prec = (bars.first().p_open < 1.5 ? 3 : 2);
    float sh_size = averageOverShadow();
    float d = sh_size/bars.last().p_open;
    s = QString("%1 (%2%)").arg(QString::number(sh_size, 'f', prec)).arg(QString::number(d*100, 'f', 1));
    sh_size = averageUnderShadow();
    d = sh_size/bars.last().p_open;
    s = QString("%1 / %2 (%3%) ").arg(s).arg(QString::number(sh_size, 'f', prec)).arg(QString::number(d*100, 'f', 1));
    return s;
}
QString ShadowPrivotData::strMinMaxShadows(bool over) const
{
    QString s("---");
    if (bars.isEmpty()) return s;

    quint8 prec = (bars.first().p_open < 1.5 ? 3 : 2);
    float sh_size = maxShadowSize(over);
    float d = sh_size/bars.last().p_open;
    s = QString("%1 (%2%)").arg(QString::number(sh_size, 'f', prec)).arg(QString::number(d*100, 'f', 1));
    //sh_size = maxShadowSize(over);
    //d = sh_size/bars.last().p_open;
    //s = QString("%1 / %2 (%3%) ").arg(s).arg(QString::number(sh_size, 'f', prec)).arg(QString::number(d*100, 'f', 1));
    return s;
}
QString ShadowPrivotData::strHasLimitCandles(bool over) const
{
    QString s("---");
    if (bars.isEmpty()) return s;

    int n = 0;
    float cp = bars.last().p_open;
    foreach (const BB_Bar &v, bars)
    {
        if (over && (v.overShadow_p(cp) > limit_shadow_size)) n++;
        if (!over && (v.underShadow_p(cp) > limit_shadow_size)) n++;
    }

    float t = float(n)/float(bars.count());
    s = QString("%1 (%2%)").arg(n).arg(QString::number(t*100, 'f', 1));
    return s;
}
float ShadowPrivotData::minShadowSize(bool over) const
{
    float a = -1;
    foreach (const BB_Bar &v, bars)
    {
        if (over)
        {
            if (a<0 || a > v.overShadow()) a = v.overShadow();
        }
        else
        {
            if (a<0 || a > v.underShadow()) a = v.underShadow();
        }
    }
    return a;
}
float ShadowPrivotData::maxShadowSize(bool over) const
{
    float a = 0;
    foreach (const BB_Bar &v, bars)
    {
        if (over)
        {
            if (a < v.overShadow()) a = v.overShadow();
        }
        else
        {
            if (a < v.underShadow()) a = v.underShadow();
        }
    }
    return a;
}
QStringList ShadowPrivotData::candleResult(int i, int &n_ok)
{
    QStringList list;
    list << "-" << "-" << "-" << "-";
    if (i < 0 || i >= bars.count()) return list;

    const BB_Bar &bar = bars.at(i);
    float cp = bars.last().p_open;
    quint8 prec = (bar.p_open < 1.5 ? 3 : 2);
    list[0] = QString::number(bar.p_open, 'f', prec);
    if (i==0) return list;

    const BB_Bar &bar_prev = bars.at(i-1);
    float over = bar_prev.overShadow_p(cp);
    float under = bar_prev.underShadow_p(cp);
    list[1] = QString("%1% / %2%").arg(QString::number(over, 'f', 1)).arg(QString::number(under, 'f', 1));

    if (over >= limit_shadow_size && under >= limit_shadow_size)
    {
        n_ok++;
        testing_result += (2*limit_shadow_size);
        list[2] = QString("OK (%1ps)").arg(n_ok);
        return list;
    }
    if (over < limit_shadow_size && under < limit_shadow_size)
    {
        list[2] = QString("none");
        return list;
    }


    float result = 0;
    if (over < limit_shadow_size) result = bar.p_open - bar_prev.p_open;
    if (under < limit_shadow_size) result = bar_prev.p_open - bar.p_open;
    result = float(100)*result/cp;
    float result2 = result + limit_shadow_size;
    testing_result += result2;

    list[2] = QString::number(result, 'f', 1);
    if (result2 > 0) list[2].append(QString(" (+%1)").arg(QString::number(result2, 'f', 1)));
    else  list[2].append(QString(" (%1)").arg(QString::number(result2, 'f', 1)));

    list[3] = QString("%1%").arg(QString::number(testing_result, 'f', 1));

    return list;
}


