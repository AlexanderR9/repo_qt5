#include "bb_chartpage.h"
#include "lchart.h"
#include "apiconfig.h"


#include <QSplitter>
#include <QSettings>
#include <QDebug>


//BB_ChartPage
BB_ChartPage::BB_ChartPage(QWidget *parent)
    :LSimpleWidget(parent, 32),
      w_listAll(NULL),
      w_listFavor(NULL),
      w_chart(NULL)
{
    setObjectName("chart_page");
    init();
    loadTickers();

}
void BB_ChartPage::init()
{
    w_listAll = new LListWidgetBox(this);
    w_listAll->setTitle("Tickers");
    w_listAll->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::ExtendedSelection);

    w_listFavor = new LListWidgetBox(this);
    w_listFavor->setTitle("Favor");
    w_chart = new LChartWidget(this);

    delete v_splitter;
    v_splitter = NULL;
    h_splitter->addWidget(w_chart);
    v_splitter = new QSplitter(Qt::Vertical, this);
    h_splitter->addWidget(v_splitter);
    v_splitter->addWidget(w_listAll);
    v_splitter->addWidget(w_listFavor);
}
void BB_ChartPage::loadTickers()
{
    qDebug()<<QString("api_config.tickers %1").arg(api_config.tickers.count());
    foreach (const QString &v, api_config.tickers)
    {
        w_listAll->addItem(v);
    }

}


