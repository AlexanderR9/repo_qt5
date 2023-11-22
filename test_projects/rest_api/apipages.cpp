#include "apipages.h"
#include "lfile.h"
#include "ltable.h"
#include "instrument.h"
#include "apicommonsettings.h"
#include "bagstate.h"


#include <QSplitter>
#include <QLabel>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QGridLayout>
#include <QLineEdit>
#include <QSpacerItem>
#include <QTableWidget>
#include <QDir>
#include <QTimer>

#define COUNTRY_COL         2
#define CURRENCY_COL        3
#define NOMINAL_COL         4
#define FINISH_DATE_COL     5
#define COUP_YEAR_COL       6
#define RISK_COL            7

#define PROFIT_COL          5
#define TO_COMPLETE_COL     7

#define PROFITABILITY_LIMIT         1.4
#define PROFITABILITY_LIMIT2        1.15
#define COUNTRY_RU                  QString("Российская Федерация")
#define COUNTRY_US                  QString("Соединенные Штаты")


//APIBondsPage
APIBondsPage::APIBondsPage(QWidget *parent)
    :APITablePageBase(parent),
    m_countryFilterControl(NULL),
    m_riskFilterControl(NULL)

{
    setObjectName("api_bonds_page");
    m_userSign = aptBond;

    QStringList headers;
    headers << "Company" << "Tiker" << "Country" << "Currency" << "Nominal" << "Finish date" << "Coupons" << "Risk" << "Price";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Bonds list");

    initFilterBox();
}
void APIBondsPage::resetPage()
{
    APITablePageBase::resetPage();
    m_countryFilterControl->setCurrentIndex(0);
    m_riskFilterControl->setCurrentIndex(0);
    m_finishDateControl->setCurrentIndex(0);
}
void APIBondsPage::initFilterBox()
{
    if (m_filterBox->layout()) {delete m_filterBox->layout();}
    QGridLayout *g_lay = new QGridLayout(m_filterBox);
    g_lay->addWidget(new QLabel("By country"), 0, 0);
    g_lay->addWidget(new QLabel("By risk"), 1, 0);
    g_lay->addWidget(new QLabel("By finish date"), 2, 0);

    m_countryFilterControl = new QComboBox(this);
    m_countryFilterControl->setObjectName("country_control");
    m_countryFilterControl->addItem("All");
    m_countryFilterControl->addItem("Only Rus");
    m_countryFilterControl->addItem("Exept Rus");
    m_countryFilterControl->addItem("Only USA");
    m_countryFilterControl->addItem("Exept USA");
    g_lay->addWidget(m_countryFilterControl, 0, 1);

    m_riskFilterControl = new QComboBox(this);
    m_riskFilterControl->setObjectName("risk_control");
    m_riskFilterControl->addItem("All");
    m_riskFilterControl->addItem("High");
    m_riskFilterControl->addItem("Low");
    m_riskFilterControl->addItem("Medium");
    g_lay->addWidget(m_riskFilterControl, 1, 1);

    m_finishDateControl = new QComboBox(this);
    m_finishDateControl->setObjectName("date_control");
    m_finishDateControl->addItem("none");
    m_finishDateControl->addItem("1 month");
    m_finishDateControl->addItem("2 month");
    m_finishDateControl->addItem("3 month");
    m_finishDateControl->addItem("6 month");
    m_finishDateControl->addItem("1 year");
    m_finishDateControl->addItem("2 year");
    m_finishDateControl->addItem("3 year");
    g_lay->addWidget(m_finishDateControl, 2, 1);


    QSpacerItem *spcr = new QSpacerItem(10, 10, QSizePolicy::Maximum);
    g_lay->addItem(spcr, 3, 0, 2, 2);

    connect(m_countryFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter(QString)));
    connect(m_riskFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter(QString)));
    connect(m_finishDateControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter(QString)));
}
void APIBondsPage::slotFilter(QString f_value)
{
    //qDebug()<<QString("slotFilter,  f_value=%1").arg(f_value);
    if (!sender()) return;
    f_value = f_value.trimmed().toLower();

    if (sender()->objectName().contains("country"))
        countryFilter(f_value);
    else if (sender()->objectName().contains("risk"))
        riskFilter(f_value);
    else if (sender()->objectName().contains("date"))
        dateFilter(f_value);

    m_tableBox->searchExec();


    //send visible figi list to coupon page
    int n = m_tableBox->table()->rowCount();
    if (n <= 0) return;

    bool ok;
    QStringList figi_list;
    for (int i=0; i<n; i++)
    {
        quint16 rec_number = m_tableBox->table()->item(i, 0)->data(Qt::UserRole).toUInt(&ok);
        if (ok) figi_list << figiByRecNumber(rec_number);
    }
    emit signalFilter(figi_list);
}
void APIBondsPage::dateFilter(const QString &f_value)
{
    if (f_value.contains("none")) return;

    QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    if (n == 0) return;

    QDate limit_date(QDate::currentDate());
    quint16 n_days = f_value.left(1).toUInt();
    if (f_value.contains("year")) n_days *= 365;
    else n_days *= 30;
    limit_date = limit_date.addDays(n_days);

    for (int i=n-1; i>=0; i--)
    {
        QString s_date = t->item(i, FINISH_DATE_COL)->text().trimmed().toLower();
        QDate fd = QDate::fromString(s_date, InstrumentBase::userDateMask());
        if (fd > limit_date) t->removeRow(i);
    }
}
void APIBondsPage::riskFilter(const QString &f_value)
{
    if (f_value.contains("all")) return;

    QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    if (n == 0) return;

    for (int i=n-1; i>=0; i--)
    {
        QString tv = t->item(i, RISK_COL)->text().trimmed().toLower();
        if (f_value.contains("high") && !tv.contains("high")) t->removeRow(i);
        else if (f_value.contains("medium") && !tv.contains("mid")) t->removeRow(i);
        else if (f_value.contains("low") && !tv.contains("low")) t->removeRow(i);
    }
}
void APIBondsPage::loadData()
{
    m_data.clear();

    emit signalMsg(QString("OPEN FILE: %1").arg(APIBondsPage::dataFile()));
    QStringList list;
    QString err = LFile::readFileSL(APIBondsPage::dataFile(), list);
    if (!err.isEmpty()) {emit signalError(err); return;}

    int n_invalid = 0;
    foreach (const QString &v, list)
    {
        if (v.trimmed().isEmpty()) continue;
        if (!v.contains(".") || !v.contains("/")) continue;
        BondDesc rec;
        rec.fromFileLine(v);
        if (!rec.invalid())
        {
            bool filter_ok = true;
            if (api_commonSettings.g_filter.bond.country == "rus" && rec.country != COUNTRY_RU) filter_ok = false;
            if (api_commonSettings.g_filter.bond.country == "usa" && rec.country != COUNTRY_US) filter_ok = false;
            if (filter_ok) m_data.append(rec);
        }
        else {qWarning() << QString("INVALID BOND LINE: [%1]").arg(v); n_invalid++;}
    }

    emit signalMsg(QString("found invalid lines: %1/%2").arg(n_invalid).arg(list.count()));
    sortByDate();
    reloadTableByData();
}
void APIBondsPage::reloadTableByData()
{
    resetPage();
    if (m_data.isEmpty())
    {
        emit signalMsg(QString("Bonds data is empty!"));
        return;
    }

    foreach (const BondDesc &rec, m_data)
    {
        LTable::addTableRow(m_tableBox->table(), rec.toTableRowData());
        int l_row = m_tableBox->table()->rowCount() - 1;
        m_tableBox->table()->item(l_row, 0)->setData(Qt::UserRole, rec.number);
        if (!rec.api_trade) LTable::setTableRowColor(m_tableBox->table(), l_row, Qt::lightGray);
        if (rec.coupons_year > 4) m_tableBox->table()->item(l_row, COUP_YEAR_COL)->setTextColor(Qt::blue);
        if (rec.risk.toLower().contains("high")) m_tableBox->table()->item(l_row, RISK_COL)->setTextColor(Qt::red);
        if (QDate::currentDate() >= rec.finish_date) LTable::setTableRowColor(m_tableBox->table(), l_row, "#FFF8DC");
        if (rec.currency.toLower() != "rub") m_tableBox->table()->item(l_row, CURRENCY_COL)->setTextColor("#BC8F8F");
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#E6E6FA", "#800000");
    emit signalMsg(QString("loaded validity records: %1 \n").arg(m_data.count()));
    m_tableBox->searchExec();
}
void APIBondsPage::sortByDate()
{
    if (m_data.count() < 3) return;


    int n = m_data.count();
    while (1 == 1)
    {
        bool has_replace = false;
        for (int i=0; i<n-1; i++)
        {
            const QDate &d = m_data.at(i).finish_date;
            const QDate &d_next = m_data.at(i+1).finish_date;
            if (!d.isValid() && d_next.isValid())
            {
                BondDesc rec = m_data.takeAt(i);
                m_data.insert(i+1, rec);
                has_replace = true;
            }
            else if (d.isValid() && d_next.isValid())
            {
                if (d_next < d)
                {
                    BondDesc rec = m_data.takeAt(i);
                    m_data.insert(i+1, rec);
                    has_replace = true;
                }
            }
        }
        if (!has_replace) break;
    }
}
QString APIBondsPage::dataFile()
{
    return QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(QString("bonds.txt"));
}
void APIBondsPage::slotGetPaperInfo(QStringList &info)
{
    if (info.isEmpty() || m_data.isEmpty()) return;
    if (info.count() < 2 || info.first() != "bond") return;

    QString p_uid = info.at(1).trimmed();
    foreach (const BondDesc &rec, m_data)
    {
        if (rec.uid == p_uid)
        {
            info << rec.name << rec.isin << rec.finish_date.toString(InstrumentBase::userDateMask());
            break;
        }
    }
}
void APIBondsPage::slotGetTickerByFigi(const QString &figi, QString &ticker)
{
    foreach (const BondDesc &rec, m_data)
        if (rec.figi == figi || rec.uid == figi)  {ticker = rec.isin; break;}
}
void APIBondsPage::slotGetPaperTypeByUID(const QString &uid, QString &type)
{
    if (!type.isEmpty()) return;
    foreach (const BondDesc &rec, m_data)
        if (rec.uid == uid)  {type = "bond"; break;}
}
void APIBondsPage::slotGetPaperInfoByFigi(const QString &figi, QPair<QString, QString> &pair)
{
    foreach (const BondDesc &rec, m_data)
    {
        if (rec.figi == figi || rec.uid == figi)
        {
            pair.first = rec.name;
            pair.second = rec.currency;
            break;
        }
    }
}
void APIBondsPage::setSelectedUID(QString &uid, quint16 rec_number)
{
    bool need_figi = (uid == "figi");
    uid.clear();
    foreach (const BondDesc &rec, m_data)
        if (rec.number == rec_number) {uid = (need_figi ? rec.figi : rec.uid); break;}
}
void APIBondsPage::setCycleItem(QString &cycle_item, quint16 rec_number)
{
    foreach (const BondDesc &rec, m_data)
    {
        if (rec.number == rec_number)
        {
            if (rec.finish_date > QDate::currentDate())
                cycle_item = QString("bond : %1 : %2").arg(rec.figi).arg(rec.uid);
            break;
        }
    }
}
QString APIBondsPage::figiByRecNumber(quint16 rec_number) const
{
    foreach (const BondDesc &rec, m_data)
        if (rec.number == rec_number) return rec.figi;
    return QString();
}
void APIBondsPage::calcProfitability(const QString &figi, float price)
{
    if (price <= 0) return;

    foreach (const BondDesc &rec, m_data)
    {
        if (rec.figi == figi)
        {
            emit signalNeedCalcProfitability(rec, price);
            break;
        }
    }
}


//APIStocksPage
APIStocksPage::APIStocksPage(QWidget *parent)
    :APITablePageBase(parent)
{
    setObjectName("api_stocks_page");
    m_userSign = aptStock;


    QStringList headers;
    headers << "Company" << "Tiker" << "Country" << "Currency" << "Sector" << "Price";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Stocks list");

    initFilterBox();
}
void APIStocksPage::initFilterBox()
{
    if (m_filterBox->layout()) {delete m_filterBox->layout();}
    QGridLayout *g_lay = new QGridLayout(m_filterBox);
    g_lay->addWidget(new QLabel("By country"), 0, 0);
    g_lay->addWidget(new QLabel("By currency"), 1, 0);

    m_countryFilterControl = new QComboBox(this);
    m_countryFilterControl->setObjectName("country_control");
    m_countryFilterControl->addItem("All");
    m_countryFilterControl->addItem("Only Rus");
    m_countryFilterControl->addItem("Exept Rus");
    m_countryFilterControl->addItem("Only USA");
    m_countryFilterControl->addItem("Exept USA");
    g_lay->addWidget(m_countryFilterControl, 0, 1);

    m_currencyFilterControl = new QComboBox(this);
    m_currencyFilterControl->setObjectName("currency_control");
    m_currencyFilterControl->addItem("All");
    m_currencyFilterControl->addItem("RUB");
    m_currencyFilterControl->addItem("USD");
    g_lay->addWidget(m_currencyFilterControl, 1, 1);

    QSpacerItem *spcr = new QSpacerItem(10, 10, QSizePolicy::Maximum);
    g_lay->addItem(spcr, 2, 0, 2, 2);

    connect(m_countryFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter(QString)));
    connect(m_currencyFilterControl, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotFilter(QString)));
}
void APIStocksPage::slotFilter(QString f_value)
{
    if (!sender()) return;
    f_value = f_value.trimmed().toLower();

    if (sender()->objectName().contains("country"))
        countryFilter(f_value);
    else if (sender()->objectName().contains("currency"))
        currencyFilter(f_value);

    m_tableBox->searchExec();

    //send visible figi list to coupon page
    int n = m_tableBox->table()->rowCount();
    if (n <= 0) return;

    bool ok;
    QStringList figi_list;
    for (int i=0; i<n; i++)
    {
        quint16 rec_number = m_tableBox->table()->item(i, 0)->data(Qt::UserRole).toUInt(&ok);
        if (ok) figi_list << figiByRecNumber(rec_number);
    }
    emit signalFilter(figi_list);

}
void APIStocksPage::currencyFilter(const QString &f_value)
{
    if (f_value.contains("all")) return;

    QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    if (n == 0) return;

    for (int i=n-1; i>=0; i--)
    {
        QString tv = t->item(i, CURRENCY_COL)->text().trimmed().toLower();
        if (f_value != tv) t->removeRow(i);
    }
}
QString APIStocksPage::dataFile()
{
    return QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(QString("stocks.txt"));
}
void APIStocksPage::resetPage()
{
    APITablePageBase::resetPage();
    m_countryFilterControl->setCurrentIndex(0);
    m_currencyFilterControl->setCurrentIndex(0);
}
void APIStocksPage::loadData()
{
    m_data.clear();

    emit signalMsg(QString("OPEN FILE: %1").arg(APIStocksPage::dataFile()));
    QStringList list;
    QString err = LFile::readFileSL(APIStocksPage::dataFile(), list);
    if (!err.isEmpty()) {emit signalError(err); return;}

    int n_invalid = 0;
    foreach (const QString &v, list)
    {
        if (v.trimmed().isEmpty()) continue;
        if (!v.contains(".") || !v.contains("/")) continue;
        StockDesc rec;
        rec.fromFileLine(v);
        if (!rec.invalid())
        {
            bool filter_ok = true;
            if (api_commonSettings.g_filter.stock.country == "rus" && rec.country != COUNTRY_RU) filter_ok = false;
            if (api_commonSettings.g_filter.stock.country == "usa" && rec.country != COUNTRY_US) filter_ok = false;
            if (filter_ok) m_data.append(rec);
        }
        else {qWarning() << QString("INVALID STOCK LINE: [%1]").arg(v); n_invalid++;}
    }

    emit signalMsg(QString("found invalid lines: %1/%2").arg(n_invalid).arg(list.count()));
    reloadTableByData();
}
void APIStocksPage::reloadTableByData()
{
    resetPage();
    if (m_data.isEmpty())
    {
        emit signalMsg(QString("Stocks data is empty!"));
        return;
    }

    foreach (const StockDesc &rec, m_data)
    {
        LTable::addTableRow(m_tableBox->table(), rec.toTableRowData());
        int l_row = m_tableBox->table()->rowCount() - 1;
        m_tableBox->table()->item(l_row, 0)->setData(Qt::UserRole, rec.number);
        if (!rec.api_trade) LTable::setTableRowColor(m_tableBox->table(), l_row, Qt::lightGray);
    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#AFEEEE", "#3F123E");
    emit signalMsg(QString("loaded validity records: %1 \n").arg(m_data.count()));
    m_tableBox->searchExec();
}
void APIStocksPage::slotGetPaperInfo(QStringList &info)
{
    if (info.isEmpty() || m_data.isEmpty()) return;
    if (info.count() < 2 || info.first() != "share") return;

    QString p_uid = info.at(1).trimmed();
    foreach (const StockDesc &rec, m_data)
    {
        if (rec.uid == p_uid)
        {
            info << rec.name << rec.ticker << "---";
            break;
        }
    }
}
void APIStocksPage::setSelectedUID(QString &uid, quint16 rec_number)
{
    bool need_figi = (uid == "figi");
    uid.clear();
    foreach (const StockDesc &rec, m_data)
        if (rec.number == rec_number) {uid = (need_figi ? rec.figi : rec.uid); break;}
}
void APIStocksPage::setCycleItem(QString &cycle_item, quint16 rec_number)
{
    foreach (const StockDesc &rec, m_data)
    {
        if (rec.number == rec_number)
        {
            cycle_item = QString("stock : %1 : %2").arg(rec.figi).arg(rec.uid);
            break;
        }
    }
}
QString APIStocksPage::figiByRecNumber(quint16 rec_number) const
{
    foreach (const StockDesc &rec, m_data)
        if (rec.number == rec_number) return rec.figi;
    return QString();
}
void APIStocksPage::slotGetPaperInfoByFigi(const QString &figi, QPair<QString, QString> &pair)
{
    foreach (const StockDesc &rec, m_data)
    {
        if (rec.figi == figi || rec.uid == figi)
        {
            pair.first = rec.name;
            pair.second = rec.currency;
            break;
        }
    }
}
void APIStocksPage::slotGetTickerByFigi(const QString &figi, QString &ticker)
{
    foreach (const StockDesc &rec, m_data)
        if (rec.figi == figi || rec.uid == figi)  {ticker = rec.ticker; break;}
}
void APIStocksPage::slotGetPaperTypeByUID(const QString &uid, QString &type)
{
    if (!type.isEmpty()) return;
    foreach (const StockDesc &rec, m_data)
        if (rec.uid == uid)  {type = "share"; break;}
}


//APIBagPage
APIBagPage::APIBagPage(QWidget *parent)
    :APITablePageBase(parent),
      m_bag(NULL)
{
    setObjectName("api_bag_page");
    m_userSign = aptBag;


    m_bag = new BagState(this);
    m_tableBox->setHeaderLabels(m_bag->tableHeaders());
    m_tableBox->setTitle("Positions");
    m_tableBox->vHeaderHide();
    initFilterBox();

    connect(this, SIGNAL(signalLoadPortfolio(const QJsonObject&)), m_bag, SLOT(slotLoadPortfolio(const QJsonObject&)));
    connect(this, SIGNAL(signalLoadPositions(const QJsonObject&)), m_bag, SLOT(slotLoadPositions(const QJsonObject&)));
    connect(m_bag, SIGNAL(signalBagUpdate()), this, SLOT(slotBagUpdate()));

}
void APIBagPage::initFilterBox()
{
    m_filterBox->setTitle("General state");
    if (m_filterBox->layout()) {delete m_filterBox->layout();}
    QGridLayout *g_lay = new QGridLayout(m_filterBox);

    int l_row = 0;
    addGeneralEdit(QString("User ID"), l_row);
    addGeneralEdit(QString("Blocked"), l_row);
    addGeneralEdit(QString("Free"), l_row);
    addGeneralEdit(QString("Total"), l_row);

    QLineEdit *w_line = new QLineEdit(this);
    w_line->setReadOnly(true);
    w_line->setMaximumHeight(2);
    g_lay->addWidget(w_line, l_row, 0, 1, 2); l_row++;

    addGeneralEdit(QString("Full cost of papers"), l_row);
    addGeneralEdit(QString("Current profit"), l_row);


    QSpacerItem *spcr = new QSpacerItem(10, 10, QSizePolicy::Maximum);
    g_lay->addItem(spcr, l_row, 0, 2, 2);
}
void APIBagPage::addGeneralEdit(QString caption, int& row)
{
    QGridLayout *g_lay = qobject_cast<QGridLayout*>(m_filterBox->layout());
    if (g_lay)
    {
        g_lay->addWidget(new QLabel(caption), row, 0);
        caption = caption.toLower().remove(LString::spaceSymbol());
        QLineEdit *edit = new QLineEdit(this);
        edit->setReadOnly(true);
        edit->setObjectName(QString("%1_edit").arg(caption));
        g_lay->addWidget(edit, row, 1);
        row++;
        //qDebug()<<edit->objectName();
    }
    else qWarning(" APIBagPage::addGeneralEdit WARNING - grid layout is NULL");
}
void APIBagPage::slotBagUpdate()
{
    const QObjectList children(m_filterBox->children());
    foreach (QObject *child, children)
    {
        if (!child) continue;
        if (child->objectName().contains("blocked"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strBlocked());
        }
        else if (child->objectName().contains("free"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strFree());
        }
        else if (child->objectName().contains("total"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strTotal());
        }
        else if (child->objectName().contains("userid"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(QString::number(api_commonSettings.user_id));
        }
        else if (child->objectName().contains("papers"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strPapersCost());
        }
        else if (child->objectName().contains("profit"))
        {
            QLineEdit *edit = qobject_cast<QLineEdit*>(child);
            if (edit) edit->setText(m_bag->strCurProfit());
        }
    }
    reloadPosTable();
}
void APIBagPage::reloadPosTable()
{
    resetPage();
    if (!m_bag->hasPositions())
    {
        emit signalMsg(QString("Bag positions list is empty!"));
        return;
    }


    qDebug()<<QString("APIBagPage: m_bag->posCount() %2").arg(m_bag->posCount());
    bool ok;
    for (quint16 i=0; i<m_bag->posCount(); i++)
    {
        const BagPosition &pos = m_bag->posAt(i);
        qDebug()<< pos.uid;
        QStringList p_info;
        p_info << pos.paper_type << pos.uid;
        emit signalGetPaperInfo(p_info);

        QStringList row_data;
        row_data << QString::number(i+1);
        if (p_info.count() != 5) row_data << QString("?") << QString("?");
        else row_data << p_info.at(2) << p_info.at(3);
        row_data << QString::number(pos.count) << pos.strPrice() << pos.strProfit() << pos.paper_type;

        if (pos.paper_type == "bond")
        {
            if (p_info.count() >= 5)
            {
                QDate fd = QDate::fromString(p_info.at(4), InstrumentBase::userDateMask());
                if (fd.isValid())
                {
                    int dn = QDate::currentDate().daysTo(fd);
                    if (dn < 0) dn = -1;
                    row_data << QString::number(dn);
                }
                else row_data << "???";
            }
            else row_data << "????";
        }
        else
        {
            if (p_info.count() >= 5) row_data << p_info.at(4);
            else row_data << "?????";
        }

        LTable::addTableRow(m_tableBox->table(), row_data);
        int l_row = m_tableBox->table()->rowCount() - 1;
        if (pos.paper_type != "bond") LTable::setTableRowColor(m_tableBox->table(), l_row, "#FFFFE0");

        if (pos.curProfit() > 0) m_tableBox->table()->item(l_row, PROFIT_COL)->setTextColor("#006400");
        else if (pos.curProfit() < 0) m_tableBox->table()->item(l_row, PROFIT_COL)->setTextColor("#A52A2A");

        int to_complete = m_tableBox->table()->item(l_row, TO_COMPLETE_COL)->text().toInt(&ok);
        if (!ok) continue;

        if (to_complete <= 0) LTable::setTableTextRowColor(m_tableBox->table(), l_row, QColor("#C0C0C0"));
        else if (to_complete < 20) m_tableBox->table()->item(l_row, TO_COMPLETE_COL)->setTextColor(QColor("#0000CD"));
        else if (to_complete > 180) m_tableBox->table()->item(l_row, TO_COMPLETE_COL)->setTextColor(QColor("#D2691E"));

    }

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    m_tableBox->setSelectionColor("#DEFFBF", "#6B8E23");
    m_tableBox->searchExec();
}


//APITablePageBase
APITablePageBase::APITablePageBase(QWidget *parent)
    :LSimpleWidget(parent, 20),
      m_tableBox(NULL),
      m_filterBox(NULL)
{
    setObjectName("api_base_page");
    initWidgets();
}
void APITablePageBase::initWidgets()
{
    m_tableBox = new LSearchTableWidgetBox(this);
    m_tableBox->setTitle("Data");
    m_filterBox = new QGroupBox(this);
    m_filterBox->setTitle("Filter parameters");

    m_tableBox->resizeByContents();
    m_tableBox->sortingOn();
    h_splitter->addWidget(m_tableBox);
    h_splitter->addWidget(m_filterBox);
}
int APITablePageBase::priceCol() const
{
    if (m_tableBox->table() && m_tableBox->table()->columnCount() > 0)
        return (m_tableBox->table()->columnCount() - 1);
    return -1;
}
void APITablePageBase::resetPage()
{
    LTable::removeAllRowsTable(m_tableBox->table());
    m_tableBox->searchReset();
}
void APITablePageBase::countryFilter(const QString &f_value)
{
    if (f_value.contains("all")) return;

    QTableWidget *t = m_tableBox->table();
    int n = t->rowCount();
    if (n == 0) return;

    //QString f_key_ru("Российская Федерация");
    //QString f_key_us("Соединенные Штаты");

    for (int i=n-1; i>=0; i--)
    {
        QString tv = t->item(i, COUNTRY_COL)->text().trimmed();
        if (f_value.contains("only rus") && tv != COUNTRY_RU) t->removeRow(i);
        else if (f_value.contains("exept rus") && tv == COUNTRY_RU) t->removeRow(i);
        else if (f_value.contains("only usa") && !tv.contains(COUNTRY_US)) t->removeRow(i);
        else if (f_value.contains("exept usa") && tv.contains(COUNTRY_US)) t->removeRow(i);
    }
}
void APITablePageBase::slotSetSelectedUID(QString &uid)
{
    QList<int> sel_rows = LTable::selectedRows(m_tableBox->table());
    if (sel_rows.isEmpty()) {uid.clear(); return;}

    bool ok;
    quint16 rec_number = m_tableBox->table()->item(sel_rows.first(), 0)->data(Qt::UserRole).toUInt(&ok);
    if (!ok) {uid.clear(); return;}

    setSelectedUID(uid, rec_number);
}
void APITablePageBase::slotSetSelectedUIDList(QStringList &uid_list)
{
    uid_list.clear();
    QList<int> sel_rows = LTable::selectedRows(m_tableBox->table());
    if (sel_rows.isEmpty()) return;

    bool ok;
    foreach (int row, sel_rows)
    {
        quint16 rec_number = m_tableBox->table()->item(row, 0)->data(Qt::UserRole).toUInt(&ok);
        if (ok)
        {
            QString uid;
            setSelectedUID(uid, rec_number);
            uid_list.append(uid);
        }
    }
}
void APITablePageBase::slotSetCycleData(QStringList &list)
{
    int n = m_tableBox->table()->rowCount();
    if (n <= 0) return;

    bool ok;
    for (int i=0; i<n; i++)
    {
        if (m_tableBox->table()->isRowHidden(i)) continue;
        quint16 rec_number = m_tableBox->table()->item(i, 0)->data(Qt::UserRole).toUInt(&ok);
        if (ok)
        {
            QString cycle_item;
            setCycleItem(cycle_item, rec_number);
            if (!cycle_item.trimmed().isEmpty()) list.append(cycle_item);
        }
    }
}
void APITablePageBase::slotCyclePrice(const QString &figi, float p)
{
    qDebug()<<QString("APITablePageBase::slotCyclePrice PAGE[%1]  figi=%2   p=%3").arg(objectName()).arg(figi).arg(p);
    int n = m_tableBox->table()->rowCount();
    if (n <= 0) return;

    bool ok;
    int precision = ((p < 10) ? 2 : 1);
    for (int i=0; i<n; i++)
    {
        quint16 rec_number = m_tableBox->table()->item(i, 0)->data(Qt::UserRole).toUInt(&ok);
        if (ok && figi == figiByRecNumber(rec_number))
        {
            if (objectName().contains("bond"))
                p *= (m_tableBox->table()->item(i, NOMINAL_COL)->text().toFloat()/float(100));

            m_tableBox->table()->item(i, priceCol())->setText(QString::number(p, 'f', precision));
            calcProfitability(figi, p);
            break;
        }
    }
}


//APIProfitabilityPage
APIProfitabilityPage::APIProfitabilityPage(QWidget *parent)
    :APITablePageBase(parent),
      m_tick(0)
{
    setObjectName("profitability_page");
    m_userSign = aptProfitability;

    QStringList headers;
    headers << "Company" << "Ticker" << "Finish date" << "Coupon" << "Price/Nominal" << "Finish coupon" << "Finish profit" << "Profitability_M, %";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->setTitle("Profitability by finish date");

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(slotResizeTimer()));
    t->start(1000);
}
void APIProfitabilityPage::slotResizeTimer()
{
    if (m_tick < 0) return;
    m_tick = -1;

    m_tableBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);
    //m_tableBox->setSelectionColor("#DEFFBF", "#6B8E23");
    m_tableBox->searchExec();

}
void APIProfitabilityPage::slotRecalcProfitability(const BondDesc &bond_rec, float price)
{
    if (bond_rec.invalid() || price <= 0) return;
    qDebug("APIProfitabilityPage::slotRecalcProfitability");

    const BCoupon *c_rec = NULL;
    emit signalGetCouponRec(bond_rec.figi, c_rec);
    if (!c_rec ) {qWarning()<<QString("APIProfitabilityPage::slotRecalcProfitability WARNING - not found coupon record by figi [%1]").arg(bond_rec.figi); return;}
    if (c_rec->invalid()) return;


    QStringList row_data;
    row_data << bond_rec.name << bond_rec.isin;
    row_data << QString("%1 (%2)").arg(bond_rec.finish_date.toString(InstrumentBase::userDateMask())).arg(bond_rec.daysToFinish());
    float accumulated = c_rec->size*float(c_rec->period-c_rec->daysTo())/float(c_rec->period);
    row_data << QString("%1/%2/%3").arg(QString::number(accumulated, 'f', 2)).arg(QString::number(c_rec->size-accumulated, 'f', 2)).arg(QString::number(c_rec->size, 'f', 2));
    row_data << QString("%1/%2 (%3)").arg(QString::number(price, 'f', 1)).arg(QString::number(bond_rec.nominal, 'f', 1)).arg(QString::number(bond_rec.nominal-price, 'f', 1));
    float c_finish = c_rec->daySize()*bond_rec.daysToFinish();
    row_data << QString::number(c_finish, 'f', 2);
    float p_finish = c_finish + (bond_rec.nominal-price);
    row_data << QString::number(p_finish, 'f', 2);
    float v_month = ((p_finish*100/(price+accumulated))/float(bond_rec.daysToFinish()))*30;
    row_data << QString("%1/%2").arg(QString::number(v_month, 'f', 3)).arg(QString::number(v_month*12, 'f', 1));

    //sync by table
    syncTableData(bond_rec, row_data, v_month);
}
void APIProfitabilityPage::syncTableData(const BondDesc &bond_rec, const QStringList &row_data, float v_month)
{
    int row = -1;
    int n = m_tableBox->table()->rowCount();
    if (n > 0)
    {
        for (int i=0; i<n-1; i++)
        {
            QString u_data = m_tableBox->table()->item(i, 0)->data(Qt::UserRole).toString();
            if (u_data == bond_rec.figi) {row = i; break;}
        }
    }

    //update table data
    if (row >= 0)
    {
        LTable::setTableRow(row, m_tableBox->table(), row_data);
    }
    else
    {
        LTable::addTableRow(m_tableBox->table(), row_data);
        m_tableBox->table()->item(n, 0)->setData(Qt::UserRole, bond_rec.figi);
        row = n;
    }

    if (v_month > PROFITABILITY_LIMIT)
        m_tableBox->table()->item(row, priceCol())->setTextColor(Qt::blue);
    else if (v_month > PROFITABILITY_LIMIT2)
        m_tableBox->table()->item(row, priceCol())->setTextColor(Qt::darkGreen);

    m_tick++;
}
