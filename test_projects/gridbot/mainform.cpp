#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "gbotpage.h"


#include <QSplitter>


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    m_botPage(NULL),
    v_splitter(NULL)
{
    setObjectName("main_form_gbot");


}
void MainForm::init()
{
    LMainWidget::init();
    //loadConfig();
}
void MainForm::initActions()
{
    //addAction(LMainWidget::atStart);
    addAction(LMainWidget::atRefresh);
    //addAction(LMainWidget::atLoadData);
    //addAction(LMainWidget::atData);
    addAction(LMainWidget::atClear);
    //addAction(LMainWidget::atSendMsg);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atRefresh: {m_botPage->exec(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        default: break;
    }
}

void MainForm::initWidgets()
{
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    m_botPage = new GBotPage(this);
    m_protocol = new LProtocolBox(false, this);
    v_splitter->addWidget(m_botPage);
    v_splitter->addWidget(m_protocol);

    connect(m_botPage, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_botPage, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));

}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=0; i<=5; i++)
        combo_list.append(QString::number(i));
    
    QString key = QString("config");
    lCommonSettings.addParam(QString("Config file"), LSimpleDialog::sdtFilePath, key);
    lCommonSettings.setDefValue(key, QString(""));
}
void MainForm::slotMessage(const QString &text)
{
    m_protocol->addText(text);
}
void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::load()
{
    LMainWidget::load();
    QSettings settings(companyName(), projectName());
    QByteArray vba = settings.value(QString("%1/v_spltitter_state").arg(objectName()), QByteArray()).toByteArray();
    if (v_splitter && !vba.isEmpty()) v_splitter->restoreState(vba);

    m_botPage->load(settings);
}
void MainForm::save()
{
    LMainWidget::save();
    QSettings settings(companyName(), projectName());
    if (v_splitter) settings.setValue(QString("%1/v_spltitter_state").arg(objectName()), v_splitter->saveState());


    m_botPage->save(settings);
}



