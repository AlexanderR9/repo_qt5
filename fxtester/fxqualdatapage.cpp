#include "fxqualdatapage.h"
#include "fxbarcontainer.h"
#include "ltable.h"

#include <QSplitter>
#include <QTableWidget>
#include <QListWidget>


//FXQualDataPage
FXQualDataPage::FXQualDataPage(QWidget *parent)
    :LSimpleWidget(parent, 22),
      m_tableBox(NULL),
      m_listBox(NULL)
{
    setObjectName("fx_qualdata_page");

    initWidgets();

}
void FXQualDataPage::initWidgets()
{
    m_tableBox = new LTableWidgetBox(this);
    m_tableBox->setTitle("Time statistic");
    QStringList headers;
    headers << "Open time" << "Week day" << "Prev. days" << "Price";
    m_tableBox->setHeaderLabels(headers);

    m_listBox = new LListWidgetBox(this);
    m_listBox->setTitle("Integrated result");

    h_splitter->addWidget(m_tableBox);
    h_splitter->addWidget(m_listBox);
}
void FXQualDataPage::check(const FXBarContainer *data)
{
    if (!data) return;

    reset();
    emit signalMsg(QString("check quality data: %1").arg(data->toStr()));

    int n = data->barCount();
    if (n < 2)
    {
        emit signalMsg(QString("WARNING: data bar count %1").arg(n));
        return;
    }

    int days_full = data->barAt(0)->time().daysTo(data->barAt(n-1)->time());

    QStringList row_data;
    uint fall_days = 0;
    for (int i=1; i<n; i++)
    {
        const FXBar *bar = data->barAt(i);
        const FXBar *bar_prev = data->barAt(i-1);
        if (!bar || !bar_prev)
        {
            qWarning("FXQualDataPage::check - WARNING !bar || !bar_prev");
            continue;
        }

        row_data.clear();
        row_data << bar->strTime() << QString::number(bar->time().date().dayOfWeek());
        int d = bar_prev->time().daysTo(bar->time());
        row_data << QString::number(d) << QString::number(bar->open(), 'f', data->digist());

        LTable::addTableRow(m_tableBox->table(), row_data);

        if (d > 3)
        {
            m_tableBox->table()->item(m_tableBox->table()->rowCount()-1, 2)->setTextColor(Qt::red);
            fall_days += (d-3);
        }

    }

    LTable::resizeTableContents(m_tableBox->table());
    m_listBox->listWidget()->addItem(QString("Fall days %1/%2").arg(fall_days).arg(days_full));
    m_listBox->listWidget()->addItem(QString("Quality %1%").arg(QString::number(100 - double(100*fall_days)/days_full, 'f', 1)));


}
void FXQualDataPage::reset()
{
    LTable::removeAllRowsTable(m_tableBox->table());
    m_listBox->listWidget()->clear();

}




