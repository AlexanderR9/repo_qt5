#include "fxtestpage.h"
#include "ltable.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QSplitter>



//FXTestPage
FXTestPage::FXTestPage(QWidget *parent)
    :LSimpleWidget(parent, 31),
    m_inputWidget(NULL),
    m_resultWidget(NULL),
    m_historyWidget(NULL)
{
    setObjectName("fx_test_page");

    initWidgets();

}
void FXTestPage::initWidgets()
{
    m_inputWidget = new FXInputParamsWidget(this);
    m_resultWidget = new LTableWidgetBox(this);
    m_resultWidget->setTitle("Results of test");
    m_historyWidget = new LTableWidgetBox(this);
    m_historyWidget->setTitle("History steps");


    h_splitter->addWidget(m_inputWidget);
    h_splitter->addWidget(m_historyWidget);
    v_splitter->addWidget(m_resultWidget);
}



//FXInputParamsWidget
FXInputParamsWidget::FXInputParamsWidget(QWidget *parent)
    :LSimpleWidget(parent, 10),
      m_tableBox(NULL),
      m_testTypeBox(NULL),
      m_yearStartBox(NULL),
      m_yearEndBox(NULL)
{
    setObjectName("fx_input_params_widget");

    initWidgets();
    setSpacing(0);
}
void FXInputParamsWidget::initWidgets()
{
    QGroupBox *settings_box = new QGroupBox("Test settings", this);
    if (settings_box->layout()) delete settings_box->layout();
    QGridLayout *g_lay = new QGridLayout(0);
    settings_box->setLayout(g_lay);
    v_splitter->addWidget(settings_box);

    //fill layout
    g_lay->addWidget(new QLabel("Test type", this), 0, 0, 1, 1, Qt::AlignRight);
    m_testTypeBox = new QComboBox(this);
    g_lay->addWidget(m_testTypeBox, 0, 1, 1, 3);
    g_lay->addWidget(new QLabel("Start year", this), 1, 0, 1, 1, Qt::AlignRight);
    m_yearStartBox = new QComboBox(this);
    g_lay->addWidget(m_yearStartBox, 1, 1);
    g_lay->addWidget(new QLabel("End year", this), 1, 2, 1, 1, Qt::AlignRight);
    m_yearEndBox = new QComboBox(this);
    g_lay->addWidget(m_yearEndBox, 1, 3);

    //create input table
    m_tableBox = new LTableWidgetBox(this);
    m_tableBox->vHeaderHide();
    m_tableBox->setTitle("Input parameters");
    QStringList headers;
    headers << "Parameter name" << "Value";
    m_tableBox->setHeaderLabels(headers);
    v_splitter->addWidget(m_tableBox);

    LTable::resizeTableContents(m_tableBox->table());
    layout()->setSpacing(0);
    //v_splitter->layout()->setSpacing(0);
}


