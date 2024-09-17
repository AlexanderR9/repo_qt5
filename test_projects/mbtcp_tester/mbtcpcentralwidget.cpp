#include "mbtcpcentralwidget.h"
#include "reqrespwidget.h"
#include "exchangestatewidget.h"
#include "ltable.h"

#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QSettings>
#include <QModbusRequest>




// MBTcpCentralWidget
MBTcpCentralWidget::MBTcpCentralWidget(QWidget *parent)
    :LSimpleWidget(parent, 32),
      m_slaveWidget(NULL),
      m_masterWidget(NULL),
      m_reqRespWidget(NULL),
      m_stateWidget(NULL),
      //m_mode(0),
      m_mainWidget(NULL)
{
    setObjectName(QString("mbtcp_central_widget"));

    m_reqRespWidget = new ReqRespWidget(this);
    m_stateWidget = new ExchangeStateWidget(this);

    m_mainWidget = new QWidget(this);
    if (m_mainWidget->layout()) delete m_mainWidget->layout();
    m_mainWidget->setLayout(new QVBoxLayout(0));
    m_mainWidget->layout()->setMargin(0);
    m_mainWidget->layout()->setSpacing(0);

    v_splitter->addWidget(m_reqRespWidget);
    v_splitter->addWidget(m_stateWidget);
    h_splitter->addWidget(m_mainWidget);

    //reinitWidget(0);
}
void MBTcpCentralWidget::resetMainWidget()
{
    if (m_slaveWidget)
    {
        m_mainWidget->layout()->removeWidget(m_slaveWidget);
        m_slaveWidget->deleteLater();
        m_slaveWidget = NULL;
    }
    if (m_masterWidget)
    {
        m_mainWidget->layout()->removeWidget(m_masterWidget);
        m_slaveWidget->deleteLater();
        m_slaveWidget = NULL;
    }
}
void MBTcpCentralWidget::reinitWidget(int mode)
{
    resetMainWidget();
    if (mode == 0)
    {
        m_masterWidget = new MasterWidget(this);
        m_mainWidget->layout()->addWidget(m_masterWidget);
    }
    else
    {
        m_slaveWidget = new SlaveWidget(this);
        m_mainWidget->layout()->addWidget(m_slaveWidget);
    }
}
void MBTcpCentralWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);
    if (m_masterWidget) m_masterWidget->load(settings);

}
void MBTcpCentralWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);
    if (m_masterWidget) m_masterWidget->save(settings);

}
void MBTcpCentralWidget::slotUpdateState(const QStringList &data)
{
    if (m_stateWidget)
        m_stateWidget->updateStatistic(data);
}
void MBTcpCentralWidget::slotFillReq(QModbusRequest &req, quint8 &dev_addr, QString &err)
{
    if (m_masterWidget)
        m_masterWidget->fillReq(req, dev_addr, err);
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

// MasterWidget
MasterWidget::MasterWidget(QWidget *parent)
    :LSimpleWidget(parent, 20)
{
    setObjectName(QString("mbtcp_master_widget"));

    init();

    connect(m_regsLenEdit, SIGNAL(textChanged(QString)), this, SLOT(slotRegsCountChanged()));
}
void MasterWidget::fillReq(QModbusRequest &req, quint8 &dev_addr, QString &err)
{
    bool ok;
    err.clear();
    uint a = m_transactionIdEdit->text().toUInt(&ok);
    if (!ok) a = 99;

    a = m_funcCodeEdit->text().toUInt(&ok, 16);
    if (!ok) {err=QString("invalid function %1").arg(m_funcCodeEdit->text()); return;}
    req.setFunctionCode(QModbusPdu::FunctionCode(a));

    dev_addr = m_devAddrEdit->text().toUInt(&ok);
    if (!ok) {err=QString("invalid device address %1").arg(m_devAddrEdit->text()); return;}

    quint16 start_reg = m_startRegEdit->text().toUInt(&ok);
    if (!ok) {err=QString("invalid start reg address %1").arg(m_startRegEdit->text()); return;}
    quint16 reg_count = m_regsLenEdit->text().toUInt(&ok);
    if (!ok) {err=QString("invalid regs count %1").arg(m_regsLenEdit->text()); return;}

    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << start_reg << reg_count;
    req.setData(ba);
}
void MasterWidget::slotRegsCountChanged()
{
    QString s = m_regsLenEdit->text().trimmed();
    if (s.isEmpty()) return;

    m_regTable->removeAllRows();

    bool ok;
    quint32 start_reg = m_startRegEdit->text().toUInt(&ok);
    if (!ok) return;
    quint32 n = s.toUInt(&ok);
    if (!ok || n == 0) return;

    for (quint32 i=0; i<n; i++)
    {
        QStringList list;
        list << QString::number(i+start_reg) << "0";
        LTable::addTableRow(m_regTable->table(), list);
    }
    m_regTable->resizeByContents();
}
void MasterWidget::init()
{
    QGroupBox *box = new QGroupBox("Request parameters", this);
    if (box->layout()) delete box->layout();
    QGridLayout *g_lay = new QGridLayout(0);
    box->setLayout(g_lay);
    addBoxLineEdit(g_lay);

    m_regTable = new LTableWidgetBox(this);
    m_regTable->setTitle("Registers values");
    QStringList headers;
    headers << "Address" << "Value";
    m_regTable->setHeaderLabels(headers, Qt::Horizontal);
    m_regTable->removeAllRows();
    m_regTable->vHeaderHide();
    m_regTable->resizeByContents();

    h_splitter->addWidget(box);
    h_splitter->addWidget(m_regTable);

}
void MasterWidget::addBoxLineEdit(QGridLayout *g_lay)
{
    QIntValidator *iv = new QIntValidator(this);

    int i = 0;
    g_lay->addWidget(new QLabel("Transaction ID"), i, 0);
    m_transactionIdEdit = new QLineEdit(this);
    m_transactionIdEdit->setValidator(iv);
    g_lay->addWidget(m_transactionIdEdit, i, 1);
    i++;

    g_lay->addWidget(new QLabel("Device address"), i, 0);
    m_devAddrEdit = new QLineEdit(this);
    m_devAddrEdit->setValidator(iv);
    g_lay->addWidget(m_devAddrEdit, i, 1);
    i++;

    g_lay->addWidget(new QLabel("Function code"), i, 0);
    m_funcCodeEdit = new QLineEdit(this);
    //m_funcCodeEdit->setValidator(iv);
    g_lay->addWidget(m_funcCodeEdit, i, 1);
    i++;

    g_lay->addWidget(new QLabel("First register"), i, 0);
    m_startRegEdit = new QLineEdit(this);
    m_startRegEdit->setValidator(iv);
    g_lay->addWidget(m_startRegEdit, i, 1);
    i++;

    g_lay->addWidget(new QLabel("Registers count"), i, 0);
    m_regsLenEdit = new QLineEdit(this);
    m_regsLenEdit->setValidator(iv);
    g_lay->addWidget(m_regsLenEdit, i, 1);
    i++;

    g_lay->addItem(new QSpacerItem(10, 300), i, 0);
}
void MasterWidget::load(QSettings &settings)
{
    LSimpleWidget::load(settings);
    m_transactionIdEdit->setText(settings.value(QString("%1/transaction_id").arg(objectName()), "555").toString().trimmed());
    m_devAddrEdit->setText(settings.value(QString("%1/dev_addr").arg(objectName()), "1").toString().trimmed());
    m_funcCodeEdit->setText(settings.value(QString("%1/func_code").arg(objectName()), "0x03").toString().trimmed());
    m_startRegEdit->setText(settings.value(QString("%1/start_reg").arg(objectName()), "10").toString().trimmed());
    m_regsLenEdit->setText(settings.value(QString("%1/regs_len").arg(objectName()), "4").toString().trimmed());

}
void MasterWidget::save(QSettings &settings)
{
    LSimpleWidget::save(settings);
    settings.setValue(QString("%1/transaction_id").arg(objectName()), m_transactionIdEdit->text());
    settings.setValue(QString("%1/dev_addr").arg(objectName()), m_devAddrEdit->text());
    settings.setValue(QString("%1/func_code").arg(objectName()), m_funcCodeEdit->text());
    settings.setValue(QString("%1/start_reg").arg(objectName()), m_startRegEdit->text());
    settings.setValue(QString("%1/regs_len").arg(objectName()), m_regsLenEdit->text());

}



