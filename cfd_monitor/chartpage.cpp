#include "chartpage.h"
#include "ltable.h"
#include "lsearch.h"
#include "logpage.h"
#include "lchart.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QDateTime>


//ChartPage
ChartPage::ChartPage(QWidget *parent)
    :BasePage(parent),
    m_search(NULL),
    m_chart(NULL)
{
    setupUi(this);

    initSearch();
    initChart();

    connect(sourcesListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotRepaintChart()));

}
void ChartPage::slotRepaintChart()
{
    QList<QListWidgetItem*> items = sourcesListWidget->selectedItems();
    if (items.isEmpty() || !m_chart) return;

    m_chart->clearChartPoints(0);
    QString ticker = items.first()->text();

    QList<QPointF> points;
    prepareChartPoints(ticker, points);
    if (points.isEmpty())
    {
        QString err = QString("Not found chart data for %1").arg(ticker);
        signalError(err);
        sendLog(err, 2);
    }
    else m_chart->addChartPoints(points, 0);
    m_chart->updateAxis();
}
void ChartPage::prepareChartPoints(const QString &ticker, QList<QPointF> &points)
{
    points.clear();
    QMap<QDateTime, float> chart_data;
    emit signalGetChartData(ticker, chart_data);

    QMap<QDateTime, float>::const_iterator it = chart_data.constBegin();
    while (it != chart_data.constEnd())
    {
        QPointF pt(it.key().toSecsSinceEpoch(), it.value());
        points.append(pt);
        it++;
    }
}
void ChartPage::initChart()
{
    m_chart = new LChartWidget(this);
    if (chartBox->layout()) delete chartBox->layout();
    QHBoxLayout *h_lay = new QHBoxLayout(0);
    chartBox->setLayout(h_lay);
    h_lay->addWidget(m_chart);

    m_chart->setCrossColor(QColor(150, 190, 150));
    m_chart->setAxisXType(LChartAxisParams::xvtDate);
    m_chart->setCrossXAxisTextViewMode(2);
    //m_chart->setAxisOffsets(12, 12, 50);
    m_chart->setPointSize(4);

    LChartParams p;
    p.lineColor = QColor(170, 50, 40);
    p.pointsColor = QColor(0, 100, 0);
    p.points.clear();
    m_chart->addChart(p);
    m_chart->updateAxis();

}
void ChartPage::initSearch()
{
    m_search = new LSearch(searchLineEdit, this);
    m_search->addList(sourcesListWidget, countLabel);
    searchExec();

    QPalette p(sourcesListWidget->palette());
    p.setColor(QPalette::Foreground, Qt::red);
    p.setColor(QPalette::Text, QColor(30, 140, 250));
    sourcesListWidget->setPalette(p);
}
void ChartPage::initSource()
{
    QStringList list;
    emit signalGetSource(list);


    if (list.isEmpty())
    {
        emit signalError("ChartPage: source list is empty");
        sendLog("Source list is empty", 1);
        return;
    }

    sourcesListWidget->addItems(list);
    searchExec();

    sendLog(QString("Source list loaded OK"), 0);

}
void ChartPage::searchExec()
{
    m_search->exec();
    countLabel->setText(countLabel->text().remove("Record").trimmed());
}
void ChartPage::sendLog(const QString &text, int result)
{
    LogStruct log(amtChartPage, result);
    log.msg = text;
    emit signalSendLog(log);
}



