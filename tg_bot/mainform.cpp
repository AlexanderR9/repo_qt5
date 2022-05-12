#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "lbot.h"



// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL),
    l_bot(NULL)
{
    setObjectName("main_form_tg");

    l_bot = new LBot(this);
    connect(l_bot, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(l_bot, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));


}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    //addAction(LMainWidget::atSave);
    addAction(LMainWidget::atLoadData);
    //addAction(LMainWidget::atChain);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        //case LMainWidget::atStart: {startHtmlRequest(); break;}
        //case LMainWidget::atSave: {saveHtmlToFile(); break;}
        case LMainWidget::atLoadData: {loadConfig(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        //case LMainWidget::atChain: {parseHtml(); break;}

        default: break;
    }
}
void MainForm::loadConfig()
{
    m_protocol->addSpace();
    slotMessage("try load config ...");
    if (!l_bot) return;

    QString key = QString("config");
    l_bot->loadConfig(lCommonSettings.paramValue(key).toString().trimmed());

}
void MainForm::initWidgets()
{
    m_protocol = new LProtocolBox(false, this);
    addWidget(m_protocol, 0, 0);
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=0; i<=5; i++)
        combo_list.append(QString::number(i));
    
    //QString key = QString("url");
    //lCommonSettings.addParam(QString("URL page (example: https://ya.ru)"), LSimpleDialog::sdtString, key);
    //lCommonSettings.setDefValue(key, QString("https://yandex.ru"));

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



