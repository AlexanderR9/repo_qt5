#include "jsonviewpage.h"

#include <QTreeWidget>
#include <QSplitter>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>


//JSONViewPage
JSONViewPage::JSONViewPage(QWidget *parent)
    :BB_BasePage(parent, 20),
      m_replyBox(NULL)
{
    setObjectName("json_view_page");
    initWidgets();
}
void JSONViewPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    BB_BasePage::slotJsonReply(req_type, j_obj);

    m_replyBox->loadJSON(j_obj, "JSON struct");
    m_replyBox->expandLevel();
    m_replyBox->resizeByContents();
    m_replyBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
}
void JSONViewPage::setExpandLevel(int a)
{
    qDebug()<<QString("JSONViewPage::setExpandLevel %1").arg(a);
    m_replyBox->setExpandLevel(a);
}
void JSONViewPage::initWidgets()
{
    m_replyBox = new LTreeWidgetBox(this);
    m_replyBox->setTitle("Reply data (HTTP)");

    QStringList headers;
    headers << "Key" << "Value" << "Data type";
    m_replyBox->setHeaderLabels(headers);
    headers.clear();
    headers << "JSON struct" << QString() << QString();
    m_replyBox->addRootItem(headers);
    m_replyBox->setRootItemAttrs(Qt::darkCyan, 0, false, true);
    m_replyBox->resizeByContents();
    m_replyBox->view()->setSelectionMode(QAbstractItemView::SingleSelection);
    m_replyBox->view()->setSelectionBehavior(QAbstractItemView::SelectRows);

    h_splitter->addWidget(m_replyBox);
}
