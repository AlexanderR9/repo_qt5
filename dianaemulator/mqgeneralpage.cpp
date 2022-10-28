#include "mqgeneralpage.h"
#include "ltable.h"
#include "mq.h"

#include <QTableWidget>
#include <QTreeWidget>
#include <QSplitter>
#include <QDebug>




//MQGeneralPage
MQGeneralPage::MQGeneralPage(QWidget *parent)
    :LSimpleWidget(parent, 22),
    m_tableBox(NULL),
    m_viewBox(NULL)
{
    setObjectName(QString("mq_general_page"));

    initWidget();
}
void MQGeneralPage::initWidget()
{
    m_tableBox = new LTableWidgetBox(this);
    m_tableBox->setTitle("MQ queues");
    m_viewBox = new LTreeWidgetBox(this);
    m_viewBox->setTitle("Exchange statistic");

    h_splitter->addWidget(m_tableBox);
    h_splitter->addWidget(m_viewBox);

    QStringList headers;
    headers << "Module" << "Mode" << "POSIX name" << "Msg size, b" << "State" << "Attributes";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->vHeaderHide();

    headers.clear();
    headers << "Queue name" << "Received msg" << "Sended msg";
    m_viewBox->setHeaderLabels(headers);

    m_tableBox->table()->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableBox->table()->setSelectionMode(QAbstractItemView::SingleSelection);
    LTable::removeAllRowsTable(m_tableBox->table());

}
void MQGeneralPage::slotAppendMQ(const QString &diana_name, quint32 msg_size, const MQ *mq)
{
    if (!mq) return;

    qDebug()<<QString("slotAppendMQ  [%1]").arg(mq->name());

    QStringList row_data;
    row_data << diana_name;
    QString s_type = "?";
    if (mq->name().contains("input")) s_type = "ReadOnly";
    else if (mq->name().contains("output")) s_type = "WriteOnly";
    row_data << s_type;
    row_data << mq->name() << QString::number(msg_size) << mq->strState() << mq->strAttrs();

    LTable::addTableRow(m_tableBox->table(), row_data);
    LTable::resizeTableContents(m_tableBox->table());
}

