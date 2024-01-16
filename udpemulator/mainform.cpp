#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lfile.h"
#include "lstatic.h"
#include "lfile.h"
#include "lstring.h"
#include "ltime.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QApplication>
#include <QTimer>
#include <QUdpSocket>
#include <QTest>
#include <QNetworkDatagram>



// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    m_timer(NULL),
    udp_socket(NULL),
    m_mode(emServer),
    m_counter(101)
{
    setObjectName("main_form_udpemulator");
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

    udp_socket = new QUdpSocket(this);
}
void MainForm::slotTimer()
{
    if (isServer())
    {
        sendPack();
    }
    else {} //to do
}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atSendMsg);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {start(); break;}
        case LMainWidget::atStop: {stop(); break;}
        case LMainWidget::atSendMsg: {sendPack(); break;}
        default: break;
    }
}
void MainForm::initWidgets()
{
    m_protocol = new LProtocolBox(false, this);
    addWidget(m_protocol, 0, 0, 1, 1);
    updateButtonsState();
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=0; i<=5; i++)
        combo_list.append(QString::number(i));
    
    QString key = QString("host");
    lCommonSettings.addParam(QString("Host"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString("127.0.0.1"));

    key = QString("port");
    lCommonSettings.addParam(QString("Port"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, QString("12345"));

    key = QString("timer");
    lCommonSettings.addParam(QString("Send packets period, ms"), LSimpleDialog::sdtIntLine, key);
    lCommonSettings.setDefValue(key, QString("1000"));

    key = QString("mode");
    lCommonSettings.addParam(QString("Emulator mode"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "Server" << "Client";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("byteorder");
    lCommonSettings.addParam(QString("DataStream bytes order"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "BigEndian" << "LitleEndian";
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, QString("LitleEndian"));

    key = QString("pack");
    lCommonSettings.addParam(QString("Test packet"), LSimpleDialog::sdtFilePath, key);

    key = QString("outfile");
    lCommonSettings.addParam(QString("Out file for writing received packets"), LSimpleDialog::sdtFilePath, key);
}
void MainForm::save()
{
    LMainWidget::save();
}
void MainForm::load()
{
    LMainWidget::load();
    updateStatusWidget();
    m_mode = ((lCommonSettings.paramValue("mode").toString().trimmed().toLower() == "client") ? emClient : emServer);
}
void MainForm::updateStatusWidget()
{
    slotMsg(QString("current mode: %1").arg(isServer()?"SERVER":"CLIENT"));
}
void MainForm::updateButtonsState()
{
    bool started = this->started();
    getAction(atStart)->setEnabled(!started);
    getAction(atStop)->setEnabled(started);
    getAction(atSendMsg)->setEnabled(started && isClient());
    getAction(atSettings)->setEnabled(!started);
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMsg(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}
void MainForm::preparePacket()
{
    m_parts.clear();
    m_testPacket.clear();
    QString fname(packFile().trimmed());
    quint16 part_size = 0;
    if (!fname.isEmpty())
    {
        QStringList list;
        QString err = LFile::readFileSL(fname, list);
        if (err.isEmpty())
        {
            QString s;
            foreach (const QString &v, list)
            {
                if (v.contains("#")) continue;
                if (!v.contains(":")) continue;                

                if (v.trimmed() == "BREAK:")
                {
                    if (part_size > 0) m_parts.append(part_size);
                    //qDebug()<<QString("find part %1 bytes").arg(part_size);
                    part_size = 0;
                    continue;
                }

                part_size += (LString::subCount(v, ":") + 1);

                if (s.isEmpty()) s = v.trimmed();
                else s = QString("%1:%2").arg(s).arg(v.trimmed());
            }
            qDebug()<<s;
            if (!m_parts.isEmpty() && part_size > 0) m_parts.append(part_size);

            list.clear();
            list = LString::trimSplitList(s, ":", false);

            bool ok;
            slotMsg(QString("prepared file packet, size %1").arg(list.count()));
            foreach (const QString &v, list)
            {
                 if (v.length() != 2) continue;
                 char ch = v.toInt(&ok, 16);
                 if (ok) m_testPacket.append(ch);
            }

            if (!m_parts.isEmpty())
                foreach (quint16 v, m_parts)
                    qDebug()<<QString("find part %1 bytes").arg(v);

        }
        else slotError(err);
    }

    if (m_testPacket.isEmpty()) //need test bytes
    {
        QDataStream stream(&m_testPacket, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::ByteOrder(byteOrder()));
        for (quint8 i=0; i<8; i++) stream << i;

        slotMsg(QString("prepared test packet, size %1").arg(m_testPacket.count()));
    }
    slotMsg(LStatic::baToStr(m_testPacket, 16));

}
void MainForm::start()
{
    if (isServer())
    {

    }
    else
    {
        udp_socket->bind(port());
        connect(udp_socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    }

    preparePacket();

    m_protocol->addSpace();
    if (isServer()) m_protocol->addText("UDP Server started", LProtocolBox::ttData);
    else m_protocol->addText("UDP Client started", LProtocolBox::ttData);

    m_timer->setInterval(timerPeriod());
    m_timer->start();
    updateStatusWidget();
    updateButtonsState();
}
void MainForm::stop()
{
    m_timer->stop();
    m_protocol->addSpace();
    if (isServer()) m_protocol->addText("UDP Server stoped", LProtocolBox::ttData);
    else m_protocol->addText("UDP Client stoped", LProtocolBox::ttData);

    udp_socket->flush();
    udp_socket->close();
    disconnect(udp_socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));

    QTest::qWait(200);
    updateStatusWidget();
    updateButtonsState();
}
bool MainForm::started() const
{
    return m_timer->isActive();
}
void MainForm::slotAppSettingsChanged(QStringList keys)
{
    LMainWidget::slotAppSettingsChanged(keys);
    foreach (QString key, keys)
    {
        if (key == "mode")
        {
            m_mode = ((lCommonSettings.paramValue("mode").toString().trimmed().toLower() == "client") ? emClient : emServer);
            updateStatusWidget();
        }
    }
}
int MainForm::byteOrder() const
{
    if (lCommonSettings.paramValue("byteorder").toString().toLower().contains("big")) return QDataStream::BigEndian;
    return QDataStream::LittleEndian;
}
int MainForm::timerPeriod() const
{
    return lCommonSettings.paramValue("timer").toString().toInt();
}
void MainForm::sendPack()
{
    qDebug("MainForm::sendPack()");
    QHostAddress h(host());
    qint64 n = -1;
    if (m_parts.isEmpty())
    {
        n = udp_socket->writeDatagram(m_testPacket, h, port());
        slotMsg(QString("sended datagram, size %1").arg(n));
    }
    else
    {
        QByteArray ba(m_testPacket);
        foreach (quint16 v, m_parts)
        {
            n = udp_socket->writeDatagram(ba.left(v), h, port());
            slotMsg(QString("sended part datagram, size %1").arg(n));
            ba.remove(0, v);
            QTest::qWait(50);
        }
    }
}
QString MainForm::host() const
{
    return lCommonSettings.paramValue("host").toString();
}
QString MainForm::packFile() const
{
    return lCommonSettings.paramValue("pack").toString();
}
QString MainForm::outFile() const
{
    return lCommonSettings.paramValue("outfile").toString();
}
quint16 MainForm::port() const
{
    return lCommonSettings.paramValue("port").toString().toUInt();
}
void MainForm::slotReadyRead()
{
    qint64 n_buff = udp_socket->pendingDatagramSize();
    slotMsg(QString("received datagram, size %1").arg(n_buff));
    if (n_buff > 0)
    {
        QNetworkDatagram dg(udp_socket->receiveDatagram());
        QByteArray ba(dg.data());
        slotMsg(LStatic::baToStr(ba));
        writeOutFile(ba);
    }
}
void MainForm::writeOutFile(const QByteArray &ba)
{
    QString fname(outFile().trimmed());
    if (fname.isEmpty()) return;

    QString s = QString("Received UDP data (%1)").arg(LTime::strCurrentDateTime());
    s = QString("%1\n%2").arg(s).arg(LStatic::baToStr(ba, 24));
    s = QString("%1\n%2\n\n").arg(s).arg(QString(50, '-'));
    QString err = LFile::appendFile(fname, s);
    if (!err.isEmpty()) slotError(err);
}

