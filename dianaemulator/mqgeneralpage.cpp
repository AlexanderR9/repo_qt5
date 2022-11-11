#include "mqgeneralpage.h"
#include "ltable.h"
#include "mq.h"
#include "dianaobj.h"

#include <QTableWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QDebug>


#define MQ_MODE_COL         1
#define MQ_STATE_COL        4
#define MQ_ATTR_COL         5

#define VIEW_RECEIVED_COL   1
#define VIEW_SENDED_COL     2
#define VIEW_ERRS_COL       3

//MQGeneralPage
MQGeneralPage::MQGeneralPage(QWidget *parent)
    :LSimpleWidget(parent, 22),
    m_tableBox(NULL),
    m_viewBox(NULL)
{
    setObjectName(QString("mq_general_page"));

    m_queues.clear();
    initWidget();

}
void MQGeneralPage::initWidget()
{
    m_tableBox = new LTableWidgetBox(this);
    m_tableBox->setTitle("MQ queues");
    m_viewBox = new LTreeWidgetBox(this);
    m_viewBox->setTitle("Exchange statistic");
    m_viewBox->view()->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_viewBox->view()->setSelectionMode(QAbstractItemView::SingleSelection);

    h_splitter->addWidget(m_tableBox);
    h_splitter->addWidget(m_viewBox);

    QStringList headers;
    headers << "Module" << "Mode" << "POSIX name" << "Msg size, b" << "State" << "Attributes";
    m_tableBox->setHeaderLabels(headers);
    m_tableBox->vHeaderHide();

    headers.clear();
    headers << "Queue name" << "Received msg" << "Sended msg" << "Errors";
    m_viewBox->setHeaderLabels(headers);

    m_tableBox->table()->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableBox->table()->setSelectionMode(QAbstractItemView::SingleSelection);
    LTable::removeAllRowsTable(m_tableBox->table());
}
void MQGeneralPage::slotAppendMQ(const QString &diana_name, quint32 msg_size, const MQ *mq)
{
    if (!mq) return;

    //qDebug()<<QString("slotAppendMQ  [%1]").arg(mq->name());
    m_queues.insert(m_tableBox->table()->rowCount(), mq);

    if (mq->name().contains(DianaObject::inputType()))
        appendDianaToView(diana_name, DianaObject::inputType());
    else if (mq->name().contains(DianaObject::outputType()))
        appendDianaToView(diana_name, DianaObject::outputType());

    QStringList row_data;
    row_data << diana_name << mq->strMode() << mq->name() << QString::number(msg_size) << mq->strState() << mq->strAttrs();
    LTable::addTableRow(m_tableBox->table(), row_data);
    LTable::resizeTableContents(m_tableBox->table());
    updateMQState();
}
void MQGeneralPage::appendDianaToView(const QString &diana_name, const QString &type)
{
    int pos = viewDianaIndex(diana_name);
    if (pos < 0)
    {
        QTreeWidgetItem *diana_item = new QTreeWidgetItem(view());
        diana_item->setText(0, diana_name);
        diana_item->setTextColor(0, QColor(70, 130, 180));
        view()->addTopLevelItem(diana_item);
        pos = view()->topLevelItemCount() - 1;
    }

    //qDebug()<<QString("MQGeneralPage::appendDianaToView  %1/%2  %3").arg(diana_name).arg(pos).arg(type);
    QTreeWidgetItem *item = new QTreeWidgetItem(view()->topLevelItem(pos));
    item->setText(0, type);
    item->setText(1, QString::number(0));
    item->setText(2, QString::number(0));
    item->setText(3, QString::number(0));
    view()->expandAll();
}
int MQGeneralPage::viewDianaIndex(const QString &diana_name) const
{
    int n = view()->topLevelItemCount();
    for (int i=0; i<n; i++)
    {
        if (view()->topLevelItem(i)->text(0) == diana_name)
            return i;
    }
    return -1;
}
void MQGeneralPage::updateMQState()
{
    //qDebug("MQGeneralPage::updateMQState()");
    for(int i=0; i<m_tableBox->table()->rowCount(); i++)
    {
        const MQ *mq = m_queues.value(i);
        if (mq)
        {
            m_tableBox->table()->item(i, MQ_MODE_COL)->setText(mq->strMode());
            m_tableBox->table()->item(i, MQ_STATE_COL)->setText(mq->strState());
            m_tableBox->table()->item(i, MQ_STATE_COL)->setTextColor(mq->colorStatus());
            m_tableBox->table()->item(i, MQ_ATTR_COL)->setText(mq->strAttrs());
        }
    }
    LTable::resizeTableContents(m_tableBox->table());
}
QTreeWidget* MQGeneralPage::view() const
{
    return (m_viewBox ? m_viewBox->view() : NULL);
}
void MQGeneralPage::slotSendMsgOk(const QString &diana_name)
{
    int pos = viewDianaIndex(diana_name);
    if (pos >= 0)
    {
        QTreeWidgetItem *item = view()->topLevelItem(pos);
        if (item)
        {
            QTreeWidgetItem *in_item = item->child(0);
            if (in_item)
            {
                int n = in_item->text(VIEW_SENDED_COL).toInt() + 1;
                in_item->setText(VIEW_SENDED_COL, QString::number(n));
            }
        }
    }
}
void MQGeneralPage::slotReceiveMsgOk(const QString &diana_name)
{
    int pos = viewDianaIndex(diana_name);
    if (pos >= 0)
    {
        QTreeWidgetItem *item = view()->topLevelItem(pos);
        if (item)
        {
            QTreeWidgetItem *in_item = item->child(1);
            if (in_item)
            {
                int n = in_item->text(VIEW_RECEIVED_COL).toInt() + 1;
                in_item->setText(VIEW_RECEIVED_COL, QString::number(n));
            }
        }
    }
}
void MQGeneralPage::slotSendMsgErr(const QString &diana_name)
{
    int pos = viewDianaIndex(diana_name);
    if (pos >= 0)
    {
        QTreeWidgetItem *item = view()->topLevelItem(pos);
        if (item)
        {
            QTreeWidgetItem *in_item = item->child(0);
            if (in_item)
            {
                int n = in_item->text(VIEW_ERRS_COL).toInt() + 1;
                in_item->setText(VIEW_ERRS_COL, QString::number(n));
                in_item->setTextColor(VIEW_ERRS_COL, Qt::red);
            }
        }
    }
}
void MQGeneralPage::slotReceiveMsgErr(const QString &diana_name)
{
    int pos = viewDianaIndex(diana_name);
    if (pos >= 0)
    {
        QTreeWidgetItem *item = view()->topLevelItem(pos);
        if (item)
        {
            QTreeWidgetItem *in_item = item->child(1);
            if (in_item)
            {
                int n = in_item->text(VIEW_ERRS_COL).toInt() + 1;
                in_item->setText(VIEW_ERRS_COL, QString::number(n));
                in_item->setTextColor(VIEW_ERRS_COL, Qt::red);
            }
        }
    }
}



