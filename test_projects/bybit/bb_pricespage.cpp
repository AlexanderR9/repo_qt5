#include "bb_pricespage.h"
#include "apiconfig.h"
#include "bb_apistruct.h"
#include "ltable.h"
#include "ltime.h"
#include "lfile.h"
#include "apitradedialog.h"
#include "lhttp_types.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QTableWidget>
#include <QSplitter>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QDateTime>


#define API_TICKERS_URI           QString("v5/market/tickers")
#define PRICES_COL                  2
#define TIME_COL                    1
#define DEVIATION_COUNT             7
#define DEVIATION_PRECISION         1
#define TICKER_COL                  0

// создать лимитный ордер
#define  API_OPTION_CREATE_ORDER_URI QString("v5/order/create")

//BB_PricesPage
BB_PricesPage::BB_PricesPage(QWidget *parent)
    :BB_BasePage(parent, 20, rtPrices),
    m_monitTable(NULL)
{
    setObjectName("prices_page");

    init();
    loadTickers();
    m_monitTable->searchExec();

    m_reqData->params.insert("category", "linear");
    m_reqData->uri = API_TICKERS_URI;
    m_reqData->req_type = m_userSign;

    // init popup
    initPopupMenu();

}
void BB_PricesPage::initPopupMenu()
{
    QString path = APIConfig::commonIconsPath(); // icons path

    //prepare menu actions data for assets table
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Buy", QString("%1/ball_green.svg").arg(path));
    QPair<QString, QString> pair2("Sell", QString("%1/ball_red.svg").arg(path));
    act_list.append(pair1);
    act_list.append(pair2);

    //init popup menu actions
    m_monitTable->popupMenuActivate(act_list);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_monitTable->connectSlotToPopupAction(i_menu, this, SLOT(slotOrderBuy())); i_menu++;
    m_monitTable->connectSlotToPopupAction(i_menu, this, SLOT(slotOrderSell())); i_menu++;

}
void BB_PricesPage::slotOrderBuy()
{
    qDebug("BB_PricesPage::slotOrderBuy()");
    bool ok = false;
    TradeOperationData data(totBuyLimit);
    prepareTradeOperationData(data, ok);
    if (!ok) return;


    APILinearTradeDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        qDebug("d.isApply(LONG)");
        sendTradeReq(data);
    }
    else
    {
        qDebug("canceled operation [LONG]");
        emit signalError("Operation canceled! [LONG]");
    }
}
void BB_PricesPage::sendTradeReq(const TradeOperationData &data)
{
    emit signalMsg("Try send request on trade command ...........");

    resetReqParams(hrmPost);
    m_reqData->uri = API_OPTION_CREATE_ORDER_URI;

    m_reqData->params.insert("symbol", data.ticker+QString("USDT"));
    m_reqData->params.insert("side", (data.isBuy() ? "Buy" : "Sell"));
    m_reqData->params.insert("orderType", "Limit");
    m_reqData->params.insert("timeInForce", "GTC");
    m_reqData->params.insert("qty", QString::number(data.lot_size));
    m_reqData->params.insert("price", QString::number(data.award, 'f', 2));
    //m_reqData->params.insert("positionIdx", QString::number(0));
    int positionIdx = (data.isBuy() ? 1 : 2);
    m_reqData->params.insert("positionIdx", QString::number(positionIdx));

    //extra params
    //m_reqData->params.insert("reduceOnly", "false");
    //QString custom_id = QString("my_custom_id_%1").arg(data.isBuy()?"long":"short");
    if (!data.custom_id.isEmpty())
        m_reqData->params.insert("orderLinkId", data.custom_id);

    emit signalMsg(m_reqData->toStr());

    m_reqData->outParams();


    sendRequest();
}
void BB_PricesPage::resetReqParams(int http_m)
{
    m_reqData->params.clear();
    m_reqData->params.insert("category", "linear");
    m_reqData->params.insert("baseCoin", "USDT");

    m_reqData->metod = http_m;
}


void BB_PricesPage::slotOrderSell()
{

}
void BB_PricesPage::init()
{
    m_monitTable = new LSearchTableWidgetBox(this);
    m_monitTable->setObjectName("prices_table");
    m_monitTable->setTitle("Prices");
    m_monitTable->vHeaderHide();
    m_monitTable->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    //headers
    QStringList list;
    list << "Ticker" << "Update time" << "Price";
    list << "1d, %" << "3d, %" << "1W, %" << "1M, %" << "3M, %" << "6M, %" << "12M, %";
    m_monitTable->setHeaderLabels(list);

    //sorting
    m_monitTable->addSortingData(0, LSearchTableWidgetBox::sdtString);
    for (int j=2; j<m_monitTable->table()->columnCount(); j++)
        m_monitTable->addSortingData(j, LSearchTableWidgetBox::sdtNumeric);
    m_monitTable->sortingOn();


    h_splitter->addWidget(m_monitTable);
}
void BB_PricesPage::updateDataPage(bool force)
{
    if (!updateTimeOver(force)) return;

    resetReqParams(hrmGet);
    m_reqData->uri = API_TICKERS_URI;

    sendRequest();

}
void BB_PricesPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != m_userSign) return;
    if (m_reqData->uri != API_TICKERS_URI) return;
    if (api_config.favor_tickers.isEmpty()) return;

    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_PricesPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (!j_list.isArray()) {emit signalError("BB_PricesPage: j_list not array"); return;}

    newPricesReceived(j_list.toArray());
    updateTableDeviations();

    m_monitTable->searchExec();
}
void BB_PricesPage::newPricesReceived(const QJsonArray &j_arr)
{
    bool ok = false;
    QMap<QString, float> map;
    for (int i=0; i<j_arr.count(); i++)
    {
        QJsonObject j_el = j_arr.at(i).toObject();
        if (j_el.isEmpty()) {qWarning()<<QString("BB_SpotHistoryPage::fillOrdersTable WARNING j_el is empty (index=%1)").arg(i); break;}

        QString symbol = j_el.value("symbol").toString().trimmed();
        symbol.remove("USDT");
        if (!api_config.favor_tickers.contains(symbol.trimmed())) continue;

        float prs = j_el.value("lastPrice").toString().toFloat(&ok);
        if (ok && prs > 0) map.insert(symbol, prs);
        else qWarning()<<QString("BB_PricesPage - WARNING invalid convert to float [%1]").arg(j_el.value("lastPrice").toString());
    }
    updateContainer(map);
    updateTablePrices(map);
}
void BB_PricesPage::updateTableDeviations()
{
    bool ok = false;
    QTableWidget *t = m_monitTable->table();
    qint64 t_cur = QDateTime::currentDateTime().toSecsSinceEpoch();
    //qDebug()<<QString("updateTableDeviations  t_cur=%1").arg(t_cur);


    for (int i=0; i<t->rowCount(); i++)
    {
        int j = 0;
        QStringList deviations;
        for (j=0; j<DEVIATION_COUNT; j++) deviations.append("?");

        QString ticker = t->item(i, 0)->text();
        float p = t->item(i, PRICES_COL)->text().toFloat(&ok);
        if (!ok || p <= 0) {updateTableDeviationsRow(i, deviations); continue;}

        j = 0;

        //calc for 1 day
        float d = m_container.getDeviation(ticker, p, t_cur, quint16(dd1D));
        if (d > -9998) deviations[j] = QString::number(d, 'f', DEVIATION_PRECISION);
        j++;

        //calc for 3 day
        d = m_container.getDeviation(ticker, p, t_cur, quint16(dd3D));
        if (d > -9998) deviations[j] = QString::number(d, 'f', DEVIATION_PRECISION);
        j++;

        //calc for 1 week
        d = m_container.getDeviation(ticker, p, t_cur, quint16(dd1W));
        if (d > -9998) deviations[j] = QString::number(d, 'f', DEVIATION_PRECISION);
        j++;

        //calc for 1 month
        d = m_container.getDeviation(ticker, p, t_cur, quint16(dd1M));
        if (d > -9998) deviations[j] = QString::number(d, 'f', DEVIATION_PRECISION);
        j++;

        //calc for 3 month
        d = m_container.getDeviation(ticker, p, t_cur, quint16(dd3M));
        if (d > -9998) deviations[j] = QString::number(d, 'f', DEVIATION_PRECISION);
        j++;

        //calc for 6 month
        d = m_container.getDeviation(ticker, p, t_cur, quint16(dd6M));
        if (d > -9998) deviations[j] = QString::number(d, 'f', DEVIATION_PRECISION);
        j++;

        //calc for 12 month
        d = m_container.getDeviation(ticker, p, t_cur, quint16(dd12M));
        if (d > -9998) deviations[j] = QString::number(d, 'f', DEVIATION_PRECISION);
        j++;

        updateTableDeviationsRow(i, deviations);
    }
}
void BB_PricesPage::updateTableDeviationsRow(int row, const QStringList &list)
{
    bool ok = false;
    QTableWidget *t = m_monitTable->table();
    int f_col = t->columnCount() - DEVIATION_COUNT;
    for (int j=0; j<DEVIATION_COUNT; j++)
    {
        t->item(row, j+f_col)->setText(list.at(j));
        if (list.at(j).contains("?"))
        {
            t->item(row, j+f_col)->setTextColor(Qt::lightGray);
            continue;
        }

//        t->item(row, j+f_col)->setText(QString("%1 %").arg(list.at(j)));
        float dp = list.at(j).toFloat(&ok);
        if (!ok) t->item(row, j+f_col)->setTextColor("#DEB887");
        else if (dp > 5) t->item(row, j+f_col)->setTextColor("#0000CD");
        else if (dp < -5) t->item(row, j+f_col)->setTextColor(Qt::darkRed);
        else t->item(row, j+f_col)->setTextColor(Qt::black);
    }
}
void BB_PricesPage::updateTablePrices(const QMap<QString, float> &map)
{
    QTableWidget *t = m_monitTable->table();
    for (int i=0; i<t->rowCount(); i++)
    {
        QString ticker = t->item(i, 0)->text();
        if (map.contains(ticker))
        {
            float p = map.value(ticker);
            quint8 prec = 3;
            if (p > 2) prec = 2;
            if (p > 100) prec = 1;
            if (ticker.trimmed() == "USDC") prec = 4;

            t->item(i, PRICES_COL)->setText(QString::number(p, 'f', prec));
            t->item(i, TIME_COL)->setText(LTime::strCurrentDateTime());
            t->item(i, PRICES_COL)->setTextColor("#008B8B");
        }
    }
}
void BB_PricesPage::loadTickers()
{
    if (api_config.favor_tickers.isEmpty()) return;

    QTableWidget *t = m_monitTable->table();
    foreach (const QString &v, api_config.favor_tickers)
    {
        QStringList row_data;
        row_data.append(v);
        for (int j=1; j<t->columnCount(); j++) row_data << "-";
        LTable::addTableRow(t, row_data);
    }
}
void BB_PricesPage::loadContainer()
{
    //qDebug("BB_PricesPage::loadContainer()");
    m_container.reset();

    QString fname(QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(BB_PricesContainer::priceFile()));
    if (LFile::fileExists(fname))
    {
        QString fdata;
        QString err = LFile::readFileStr(fname, fdata);
        if (err.isEmpty())
        {
            m_container.reloadPrices(fdata);
            emit signalMsg(QString("Loaded %1 price_points").arg(m_container.data.count()));
        }
        else emit signalError(err);
    }
    else emit signalError(QString("spot file [%1] not found").arg(fname));
}
void BB_PricesPage::load(QSettings &settings)
{
    BB_BasePage::load(settings);
    loadContainer();
}
void BB_PricesPage::updateContainer(const QMap<QString, float> &map)
{
    quint32 n_old = m_container.size();
    qint64 t_cur = QDateTime::currentDateTime().toSecsSinceEpoch();

    QStringList tickers(map.keys());
    foreach (const QString &v, tickers)
    {
        m_container.addPricePoint(v, map.value(v), t_cur);
    }
    emit signalMsg(QString("updateContainer: n_old=%1  n_cur=%2").arg(n_old).arg(m_container.size()));

    if (n_old != m_container.size()) //need update file
    {
        rewriteFile();
    }
}
void BB_PricesPage::rewriteFile()
{
    QString fdata;
    m_container.toFileData(fdata);

    QString fname(QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(BB_PricesContainer::priceFile()));
    QString err = LFile::writeFile(fname, fdata);
    if (!err.isEmpty()) signalError(err);
    else signalMsg("File prices was rewrited OK!");
}
void BB_PricesPage::prepareTradeOperationData(TradeOperationData &data, bool &ok)
{
    ok = false;

    QTableWidget *t = m_monitTable->table();
    QList<int> sel_rows = LTable::selectedRows(t);
    if (sel_rows.isEmpty() || sel_rows.count() > 1)
    {
        emit signalError("can't buy, selection record is invalid");
        return;
    }

    data.ticker = t->item(sel_rows.first(), TICKER_COL)->text().trimmed();
    qDebug()<<QString("ticker [%1]").arg(data.ticker);

    data.asset_price = t->item(sel_rows.first(), PRICES_COL)->text().toFloat();


    /*
    int pos = findRecByTicker(ticker);
    if (pos < 0) return;

    data.ticker = m_container.at(pos).ticker;
    data.strike = m_container.at(pos).strike;
    data.type = m_container.at(pos).type;
    data.expirate = m_container.at(pos).expiration;
    data.award = t->item(sel_rows.first(), MARKET_COL)->text().toFloat();
    data.asset_price = t->item(sel_rows.first(), ASSET_PRICE)->text().toFloat();
    */

    ok = true;
}

