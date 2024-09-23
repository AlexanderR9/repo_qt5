#include "mbtcpcentralwidget.h"
#include "reqrespwidget.h"
#include "exchangestatewidget.h"
#include "ltable.h"

#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDebug>
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
void MBTcpCentralWidget::slotUpdateReqRespTable(const QStringList &req_list, const QStringList &resp_list)
{
    if (m_reqRespWidget)
        m_reqRespWidget->updateTable(req_list, resp_list);
}
void MBTcpCentralWidget::slotUpdateRegTable(const QModbusResponse &resp)
{
    if (m_masterWidget)
        m_masterWidget->updateRegValueByResponse(resp);
}



//RegDataTable
RegDataTable::RegDataTable(QWidget *parent)
    :LTableWidgetBox(parent, 1),
      m_startReg(0)
{
    this->setTitle("");
    this->setFlat(true);
    QStringList headers;
    for (int i=0; i<10; i++)
        headers << QString::number(i+1);
    setHeaderLabels(headers, Qt::Horizontal);

    setSelectionMode(QAbstractItemView::SelectRows, QAbstractItemView::NoSelection);
    resizeByContents();

    initBitSwithers();
}
quint16 RegDataTable::regCount() const
{
    if (!m_table) return 0;
    return m_table->rowCount();
}
void RegDataTable::regsCountChanged(quint32 start_addr, quint32 n_regs)
{
    m_startReg = start_addr;
    for (quint32 i=0; i<n_regs; i++)
    {
        QStringList list;
        list << QString::number(i+start_addr) << "0";
        LTable::addTableRow(m_table, list);
    }

    setPropertyItems();
    resizeByContents();
}
void RegDataTable::setPropertyItems()
{
    for (int i=0; i<m_table->rowCount(); i++)
    {
        m_table->item(i, 0)->setFlags(Qt::ItemIsEnabled);
        m_table->item(i, 0)->setTextAlignment(Qt::AlignCenter);
        m_table->item(i, 1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        m_table->item(i, 1)->setTextAlignment(Qt::AlignCenter);
    }
}
void RegDataTable::fillRegValues(QDataStream &stream, quint8 f_code, quint16 start_reg)
{
    switch (f_code)
    {
        case QModbusPdu::WriteSingleCoil:
        {
            quint16 reg_value = 0xFF00;
            if (valueAt(start_reg) == 0) reg_value = 0;
            stream << reg_value;
            break;
        }
        case QModbusPdu::WriteMultipleCoils:
        {
            QList<quint8> req_reg_values;
            quint16 len = regCount();
            stream << len;

            //запихиваем полные байты по 8 бит
            quint8 v_byte = 0;
            while (len > 8)
            {
                v_byte = 0;
                for (int i=0; i<8; i++)
                {
                    if (valueAt(i+start_reg) > 0)
                        v_byte |= m_bitSwithers.at(i);
                }
                req_reg_values.append(v_byte);
                len -= 8;
                start_reg += 8;
            }
            //запихиваем оставшиеся биты последнего байта
            if (len > 0)
            {
                v_byte = 0;
                for (int i=0; i<len; i++)
                {
                    if (valueAt(i+start_reg) > 0)
                        v_byte |= m_bitSwithers.at(i);
                }
                req_reg_values.append(v_byte);
            }
            stream << quint8(req_reg_values.count());
            foreach (quint8 v, req_reg_values) stream << v;

            break;
        }
        case QModbusPdu::WriteSingleRegister:
        {
            stream << valueAt(start_reg);
            break;
        }
        case QModbusPdu::WriteMultipleRegisters:
        {
            quint16 n_reg = regCount();
            stream << n_reg << quint8(2*n_reg);
            for (quint16 i=0; i<n_reg; i++)
                stream << valueAt(start_reg+i);
            break;
        }
        default: break;
    }
}
quint16 RegDataTable::valueAt(quint16 addr) const
{
    quint16 reg_value = 0;
    int i = addr - m_startReg;
    if (i<0 || i>=m_table->rowCount())
    {
        qWarning()<<QString("WARNING RegDataTable::valueAt  invalid row index %1/%2").arg(i).arg(addr);
        return reg_value;
    }
    bool ok;
    reg_value = m_table->item(i, 1)->text().toUInt(&ok);
    if (!ok)
    {
        reg_value = 0;
        qWarning()<<QString("WARNING RegDataTable::valueAt  invalid cell_value %1,  row %2/%3").arg(m_table->item(i, 1)->text()).arg(i).arg(addr);
    }
    return reg_value;
}
void RegDataTable::initBitSwithers()
{
    m_bitSwithers.clear();
    m_bitSwithers.append(0x01);
    m_bitSwithers.append(0x02);
    m_bitSwithers.append(0x04);
    m_bitSwithers.append(0x08);
    m_bitSwithers.append(0x10);
    m_bitSwithers.append(0x20);
    m_bitSwithers.append(0x40);
    m_bitSwithers.append(0x80);
}
void RegDataTable::updateRegValueByResponse(quint8 f_code, const QByteArray &ba)
{
    if (ba.isEmpty()) return;

    QDataStream stream(ba);
    stream.setByteOrder(QDataStream::BigEndian);


    switch (f_code)
    {
        case QModbusPdu::ReadCoils:
        case QModbusPdu::ReadDiscreteInputs:
        {
            quint8 v_byte = 0;
            int len = ba.size();
            int row = 0;
            for (int i=0; i<len; i++)
            {
                stream >> v_byte;
                if (row >= regCount()) break;

                for (int bit=0; bit<8; bit++)
                {
                    quint8 d_value = 0;
                    if (v_byte & m_bitSwithers.at(bit)) d_value = 1;
                    m_table->item(row, 1)->setText(QString::number(d_value));
                    row++;
                    if (row >= regCount()) break;
                }
            }
            break;
        }

        case QModbusPdu::ReadHoldingRegisters:
        case QModbusPdu::ReadInputRegisters:
        {
            for (int i=0; i<ba.size()/2; i++)
            {
                quint16 reg_value = 0;
                stream >> reg_value;
                m_table->item(i, 1)->setText(QString::number(reg_value));
            }
            break;
        }

        default: break;
    }
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
    :LSimpleWidget(parent, 20),
      m_regTable(NULL)
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
    //quint16 reg_count = m_regsLenEdit->text().toUInt(&ok);
    //if (!ok) {err=QString("invalid regs count %1").arg(m_regsLenEdit->text()); return;}

    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    fillReqData(stream, req.functionCode(), start_reg);
    req.setData(ba);
}
void MasterWidget::updateRegValueByResponse(const QModbusResponse &resp)
{
    if (!resp.isValid() || resp.isException()) return;

    switch (resp.functionCode())
    {
        case QModbusPdu::ReadCoils:
        case QModbusPdu::ReadDiscreteInputs:
        case QModbusPdu::ReadHoldingRegisters:
        case QModbusPdu::ReadInputRegisters:
        {
            int size = resp.dataSize();
            if (size > 1)
                m_regTable->updateRegValueByResponse(resp.functionCode(), resp.data().right(size-1));
            break;
        }

        default: break;
    }
}
void MasterWidget::fillReqData(QDataStream &stream, quint8 f_code, quint16 start_reg)
{
    switch (f_code)
    {
        case QModbusPdu::ReadCoils:
        case QModbusPdu::ReadDiscreteInputs:
        case QModbusPdu::ReadHoldingRegisters:
        case QModbusPdu::ReadInputRegisters:
        {
            stream << start_reg << m_regTable->regCount();
            break;
        }
        case QModbusPdu::WriteSingleCoil:
        case QModbusPdu::WriteSingleRegister:
        case QModbusPdu::WriteMultipleCoils:
        case QModbusPdu::WriteMultipleRegisters:
        {
            stream << start_reg;
            m_regTable->fillRegValues(stream, f_code, start_reg);
            break;
        }
        default: {emit signalError(QString("MasterWidget: invalid func (%1)").arg(f_code)); break;}
    }
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

    m_regTable->regsCountChanged(start_reg, n);
}
void MasterWidget::init()
{
    QGroupBox *box = new QGroupBox("Request parameters", this);
    if (box->layout()) delete box->layout();
    QGridLayout *g_lay = new QGridLayout(0);
    box->setLayout(g_lay);
    addBoxLineEdit(g_lay);

    m_regTable = new RegDataTable(this);
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
    m_transactionIdEdit->setEnabled(false);
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



