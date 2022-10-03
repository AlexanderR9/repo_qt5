#include "fxcentralwidget.h"
#include "fxdataloader.h"
#include "fxdataloaderwidget.h"
#include "fxchartwidget.h"
#include "fxtestpage.h"
#include "fxqualdatapage.h"
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
void FXCentralWidget::setChartSettings(const FXChartSettings &chs)
{
    FXChartWidget *chart_page = qobject_cast<FXChartWidget*>(m_pages.value(fxptChart));
    if (chart_page)
    {
        chart_page->setChartSettings(chs);
        if (chart_page->isActiveWindow()) chart_page->update();
    }
}
int FXCentralWidget::currentPageType() const
{
    QList<QListWidgetItem*> list = m_pagesListWidget->selectedItems();
    if (list.isEmpty()) return -1;
    return (list.last()->data(Qt::UserRole).toInt() + fxptChart);
}
void FXCentralWidget::checkQualData()
{
    QList<FXCoupleDataParams> list;
    m_loaderWidget->getSelection(list);
    if (list.isEmpty())
    {
        emit signalError("FXCentralWidget: data selection is empty");
        return;
    }
    if (list.count() > 1)
    {
        emit signalError("FXCentralWidget: select only one couple");
        return;
    }

    QList<const FXBarContainer*> data;
    emit signalGetLoadedDataByReq(list, data);
    if (data.isEmpty())
    {
        emit signalError("FXCentralWidget: ckecking data is empty");
        return;
    }

    FXQualDataPage *q_page = qobject_cast<FXQualDataPage*>(m_pages.value(fxptQualData));
    if (q_page) q_page->check(data.first());
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
void FXCentralWidget::clearPages()
{
    clearStackedWidget();
    qDeleteAll(m_pages);
    m_pages.clear();
}
void FXCentralWidget::clearStackedWidget()
{
    while (m_stackedWidget->count() > 0)
    {
        QWidget *w = m_stackedWidget->widget(0);
        m_stackedWidget->removeWidget(w);
    }
}
void FXCentralWidget::createPages()
{
    //clear m_stackedWidget and m_pages
    clearPages();

    //create chart page
    FXChartWidget *w_chart = new FXChartWidget(this);
    m_pages.insert(fxptChart, w_chart);

    //create testing page
    FXTestPage *t_page = new FXTestPage(this);
    m_pages.insert(fxptTester, t_page);

    //create quality page
    FXQualDataPage *qd_page = new FXQualDataPage(this);
    m_pages.insert(fxptQualData, qd_page);



    //fill stack
    /*
    int i = 0;
    QMap<int, LSimpleWidget*>::const_iterator it = m_pages.constBegin();
    while (it != m_pages.constEnd())
    {
        m_stackedWidget->insertWidget(i, it.value());
        QListWidgetItem *p_item = new QListWidgetItem(QIcon(it.value()->iconPath()), it.value()->caption());
        p_item->setData(Qt::UserRole, it.key());
        m_pagesListWidget->addItem(p_item);

        connect(it.value(), SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
        connect(it.value(), SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));

        i++;
        it++;
    }
    */

    int i = 0;
    foreach (LSimpleWidget *page, m_pages)
    {
        m_stackedWidget->insertWidget(i, page);
        QListWidgetItem *p_item = new QListWidgetItem(QIcon(page->iconPath()), page->caption());
        p_item->setData(Qt::UserRole, i);
        m_pagesListWidget->addItem(p_item);

        connect(page, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
        connect(page, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));

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

    int p_index = settings.value(QString("%1/page_index").arg(objectName()), 0).toInt();
    m_pagesListWidget->clearFocus();
    m_pagesListWidget->clearSelection();
    m_pagesListWidget->item(p_index)->setSelected(true);
    //slotSelectedPageChanged();

}
void FXCentralWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);
    foreach (LSimpleWidget *page, m_pages)
        page->save(settings);

    settings.setValue(QString("%1/page_index").arg(objectName()), m_stackedWidget->currentIndex());
}
void FXCentralWidget::slotSelectionDataChanged()
{
    if (!m_loaderWidget) return;

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
    QList<QListWidgetItem*> list =  m_pagesListWidget->selectedItems();
    if (list.isEmpty()) return;

    int p_index = list.last()->data(Qt::UserRole).toInt();
    if (p_index >= 0 && p_index < m_stackedWidget->count())
        m_stackedWidget->setCurrentIndex(p_index);

}


