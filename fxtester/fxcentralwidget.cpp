#include "fxcentralwidget.h"
#include "fxdataloader.h"
#include "fxdataloaderwidget.h"
#include "fxchartwidget.h"
#include "fxbarcontainer.h"

#include <QStackedWidget>
#include <QSplitter>
#include <QGroupBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QIcon>
#include <QListWidgetItem>
#include <QSettings>


//FXCentralWidget
FXCentralWidget::FXCentralWidget(QWidget *parent)
    :LSimpleWidget(parent, 32),
      m_stackedWidget(NULL),
      m_loaderWidget(NULL),
      m_pagesListWidget(NULL)
{
    setObjectName("fx_central_widget");

    initWidgets();
    createPages();

}
void FXCentralWidget::loaderDataUpdate(const FXDataLoader *loader)
{

    QList<FXCoupleDataParams> list;
    if (loader->dataEmpty())
    {
        emit signalError("FXCentralWidget: loaded data is empty");
    }
    else
    {
        for (int i=0; i<loader->count(); i++)
        {
            const FXBarContainer *data = loader->containerAt(i);
            if (!data) continue;

            FXCoupleDataParams p(data->couple(), data->timeframe());
            p.bar_count = data->barCount();
            list.append(p);
        }
    }
    m_loaderWidget->reloadData(list);
}
void FXCentralWidget::initWidgets()
{
    m_stackedWidget = new QStackedWidget(this);
    h_splitter->addWidget(m_stackedWidget);

    QGroupBox *p_box = new QGroupBox("Pages types", this);
    if (p_box->layout()) delete p_box->layout();
    p_box->setLayout(new QVBoxLayout(0));
    m_pagesListWidget = new QListWidget(this);
    m_pagesListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pagesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    p_box->layout()->addWidget(m_pagesListWidget);
    v_splitter->addWidget(p_box);

    m_loaderWidget = new FXDataLoaderWidget(this);
    v_splitter->addWidget(m_loaderWidget);

    connect(m_loaderWidget, SIGNAL(signalSelectionChanged()), this, SLOT(slotSelectionDataChanged()));
    connect(m_pagesListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectedPageChanged()));


}
void FXCentralWidget::createPages()
{
    //clear m_stackedWidget
    while (m_stackedWidget->count() > 0)
    {
        QWidget *w = m_stackedWidget->widget(0);
        m_stackedWidget->removeWidget(w);
    }

    //create chart page
    FXChartWidget *w_chart = new FXChartWidget(this);
    m_pages.insert(fxptChart, w_chart);

    //fill stack
    int i = 0;
    foreach (LSimpleWidget *page, m_pages)
    {
        m_stackedWidget->insertWidget(i, page);
        QListWidgetItem *p_item = new QListWidgetItem(QIcon(page->iconPath()), page->caption());
        m_pagesListWidget->addItem(p_item);
        i++;
    }

    if (m_pagesListWidget->count() > 0)
        m_pagesListWidget->item(0)->setSelected(true);
}
void FXCentralWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);
    foreach (LSimpleWidget *page, m_pages)
        page->load(settings);
}
void FXCentralWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);
    foreach (LSimpleWidget *page, m_pages)
        page->save(settings);
}
void FXCentralWidget::slotSelectionDataChanged()
{
    qDebug("FXCentralWidget::slotSelectionDataChanged()");
    if (m_pages.value(fxptChart)->isActiveWindow())
    {
        QList<FXCoupleDataParams> req;
        m_loaderWidget->getSelection(req);

        QList<const FXBarContainer*> paint_data;
        emit signalGetLoadedDataByReq(req, paint_data);
        m_pages.value(fxptChart)->repaint();

        FXChartWidget *chart_page = qobject_cast<FXChartWidget*>(m_pages.value(fxptChart));
        if (chart_page) chart_page->repaintChart(paint_data);
    }
}
void FXCentralWidget::slotSelectedPageChanged()
{
    qDebug("FXCentralWidget::slotSelectedPageChanged()");

}


