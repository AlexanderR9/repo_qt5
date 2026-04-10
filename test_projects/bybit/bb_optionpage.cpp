#include "bb_optionpage.h"
#include "apiconfig.h"
#include "bb_apistruct.h"
#include "ltable.h"
#include "ltime.h"
#include "lfile.h"
#include "lstring.h"

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
    act_list.append(pair1);

    //init popup menu actions
    m_table->popupMenuActivate(act_list, false);

    //connect OWN slots to popup actions
    int i_menu = 0;
    m_table->connectSlotToPopupAction(i_menu, this, SLOT(slotUpdateOptionsPrices())); i_menu++;
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
    list << "Ticker" << "Time (expiration)" << "To expiration, d" << "Strike" << "Type";
    m_monitTable->setHeaderLabels(list);
    h_splitter->addWidget(m_monitTable);


    m_table = new LSearchTableWidgetBox(this);
    m_table->setObjectName("price_table");
    m_table->setTitle("Prices");
    m_table->vHeaderHide();
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    list.clear();
    list << "Ticker" << "Ask" << "Bid" << "ETH" << "Days to" << "Strike";
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

    int limit = -1;
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
    if (days_to > 180) return;

    //if (!hasContainerTicker(symbol)) return;


    QTableWidget *t = m_table->table();
    float ask = j_el.value("ask1Price").toString().toFloat();
    float bid = j_el.value("bid1Price").toString().toFloat();
    float eth_p = j_el.value("indexPrice").toString().toFloat();

    float strike = m_container.at(pos).strike;
    float d = qAbs(eth_p - strike);
    QString s_strike = QString::number(strike, 'f', 1);
    if (eth_p < strike) s_strike = QString("%1 (+%2)").arg(s_strike).arg(QString::number(d, 'f', 1));
    else s_strike = QString("%1 (-%2)").arg(s_strike).arg(QString::number(d, 'f', 1));



    QStringList row_data;
    row_data << symbol << QString::number(ask, 'f', 2) << QString::number(bid, 'f', 2);
    row_data << QString::number(eth_p, 'f', 1) << QString::number(days_to, 'f', 1) << s_strike;
    LTable::addTableRow(t, row_data);

    int last_i = t->rowCount() - 1;
    if (m_container.at(pos).isPut()) LTable::setTableRowColor(t, last_i, "#FFD9DD");
    else LTable::setTableRowColor(t, last_i, "#D9FFDD");

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

        updateInfoTable();
        rewriteFile();
    }
    else if (m_reqData->uri == API_TICKERS_URI)
    {
        qDebug()<<QString("m_reqData->uri == API_TICKERS_URI  j_arr.count() %1").arg(j_arr.count());
        for (int i=0; i<n; i++)
            parsePriceRecord(j_arr.at(i).toObject());

        sortPricesTable();
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

    /*
    if (m_polledDays > 0) d1 = m_startDate.addDays(m_polledDays);
    ts1 = APIConfig::toTimeStamp(d1.day(), d1.month(), d1.year());
    m_polledDays += daysSeparator();

    QDate d2 = d1.addDays(daysSeparator());
    if (needPollDays() < daysSeparator()) {d2 = d1.addDays(needPollDays()); m_polledDays = needPollDays();}

    qDebug()<<QString("BB_SpotHistoryPage::getTSNextInterval  %1 / %2").arg(d1.toString(APIConfig::userDateMask())).
              arg((d2 >= cur_d) ? "-1" : d2.toString(APIConfig::userDateMask()));

    if (d2 >= cur_d) m_polledDays = -1;
    else ts2 = APIConfig::toTimeStamp(d2.day(), d2.month(), d2.year());
    */
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
        float min_days = LTable::minNumericColValue(t, 4, x, row_first);
        float max_days = LTable::maxNumericColValue(t, 4, x, row_first);
        qDebug()<<QString("min_days=%1  max_days=%2").arg(min_days).arg(max_days);
        if (min_days == max_days) break;


        QString s_min = QString::number(min_days);
        int n_finded = 0;
        for (int i=row_first; i<n_rows; i++)
        {
            if (s_min == t->item(i, 4)->text())
            {
                QString symbol = t->item(i, 0)->text();
                qDebug()<<QString("finded: i=%1  symbol[%2]  n_finded=%3").arg(i).arg(symbol).arg(n_finded);

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
        if (n_finded == 0) break;
        row_first += n_finded;


        //break;
    }

}

