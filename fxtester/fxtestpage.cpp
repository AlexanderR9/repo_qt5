#include "fxtestpage.h"
#include "ltable.h"
#include "fxdataloader.h"
#include "fxbarcontainer.h"
#include "fxenums.h"
#include "fxtesterobj.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QSplitter>
#include <QDebug>
#include <QList>
#include <QLineEdit>
#include <QSettings>
#include <QTableWidget>
#include <QHeaderView>



//FXTestPage
FXTestPage::FXTestPage(QWidget *parent)
    :LSimpleWidget(parent, 31),
    m_inputWidget(NULL),
    m_resultWidget(NULL),
    m_historyWidget(NULL),
    m_tester(NULL)
{
    setObjectName("fx_test_page");

    initWidgets();
    initResultsTable();

    m_tester = new FXTesterObj(this);
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
void FXTestPage::initResultsTable()
{
    QStringList headers;
    headers << "Couple" << "Sum";
    m_resultWidget->setHeaderLabels(headers);
    m_resultWidget->vHeaderHide();
    LTable::resizeTableContents(m_resultWidget->table());
}
void FXTestPage::updateLoadedData(const FXDataLoader *loader)
{
    LTable::removeAllRowsTable(m_resultWidget->table());

    int y_min = -1;
    int y_max = -1;
    if (loader)
    {
        int n = loader->count();
        for (int i=0; i<n; i++)
        {
            const FXBarContainer *cnt = loader->containerAt(i);
            if (!cnt) continue;

            if (y_min < 0)
            {
                y_min = cnt->firstTime().date().year();
                y_max = cnt->lastTime().date().year();
            }
            else
            {
                if (y_min > cnt->firstTime().date().year()) y_min = cnt->firstTime().date().year();
                if (y_max < cnt->lastTime().date().year()) y_max = cnt->lastTime().date().year();
            }

            addDataContainer(cnt);
        }
    }
    m_inputWidget->setYearsRange(y_min, y_max);
    LTable::resizeTableContents(m_resultWidget->table());
    reinitTests(loader);
}
void FXTestPage::reinitTests(const FXDataLoader *loader)
{
    m_tester->reset();
    if (!loader) return;

    QList<int> t_list = FXEnumStaticObj::testTypes();
    for (int i=0; i<t_list.count(); i++)
    {
        int t = t_list.at(i);
        int n = loader->count();
        for (int j=0; j<n; j++)
        {
            const FXBarContainer *cnt = loader->containerAt(i);
            if (cnt) m_tester->addTest(t, cnt);
        }
    }
}
void FXTestPage::addDataContainer(const FXBarContainer *cnt)
{
    QStringList list;
    list << QString("%1 (%2)").arg(cnt->couple()).arg(cnt->timeframe());
    int cols = m_resultWidget->table()->columnCount();
    for (int i=1; i<cols; i++)
        list.append(QString("-"));

    LTable::addTableRow(m_resultWidget->table(), list);
}
void FXTestPage::load(QSettings &settings)
{
    LSimpleWidget::load(settings);
    m_inputWidget->load(settings);
}
void FXTestPage::save(QSettings &settings)
{
    LSimpleWidget::save(settings);
    m_inputWidget->save(settings);
}


//FXInputParamsWidget
FXInputParamsWidget::FXInputParamsWidget(QWidget *parent)
    :LSimpleWidget(parent, 11),
      m_tableBox(NULL),
      m_testTypeBox(NULL),
      m_yearStartBox(NULL),
      m_yearEndBox(NULL)
{
    setObjectName("fx_input_params_widget");

    initWidgets();
    setSpacing(0);

    fillTestsBox();
}
void FXInputParamsWidget::initWidgets()
{
    QGroupBox *settings_box = new QGroupBox("Test settings", this);
    if (settings_box->layout()) delete settings_box->layout();
    QGridLayout *g_lay = new QGridLayout(0);
    settings_box->setLayout(g_lay);

    //fill settings_box layout
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
    LTable::resizeTableContents(m_tableBox->table());

    //add to v_splitter
    v_splitter->addWidget(settings_box);
    v_splitter->addWidget(m_tableBox);
}
int FXInputParamsWidget::currentTest() const
{
    if (m_testTypeBox->count() > 0)
        return m_testTypeBox->currentData().toInt();
    return -1;
}
void FXInputParamsWidget::setYearsRange(int y_min, int y_max)
{
    qDebug()<<QString("FXInputParamsWidget::setYearsRange   y_min=%1  y_max=%2").arg(y_min).arg(y_max);
    m_yearStartBox->clear();
    m_yearEndBox->clear();
    if (y_min < 2000 || y_max < 2000 || y_min > y_max) return;


    for (int i=y_min; i <= y_max; i++)
    {
        m_yearStartBox->addItem(QString::number(i));
        m_yearEndBox->addItem(QString::number(i));
    }

    m_yearStartBox->setCurrentIndex(0);
    m_yearEndBox->setCurrentIndex(m_yearEndBox->count()-1);
}
void FXInputParamsWidget::fillTestsBox()
{
    m_testTypeBox->clear();
    QList<int> t_list = FXEnumStaticObj::testTypes();
    for (int i=0; i<t_list.count(); i++)
    {
        int t = t_list.at(i);
        QString t_name = FXEnumStaticObj::testShortName(t);
        QString t_desc = FXEnumStaticObj::testDesc(t);
        m_testTypeBox->addItem(t_name);
        m_testTypeBox->setItemData(i, t, Qt::UserRole);
        m_testTypeBox->setItemData(i, t_desc, Qt::ToolTipRole);
    }
}
void FXInputParamsWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);
    int pos = settings.value(QString("%1/test_index").arg(objectName()), 0).toInt();
    if (pos >= 0 && m_testTypeBox->count() > 0) m_testTypeBox->setCurrentIndex(pos);
}
void FXInputParamsWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);
    settings.setValue(QString("%1/test_index").arg(objectName()), m_testTypeBox->currentIndex());
}

