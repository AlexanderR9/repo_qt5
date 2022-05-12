#include "mainwidget.h"
#include "dsnsfilestruct.h"

#include <QDebug>
#include <QSettings>
#include <QString>
#include <QTimer>


MainWidget::MainWidget(QWidget *parent)
    :QWidget(parent),
      m_reader(0),
      m_timer(NULL)
{
    setupUi(this);
    resize(800, 400);
    move(500, 100);

    dirLineEdit->setText("/home/roman/shara/tmp/emul_smb_blk3");
    intervalComboBox->setCurrentIndex(3);

    m_reader = new DSNSFileStruct(this);
    m_timer = new QTimer(this);

    connect(readButton, SIGNAL(clicked()), this, SLOT(slotRead()));
    connect(startButton, SIGNAL(clicked()), this, SLOT(slotStart()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(slotStop()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

    slotStop();
}
void MainWidget::addProtocol(QString s, bool isErr)
{
    QString str;
    if (isErr) str="Error: ";
    str += s;
    protocolTextEdit->append(str);
}
void MainWidget::slotRead()
{
    protocolTextEdit->clear();
    addProtocol("reading............");
    m_reader->tryReadComplectFiles();
}
void MainWidget::slotStart()
{
    m_reader->setEmulDir(dirLineEdit->text());
    QString err = m_reader->checkEmulDir();
    if (!err.isEmpty())
    {
        addProtocol(err, true);
        return;
    }

    m_reader->tryGenFiles();
    stopButton->setEnabled(true);
    startButton->setEnabled(false);
    addProtocol("Emulation timer started.");

    m_timer->setInterval(intervalComboBox->currentText().toInt());
    m_timer->start();
}
void MainWidget::slotStop()
{
    addProtocol("Emulation timer stoped.");
    stopButton->setEnabled(false);
    startButton->setEnabled(true);
    m_timer->stop();
}
void MainWidget::slotTimer()
{
    addProtocol("tick");
    m_reader->nextEmulValues();
}




