#include "fxqualdatapage.h"

#include <QSplitter>

//FXQualDataPage
FXQualDataPage::FXQualDataPage(QWidget *parent)
    :LSimpleWidget(parent, 22),
      m_tableBox(NULL),
      m_listBox(NULL)
{
    setObjectName("fx_qualdata_page");

    initWidgets();

}
void FXQualDataPage::initWidgets()
{
    m_tableBox = new LTableWidgetBox(this);
    m_tableBox->setTitle("Time statistic");
    QStringList headers;
    headers << "Open time" << "Week day" << "Prev. days" << "Price";
    m_tableBox->setHeaderLabels(headers);

    m_listBox = new LListWidgetBox(this);
    m_listBox->setTitle("Integrated result");

    h_splitter->addWidget(m_tableBox);
    h_splitter->addWidget(m_listBox);
}
