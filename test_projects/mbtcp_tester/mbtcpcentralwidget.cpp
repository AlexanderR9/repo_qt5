#include "mbtcpcentralwidget.h"
#include "reqrespwidget.h"
#include "exchangestatewidget.h"

#include <QSplitter>
#include <QTabWidget>


// MBTcpCentralWidget
MBTcpCentralWidget::MBTcpCentralWidget(QWidget *parent)
    :LSimpleWidget(parent, 32),
      m_slaveWidget(NULL),
      m_reqRespWidget(NULL),
      m_stateWidget(NULL),
      m_mode(0)
{
    setObjectName(QString("mbtcp_central_widget"));
//    initWidget();


    m_slaveWidget = new SlaveWidget(this);
    m_reqRespWidget = new ReqRespWidget(this);
    m_stateWidget = new ExchangeStateWidget(this);

    h_splitter->addWidget(m_slaveWidget);
    v_splitter->addWidget(m_reqRespWidget);
    v_splitter->addWidget(m_stateWidget);

}
void MBTcpCentralWidget::reinitWidget()
{

}


//RegDataTable
RegDataTable::RegDataTable(QWidget *parent)
    :LTableWidgetBox(parent, 1)
{
    this->setTitle("");
    this->setFlat(true);
    QStringList headers;
    for (int i=0; i<10; i++) headers << QString::number(i+1);
    setHeaderLabels(headers, Qt::Horizontal);


    this->resizeByContents();
}



// SlaveWidget
SlaveWidget::SlaveWidget(QWidget *parent)
    :LTabWidgetBox(parent, 1)
{
    setObjectName(QString("mbtcp_slave_widget"));
    setTitle("Registers map");
    removeAllPages();

    m_tab->addTab(new RegDataTable(this), "DO");
    m_tab->addTab(new RegDataTable(this), "DI");
    m_tab->addTab(new RegDataTable(this), "AO");
    m_tab->addTab(new RegDataTable(this), "AI");
}



