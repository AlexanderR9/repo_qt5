#include "exchangestatewidget.h"
#include "ltable.h"
#include <QTableWidget>
#include <QSplitter>



//Parametes ViewWidget
ExchangeStateWidget::ExchangeStateWidget(QWidget *parent)
    :LSimpleWidget(parent, 10),
    m_tableBox(NULL)
{
    setObjectName(QString("skd_exhange_widget"));
    initWidget();
}
void ExchangeStateWidget::initWidget()
{
    m_tableBox = new LTableWidgetBox(this);
    //m_tableBox->setTitle("Exchange state");
    m_tableBox->setHeaderLabels(stateTableHeaders(), Qt::Vertical);
    m_tableBox->setHeaderLabels(QStringList("Value"), Qt::Horizontal);
    for (int i=0; i<m_tableBox->table()->rowCount(); i++)
        LTable::createTableItem(m_tableBox->table(), i, 0, "-");

    v_splitter->addWidget(m_tableBox);

    m_tableBox->resizeByContents();
}
QStringList ExchangeStateWidget::stateTableHeaders() const
{
    QStringList headers;
    headers.append("Connection status");
    headers.append("TCP host");
    headers.append("TCP port");
    //headers.append("Modbus device state");
    headers.append("Received time");
    headers.append("Received packet size, b");
    headers.append("Received packets (total)");

    //headers.append("MBTCP request command");
    //headers.append("Device address ");
    headers.append("Sended time");
    headers.append("Sended packet size, b");
    headers.append("Sended packets (total)");

    //headers.append("MBTCP Response command");
    headers.append("Connection errors");
    //headers.append("Modbus errors");
    return headers;
}
void ExchangeStateWidget::updateStatistic(const QStringList &list)
{
    QColor cell_color = Qt::black;
    int rows = m_tableBox->table()->rowCount();
    for (int i=0; i<list.count(); i++)
    {
        cell_color = Qt::black;
        if (i < rows) m_tableBox->table()->item(i, 0)->setText(list.at(i));
        else break;

        if (i == 0)
        {
            cell_color = Qt::gray;
            if (list.at(i).toLower().contains("conn")) cell_color = Qt::darkGreen;
            else if (list.at(i).toLower().contains("listen")) cell_color = Qt::darkYellow;
            else if (list.at(i).toLower().contains("open")) cell_color = Qt::darkYellow;
            m_tableBox->table()->item(i, 0)->setTextColor(cell_color);
        }
        if (i == rows-1)
        {
            bool ok;
            quint32 n_err = list.at(i).toUInt(&ok);
            if (ok && n_err > 0) cell_color = Qt::red;
            m_tableBox->table()->item(i, 0)->setTextColor(cell_color);
        }
    }
    m_tableBox->resizeByContents();
}

