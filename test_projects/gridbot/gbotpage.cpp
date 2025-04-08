#include "gbotpage.h"
#include "gbotobj.h"
#include "lstring.h"
#include "ltable.h"

#include <QDebug>
#include <QSplitter>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QSpacerItem>
#include <QSettings>
#include <QTableWidget>


//GBotPage
GBotPage::GBotPage(QWidget *parent)
    :LSimpleWidget(parent, 21),
      m_table(NULL),
      m_bot(NULL)
{
    m_userSign = 0;
    setObjectName("gbot_page");

    initPage();
    m_table->resizeByContents();
    m_table->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);

    m_bot = new GBotObj(this);
    connect(m_bot, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_bot, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));
    connect(m_bot, SIGNAL(signalTableResult(const QStringList&)), this, SLOT(slotReceivedTableResult(const QStringList&)));

}
void GBotPage::exec()
{
    qDebug("GBotPage::exec()");
    reset();
    setBotParams();
    m_bot->exec();
}
void GBotPage::reset()
{
    qDebug("GBotPage::reset()");
    m_table->removeAllRows();
    m_bot->resetParams();
    m_table->resizeByContents();
}
void GBotPage::initPage()
{
    QGroupBox *p_box = new QGroupBox("Parameters", this);
    m_table = new LTableWidgetBox(this);

    QStringList headers;
    headers << "Grid value" << "Deviation, %" << "Step" << "Step lot" << "Trade type" <<
               "Amount lot" << "Step profit" << "Result" << "Profit";
    // grid_value / step_number / step_lot / trade_type / amount_lot / step_profit / step_result

    m_table->setHeaderLabels(headers);
    m_table->setTitle("Statistic");

    h_splitter->addWidget(p_box);
    h_splitter->addWidget(m_table);
    initParamBox(p_box);
}
void GBotPage::initParamBox(QGroupBox *box)
{
    if (box->layout()) delete box->layout();
    QGridLayout *g_lay = new QGridLayout(0);
    box->setLayout(g_lay);

    int row = 0;
    g_lay->addWidget(new QLabel("Input sum"), row, 0);
    m_inputSumEdit = new QLineEdit("100.0");
    g_lay->addWidget(m_inputSumEdit, row, 1);
    row++;

    g_lay->addWidget(new QLabel("Leverage"), row, 0);
    m_leverageEdit = new QComboBox();
    m_leverageEdit->clear();
    m_leverageEdit->addItems(QStringList() << "none" << "2" << "4" << "6" << "8" << "10" << "12" << "16");
    g_lay->addWidget(m_leverageEdit, row, 1);
    row++;

    g_lay->addWidget(new QLabel("Grid count"), row, 0);
    m_gridCountEdit = new QLineEdit("20");
    g_lay->addWidget(m_gridCountEdit, row, 1);
    row++;

    g_lay->addWidget(new QLabel("Range"), row, 0);
    m_rangeEdit = new QLineEdit("0.02");
    g_lay->addWidget(m_rangeEdit, row, 1);
    row++;

    g_lay->addWidget(new QLabel("Start price"), row, 0);
    m_curPriceEdit = new QLineEdit("1.02");
    g_lay->addWidget(m_curPriceEdit, row, 1);
    row++;


    g_lay->addWidget(new QLabel("Trade direction"), row, 0);
    m_directionEdit = new QComboBox();
    m_directionEdit->clear();
    m_directionEdit->addItems(QStringList() << "Short" << "Long");
    g_lay->addWidget(m_directionEdit, row, 1);
    row++;

    QSpacerItem *spcr = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    g_lay->addItem(spcr, row, 0);
}
void GBotPage::load(QSettings &settings)
{
    LSimpleWidget::load(settings);

    m_inputSumEdit->setText(settings.value(QString("%1/input_sum").arg(objectName())).toString());
    m_leverageEdit->setCurrentIndex(settings.value(QString("%1/leverage_index").arg(objectName())).toInt());
    m_gridCountEdit->setText(settings.value(QString("%1/grid_count").arg(objectName())).toString());
    m_rangeEdit->setText(settings.value(QString("%1/range").arg(objectName())).toString());
    m_curPriceEdit->setText(settings.value(QString("%1/start_price").arg(objectName())).toString());
    m_directionEdit->setCurrentIndex(settings.value(QString("%1/direction_index").arg(objectName())).toInt());

}
void GBotPage::save(QSettings &settings)
{
    LSimpleWidget::save(settings);


    settings.setValue(QString("%1/input_sum").arg(objectName()), m_inputSumEdit->text());
    settings.setValue(QString("%1/leverage_index").arg(objectName()), m_leverageEdit->currentIndex());
    settings.setValue(QString("%1/grid_count").arg(objectName()), m_gridCountEdit->text());
    settings.setValue(QString("%1/range").arg(objectName()), m_rangeEdit->text());
    settings.setValue(QString("%1/start_price").arg(objectName()), m_curPriceEdit->text());
    settings.setValue(QString("%1/direction_index").arg(objectName()), m_directionEdit->currentIndex());

}
void GBotPage::setBotParams()
{
    qDebug("GBotPage::setBotParams()");

    GBotParams params;

    bool ok;
    params.in_sum = m_inputSumEdit->text().toFloat(&ok);
    if (!ok || params.in_sum < 1) {emit signalError("Invalid input sum"); return;}
    params.leverage = 1;
    if (m_leverageEdit->currentIndex() > 0)
        params.leverage = m_leverageEdit->currentText().toUInt();
    params.direction = m_directionEdit->currentIndex();

    params.grid_count = m_gridCountEdit->text().toUInt(&ok);
    if (!ok || params.grid_count < 4) {emit signalError("Invalid grid count"); return;}
    params.start_price = m_curPriceEdit->text().toFloat(&ok);
    if (!ok || params.start_price < 0.1) {emit signalError("Invalid start price"); return;}
    parseRange(params.range);
    if (params.range.first <= 0) {emit signalError("Invalid price range"); return;}

    emit signalMsg("Start testing");
    emit signalMsg(QString("RANGE[%1 - %2]").arg(params.range.first).arg(params.range.second));

    m_bot->setParams(params);
}
void GBotPage::parseRange(QPair<float, float> &range)
{
    range.first = range.second = -1;
    QString s = m_rangeEdit->text().trimmed();
    if (s.isEmpty()) return;

    s = LString::removeSpaces(s);
    int pos = s.indexOf(":");
    if (pos > 0)
    {
        bool ok;
        float p1 = s.left(pos).toFloat(&ok);
        if (!ok) return;
        s = LString::strTrimLeft(s, pos+1);
        float p2 = s.toFloat(&ok);
        if (!ok) return;

        if (p1 > 0 && p1 < p2)
        {
            range.first = p1;
            range.second = p2;
        }
    }
}
void GBotPage::slotReceivedTableResult(const QStringList &data)
{
    QTableWidget *t = m_table->table();
    for (int i=0; i<data.count(); i++)
    {
        QString s = data.at(i).trimmed();
        QStringList row_data = LString::trimSplitList(s, " / ");
        LTable::addTableRow(t, row_data);
        int last_row = t->rowCount() - 1;

        if (s.contains("start"))
            LTable::setTableRowColor(t, last_row, "#FFFF00");

        if (row_data.last().at(0) == '-') t->item(last_row, row_data.count()-1)->setTextColor("#B22222");
        else if (row_data.last().contains("%")) t->item(last_row, row_data.count()-1)->setTextColor("#556B2F");
    }
    m_table->resizeByContents();
}

