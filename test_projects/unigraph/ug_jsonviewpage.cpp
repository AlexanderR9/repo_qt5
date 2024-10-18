#include "ug_jsonviewpage.h"
#include "ug_apistruct.h"

#include <QTreeWidget>
#include <QSplitter>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QLineEdit>
#include <QSettings>
#include <QVBoxLayout>


//UG_JSONViewPage
UG_JSONViewPage::UG_JSONViewPage(QWidget *parent)
    :UG_BasePage(parent, 10, rtJsonView),
      m_replyBox(NULL),
      m_precision(5),
      m_reqEdit(NULL)
{
    setObjectName("json_view_page");
    initWidgets();
    initQueryBox();

    setPrecision(3);
}
QString UG_JSONViewPage::freeQueryData() const
{
    QString data(m_reqEdit->text().trimmed());
    if (data.isEmpty()) return data;
    return QString("{ %1 }").arg(data);
}
void UG_JSONViewPage::slotJsonReply(int req_type, const QJsonObject &j_obj)
{
    Q_UNUSED(req_type);

    m_replyBox->loadJSON(j_obj, "JSON struct");
    m_replyBox->expandLevel();
    m_replyBox->resizeByContents();
    m_replyBox->setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::SingleSelection);
    cutPrecision(m_replyBox->rootItem());
}
void UG_JSONViewPage::setExpandLevel(int a)
{
    qDebug()<<QString("UG_JSONViewPage::setExpandLevel %1").arg(a);
    m_replyBox->setExpandLevel(a);
}
void UG_JSONViewPage::initWidgets()
{
    m_replyBox = new LTreeWidgetBox(this);
    m_replyBox->setTitle("GraphQL reply");

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

    v_splitter->addWidget(m_replyBox);
}
void UG_JSONViewPage::initQueryBox()
{
    QGroupBox *req_box = new QGroupBox("Graph query data", this);
    m_reqEdit = new QLineEdit(req_box);
    if (req_box->layout()) delete req_box->layout();
    req_box->setLayout(new QVBoxLayout(0));
    req_box->layout()->addWidget(m_reqEdit);
    req_box->layout()->setMargin(2);

    v_splitter->addWidget(req_box);
}
void UG_JSONViewPage::clearPage()
{
    m_replyBox->clearView();
}
void UG_JSONViewPage::cutPrecision(QTreeWidgetItem *item)
{
    int n = item->childCount();
    int dot_pos = -1;
    if (n > 0)
    {
        for (int i=0; i<n; i++)
        {
            QTreeWidgetItem *child_item = item->child(i);
            if (child_item->text(2).trimmed().toLower() == "string")
            {
                QString text0(child_item->text(0).trimmed().toLower());
                QString text1(child_item->text(1).trimmed());
                if (text1.length() > 15 && text1.contains("."))
                {
                    dot_pos = text1.indexOf(".");
                    if (dot_pos > 2)
                    {
                        if (m_precision == 0) text1 = text1.left(dot_pos);
                        else text1 = text1.left(dot_pos+1+m_precision);
                        child_item->setText(1, text1);
                    }
                }
            }
            else cutPrecision(child_item);
        }
    }
}
void UG_JSONViewPage::load(QSettings &settings)
{
    UG_BasePage::load(settings);
    m_reqEdit->setText(settings.value(QString("%1/query_data").arg(objectName()), QString()).toString());
}
void UG_JSONViewPage::save(QSettings &settings)
{
    UG_BasePage::save(settings);
    settings.setValue(QString("%1/query_data").arg(objectName()), m_reqEdit->text());
}





