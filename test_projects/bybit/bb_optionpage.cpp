#include "bb_optionpage.h"
#include "apiconfig.h"
#include "bb_apistruct.h"
#include "ltable.h"
#include "ltime.h"
#include "lfile.h"
#include "lstring.h"
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
#include <QDate>


#define TYPE_COL        3
#define STRIKE_COL      4
#define DAY_TO_COL      5
#define MARKET_COL      6
#define DAY_PROFIT_COL  7
#define ASSET_PRICE     3


#define IN_MONEY_COLOR      QString("#FF4500")
#define OPTIONS_INFO_LIMIT  350

// Получение цен опционов
#define API_TICKERS_URI           QString("v5/market/tickers")
/*
Ответ (важные поля) Каждый элемент:
{
  "symbol": "ETH-30JUN23-2000-C",
  "lastPrice": "123.4",
  "markPrice": "120.1",
  "bid1Price": "119.5",
  "ask1Price": "121.0",
  "delta": "...",
  "gamma": "...",
  "vega": "...",
  "theta": "..."
}

Получение конкретного опциона
Если нужен один инструмент нужно в запрос добавить поле: symbol=ETH-30JUN23-2000-C

*/

// получение комбинаций страйков и дат экспираций и общее название инструментов (типа ETH-30JUN23-2000-C)
#define API_OPTIONS_INFO_URI      QString("v5/market/instruments-info")

// получение открытых позиций по опционам
#define API_OPTIONS_POSITIONS_URI      QString("v5/position/list")
/*
    Этот запрос приватный → нужна подпись.  Ответ:
result.list[]
{
  "symbol": "ETH-30JUN23-2000-C",
  "size": "1",
  "avgPrice": "100",
  "markPrice": "120",
  "unrealisedPnl": "20",
  "side": "Buy"
}
*/

// История позиций (закрытые сделки)
#define API_OPTIONS_POS_CLOSED_URI      QString("v5/execution/list")
//#define API_OPTIONS_POS_CLOSED_URI      QString("v5/position/get-closed-positions")

// Активные ордера по опционам
#define API_OPTIONS_ORDERS_URI      QString("v5/order/realtime")


//📜 6. История ордеров GET /v5/order/history

// создать лимитный ордер
#define  API_OPTION_CREATE_ORDER_URI QString("v5/order/create")



//BB_OptionPage
BB_OptionPage::BB_OptionPage(QWidget *parent)
    :BB_BasePage(parent, 20, rtOptions),
      m_table(NULL),
    m_monitTable(NULL),
    m_polledDays(5)
{
    setObjectName("options_page");

    init();


    //loadTickers();
    m_monitTable->searchExec();
    m_reqData->params.insert("category", "option");
    m_reqData->params.insert("baseCoin", "ETH"); // работаем только с опционами по эфиру


    //m_reqData->uri = API_TICKERS_URI;
    //m_reqData->uri = API_OPTIONS_INFO_URI;
    m_reqData->uri = API_OPTIONS_INFO_URI;

    m_reqData->req_type = m_userSign;

    // init popup
    initPopupMenu();

}
void BB_OptionPage::initPopupMenu()
{
    QString path = APIConfig::commonIconsPath(); // icons path

    //prepare menu actions data for assets table
    QList< QPair<QString, QString> > act_list;
    QPair<QString, QString> pair1("Get state", QString("%1/view-refresh.svg").arg(path));
    QPair<QString, QString> pair2("Buy", QString("%1/ball_green.svg").arg(path));
    QPair<QString, QString> pair3("Sell", QString("%1/ball_red.svg").arg(path));
    act_list.append(pair1);
    act_list.append(pair2);
    act_list.append(pair3);

    //init popup menu actions
    m_table->popupMenuActivate(act_list, false);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotUpdateOptionsPrices())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotOptionBuy())); i_menu++;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotOptionSell())); i_menu++;

}
void BB_OptionPage::init()
{
    m_monitTable = new LSearchTableWidgetBox(this);
    m_monitTable->setObjectName("options_table");
    m_monitTable->setTitle("Options");
    m_monitTable->vHeaderHide();
    m_monitTable->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    //headers
    QStringList list;
    list << "Ticker" << "Time (expiration)" << "To expiration, d" << "Type" << "Strike";
    m_monitTable->setHeaderLabels(list);
    h_splitter->addWidget(m_monitTable);


    m_table = new LSearchTableWidgetBox(this);
    m_table->setObjectName("price_table");
    m_table->setTitle("Prices");
    m_table->vHeaderHide();
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    list.clear();
    list << "Ticker" << "Ask" << "Bid" << "ETH" << "Strike" << "Days to" << "Market" << "Profit/d";
    m_table->setHeaderLabels(list);
    h_splitter->addWidget(m_table);
    m_table->removeAllRows();
    m_table->searchExec();

}
void BB_OptionPage::updateDataPage(bool force)
{
    m_monitTable->removeAllRows();
    m_monitTable->resizeByContents();
    m_container.clear();

    if (!force)
    {
        loadContainerByFile();
        updateInfoTable();
        return;
    }

    m_reqData->uri = API_OPTIONS_INFO_URI;
    m_reqData->metod = hrmGet;


    int limit = OPTIONS_INFO_LIMIT;
    if (m_reqData->uri == API_OPTIONS_POS_CLOSED_URI) {setTSNextInterval(); limit=30;}
    sendRequest(limit);
}
int BB_OptionPage::findRecByTicker(const QString &ticker) const
{
    for (int i=0; i<m_container.count(); i++)
    {
        const BB_Option &rec = m_container.at(i);
        if (rec.ticker == ticker) return i;
    }
    return -1;
}
void BB_OptionPage::setTSNextInterval()
{
    qint64 ts_cur = QDateTime::currentMSecsSinceEpoch();
    QDateTime dt_left = QDateTime::currentDateTime().addDays(qint64(-1*m_polledDays));
    qint64 ts_start = dt_left.toMSecsSinceEpoch();

    m_reqData->params.insert("startTime", QString::number(ts_start));
    m_reqData->params.insert("endTime", QString::number(ts_cur));
}
void BB_OptionPage::parseInfoRecord(const QJsonObject &j_el)
{
    if (j_el.isEmpty()) {qWarning()<<QString("BB_OptionPage::fillOrdersTable WARNING j_el is empty "); return;}

    QString symbol = j_el.value("symbol").toString().trimmed();
    if (!symbol.contains("USDT")) return;

    qint64 deliveryTime = j_el.value("deliveryTime").toString().toLong();
//    qint64 launchTime = j_el.value("launchTime").toString().toLong();
    QString type = j_el.value("optionsType").toString().trimmed().toUpper();

    BB_Option rec;
    rec.ticker = symbol;
    rec.expiration = deliveryTime/1000;
    rec.type = type;

    if (rec.daysToExpiration() > 180) return;
    if (rec.daysToExpiration() < 0.5) return;


    // define strike
    QString name = j_el.value("displayName").toString().trimmed();
    name = LString::strTrimRight(name, 2).trimmed();
    int pos = LString::strIndexOfByEnd(name, "-");
    if (pos > 0) rec.strike = LString::strTrimLeft(name, pos+1).toFloat();

    m_container.insert(0, rec);
}
void BB_OptionPage::parsePriceRecord(const QJsonObject &j_el)
{
    if (j_el.isEmpty()) {qWarning()<<QString("BB_OptionPage::parsePriceRecord WARNING j_el is empty "); return;}

    QString symbol = j_el.value("symbol").toString().trimmed();
    int pos = findRecByTicker(symbol);
    if (pos < 0) return;

    float days_to = m_container.at(pos).daysToExpiration();
    //if (days_to > 180) return;

    QTableWidget *t = m_table->table();
    float ask = j_el.value("ask1Price").toString().toFloat();
    float bid = j_el.value("bid1Price").toString().toFloat();
    float eth_p = j_el.value("indexPrice").toString().toFloat();
    float market_p = j_el.value("markPrice").toString().toFloat();

    float strike = m_container.at(pos).strike;
    float d = qAbs(eth_p - strike);
    QString s_strike = QString::number(strike, 'f', 1);
    if (eth_p < strike) s_strike = QString("%1 (+%2)").arg(s_strike).arg(QString::number(d, 'f', 1));
    else s_strike = QString("%1 (-%2)").arg(s_strike).arg(QString::number(d, 'f', 1));

    float day_profit = (market_p - d)/days_to;


    QStringList row_data;
    row_data << symbol << QString::number(ask, 'f', 2) << QString::number(bid, 'f', 2);
    row_data << QString::number(eth_p, 'f', 1) << s_strike << QString::number(days_to, 'f', 1);
    row_data << QString::number(market_p, 'f', 2) << QString::number(day_profit, 'f', 2);
    LTable::addTableRow(t, row_data);

    int last_i = t->rowCount() - 1;
    if (m_container.at(pos).isPut()) LTable::setTableRowColor(t, last_i, "#FFD9DD");
    else LTable::setTableRowColor(t, last_i, "#D9FFDD");

    t->item(last_i, t->columnCount()-1)->setData(Qt::UserRole, m_container.at(pos).expiration);
    t->item(last_i, 0)->setData(Qt::UserRole, m_container.at(pos).type);
    t->item(last_i, STRIKE_COL)->setData(Qt::UserRole, pos);

}
bool BB_OptionPage::hasContainerTicker(const QString &symbol) const
{
    foreach (const BB_Option &rec, m_container)
        if (rec.ticker == symbol) return true;
    return false;
}
void BB_OptionPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    if (req_type != m_userSign) return;
    qDebug()<<QString("BB_OptionPage::slotJsonReply  req_type = %1").arg(req_type);


    const QJsonValue &jv = j_obj.value("result");
    if (jv.isNull()) {emit signalError("BB_OptionPage: result QJsonValue not found"); return;}
    const QJsonValue &j_list = jv.toObject().value("list");
    if (!j_list.isArray()) {emit signalError("BB_OptionPage: j_list not array"); return;}
    const QJsonArray &j_arr = j_list.toArray();
    int n = j_arr.count();

    if (m_reqData->uri == API_OPTIONS_INFO_URI)
    {
        for (int i=0; i<n; i++)
            parseInfoRecord(j_arr.at(i).toObject());

        QString npc = jv.toObject().value("nextPageCursor").toString().trimmed();
        if (!npc.isEmpty())
        {
            m_reqData->params.insert("cursor", npc);
            sendRequest(OPTIONS_INFO_LIMIT);
        }
        else
        {
            updateInfoTable();
            rewriteFile();
        }
    }
    else if (m_reqData->uri == API_TICKERS_URI)
    {
        qDebug()<<QString("m_reqData->uri == API_TICKERS_URI  j_arr.count() %1").arg(j_arr.count());
        for (int i=0; i<n; i++)
            parsePriceRecord(j_arr.at(i).toObject());

        sortPricesTable();
        sortDayStrikes();
        setColorInMoney();
        m_table->searchExec();
    }

}
void BB_OptionPage::getTSNextInterval(qint64 &ts1, qint64 &ts2)
{
    ts1 = ts2 = -1;
    //if (m_polledDays < 0) {qWarning("BB_HistoryPage::getTSNextInterval WARNING m_polledDays < 0"); return;}

    QDate cur_d(QDate::currentDate());

    QDate d1(2026, 3, 22);
    QDate d2 = d1.addDays(m_polledDays);
    ts1 = APIConfig::toTimeStamp(d1.day(), d1.month(), d1.year());
    ts2 = APIConfig::toTimeStamp(d2.day(), d2.month(), d2.year());
}
void BB_OptionPage::rewriteFile()
{
    QStringList fdata;
    foreach (const BB_Option &rec, m_container)
        fdata.append(rec.toFileLine());

    QString fname(QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(BB_PricesContainer::optionFile()));
    QString err = LFile::writeFileSL(fname, fdata);
    if (!err.isEmpty()) signalError(err);
    else signalMsg("File options was rewrited OK!");

}
void BB_OptionPage::loadContainerByFile()
{
    m_container.clear();

    QStringList fdata;
    QString fname(QString("%1%2%3").arg(APIConfig::appDataPath()).arg(QDir::separator()).arg(BB_PricesContainer::optionFile()));
    QString err = LFile::readFileSL(fname, fdata);
    if (!err.isEmpty()) {signalError(err); return;}

    foreach (const QString &fline, fdata)
    {
        if (fline.trimmed().isEmpty()) continue;

        BB_Option rec;
        rec.fromFileLine(fline);
        if (!rec.invalid()) m_container.append(rec);
    }

    emit signalMsg(QString("load options record from file: %1").arg(m_container.count()));
}
void BB_OptionPage::updateInfoTable()
{
    QTableWidget *t = m_monitTable->table();
    for (int i=0; i<m_container.count(); i++)
    {
        const BB_Option &rec = m_container.at(i);

        QStringList row_data;
        row_data << rec.ticker << rec.strExpiration() << QString::number(rec.daysToExpiration(), 'f', 1) <<
                    rec.type  << QString::number(rec.strike) ;

        LTable::addTableRow(t, row_data);

        if (rec.isCall()) t->item(i, TYPE_COL)->setTextColor("#008000");
        if (rec.isPut()) t->item(i, TYPE_COL)->setTextColor("#9B0000");
    }

    m_monitTable->searchExec();
}
void BB_OptionPage::slotUpdateOptionsPrices()
{
    qDebug("BB_OptionPage::slotUpdateOptionsPrices()");
    m_reqData->uri = API_TICKERS_URI;
    m_reqData->metod = hrmGet;

    m_table->removeAllRows();

    sendRequest();
}
void BB_OptionPage::sortPricesTable()
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    if (n_rows < 5) return;

    int row_first = 0;
    int x = -1;
    while (2 > 1)
    {
        float min_days = LTable::minNumericColValue(t, DAY_TO_COL, x, row_first);
        float max_days = LTable::maxNumericColValue(t, DAY_TO_COL, x, row_first);
        qDebug()<<QString("min_days=%1  max_days=%2").arg(min_days).arg(max_days);
        if (min_days == max_days) break;


        QString s_min = QString::number(min_days, 'f', 1);
        int n_finded = 0;
        for (int i=row_first; i<n_rows; i++)
        {
            if (s_min == t->item(i, DAY_TO_COL)->text())
            {
                QString symbol = t->item(i, 0)->text();
                //qDebug()<<QString("finded: i=%1  symbol[%2]  n_finded=%3").arg(i).arg(symbol).arg(n_finded);

                int pos = findRecByTicker(symbol);
                if (i > row_first)
                {
                    int shift = (row_first-i);
                    if (m_container.at(pos).isPut()) shift += n_finded;
                    LTable::shiftTableRow(t, i, shift);
                }
                n_finded++;
            }
        }

        if (n_finded == 0) {qDebug("n_finded == 0, breaked!"); break;}
        row_first += n_finded;
    }

}
void BB_OptionPage::sortDayStrikes()
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    int n_col = t->columnCount();
    if (n_rows < 5) return;

    qDebug()<<QString("start sortDayStrikes ...............");
    int start_row = 0;
    int start_row_next = 0;
    int n_strikes = -1;
    while (2>1)
    {
        if (start_row_next < 0) break;

        start_row = start_row_next;
        start_row_next = -1;
        //qDebug()<<QString("start_row=%1").arg(start_row);

        n_strikes = 1;
        QPair<QString, qint64> pair; // opt type / expiration
        pair.first = t->item(start_row, 0)->data(Qt::UserRole).toString();
        pair.second = t->item(start_row, n_col-1)->data(Qt::UserRole).toLongLong();
        if ((start_row+1) >= n_rows) break;


        for (int i=(start_row+1); i<n_rows; i++)
        {
            if (t->item(i, 0)->data(Qt::UserRole).toString() != pair.first ||
                    t->item(i, n_col-1)->data(Qt::UserRole).toLongLong() != pair.second)
            {
                start_row_next = i;
                break;
            }
            else n_strikes++;
        }

        //qDebug()<<QString("n_strikes=%1").arg(n_strikes);
        if (n_strikes < 2) continue;


        // sort by max/min
        quint8 n_it = 0;
        if (pair.first == "CALL")
        {
            while (true)
            {
                float max = 0;
                float max_i = -1;
                for (int i=(start_row+n_it); i<(start_row+n_strikes); i++)
                {
                    int pos = t->item(i, STRIKE_COL)->data(Qt::UserRole).toInt();
                    if (m_container.at(pos).strike > max) {max = m_container.at(pos).strike; max_i = i;}
                }
                //qDebug()<<QString("n_it=%1  max=%2  max_i=%3").arg(n_it).arg(max).arg(max_i);

                if (max_i > n_it)
                {
                    int shift = int(n_it) - (max_i-start_row);
                    //qDebug()<<QString("shift=%1").arg(shift);

                    LTable::shiftTableRow(t, max_i, shift);
                }

                n_it++;
                if ((n_it+1) >= n_strikes) break;
            }
        }
        else
        {
            while (true)
            {
                float min = 1000000;
                float min_i = -1;
                for (int i=(start_row+n_it); i<(start_row+n_strikes); i++)
                {
                    int pos = t->item(i, STRIKE_COL)->data(Qt::UserRole).toInt();
                    if (m_container.at(pos).strike < min) {min = m_container.at(pos).strike; min_i = i;}
                }
                //qDebug()<<QString("n_it=%1  min=%2  min_i=%3").arg(n_it).arg(min).arg(min_i);

                if (min_i > n_it)
                {
                    int shift = int(n_it) - (min_i-start_row);
                    //qDebug()<<QString("shift=%1").arg(shift);

                    LTable::shiftTableRow(t, min_i, shift);
                }

                n_it++;
                if ((n_it+1) >= n_strikes) break;
            }
        }



        if (start_row_next < 0) break;

        //if (start_row > 3) break;
    }
}
void BB_OptionPage::setColorInMoney()
{
    QTableWidget *t = m_table->table();
    int n_rows = t->rowCount();
    float eth_p = t->item(0, 3)->text().toFloat();
    for (int i=0; i<n_rows; i++)
    {
        int pos = t->item(i, STRIKE_COL)->data(Qt::UserRole).toInt();
        if (m_container.at(pos).isCall())
        {
            if (eth_p > m_container.at(pos).strike)
                t->item(i, MARKET_COL)->setTextColor(IN_MONEY_COLOR);
        }
        else
        {
            if (eth_p < m_container.at(pos).strike)
                t->item(i, MARKET_COL)->setTextColor(IN_MONEY_COLOR);
        }
    }
}
void BB_OptionPage::slotOptionBuy()
{
    bool ok = false;
    TradeOperationData data(totBuyLimit);
    prepareTradeOperationData(data, ok);
    if (!ok) return;


    APITradeDialog d(data, this);
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
void BB_OptionPage::slotOptionSell()
{
    bool ok = false;
    TradeOperationData data(totSellLimit);
    prepareTradeOperationData(data, ok);
    if (!ok) return;


    APITradeDialog d(data, this);
    d.exec();
    if (d.isApply())
    {
        qDebug("d.isApply(SHORT)");
        sendTradeReq(data);
    }
    else
    {
        qDebug("canceled operation [SHORT]");
        emit signalError("Operation canceled! [SHORT]");
    }
}
void BB_OptionPage::prepareTradeOperationData(TradeOperationData &data, bool &ok)
{
    ok = false;

    QTableWidget *t = m_table->table();
    QList<int> sel_rows = LTable::selectedRows(t);
    if (sel_rows.isEmpty() || sel_rows.count() > 1)
    {
        emit signalError("can't buy, selection record is invalid");
        return;
    }

    QString ticker = t->item(sel_rows.first(), 0)->text().trimmed();
    qDebug()<<QString("try buy option [%1]").arg(ticker);

    int pos = findRecByTicker(ticker);
    if (pos < 0) return;

    data.ticker = m_container.at(pos).ticker;
    data.strike = m_container.at(pos).strike;
    data.type = m_container.at(pos).type;
    data.expirate = m_container.at(pos).expiration;
    data.award = t->item(sel_rows.first(), MARKET_COL)->text().toFloat();
    data.asset_price = t->item(sel_rows.first(), ASSET_PRICE)->text().toFloat();

    ok = true;
}
void BB_OptionPage::sendTradeReq(const TradeOperationData &data)
{
    emit signalMsg("Try send request on trade command ...........");

    m_reqData->uri = API_OPTION_CREATE_ORDER_URI;
    m_reqData->metod = hrmPost;


    m_reqData->params.insert("symbol", data.ticker);
    m_reqData->params.insert("side", (data.isBuy() ? "Buy" : "Sell"));
    m_reqData->params.insert("orderType", "Limit");
    m_reqData->params.insert("timeInForce", "GTC");
    m_reqData->params.insert("qty", QString::number(data.lot_size));
    m_reqData->params.insert("price", QString::number(data.award, 'f', 2));

    // m_reqData->params.insert("qty", "1");

    //extra params
    //m_reqData->params.insert("reduceOnly", "false");
    QString custom_id = QString("my_custom_id_%1").arg(data.isBuy()?"long":"short");
    m_reqData->params.insert("orderLinkId", custom_id);

    emit signalMsg(m_reqData->toStr());

    m_reqData->outParams();


    sendRequest();
}






