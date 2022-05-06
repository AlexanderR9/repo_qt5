#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"




// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_protocol(NULL)
{
    setObjectName("main_form_tg");

}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atSave);
    addAction(LMainWidget::atLoadData);
    addAction(LMainWidget::atChain);
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
        //case LMainWidget::atLoadData: {loadHtmlFile(); break;}
        //case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        //case LMainWidget::atChain: {parseHtml(); break;}

        default: break;
    }
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
    
    QString key = QString("url");
    lCommonSettings.addParam(QString("URL page (example: https://ya.ru)"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString("https://yandex.ru"));

    key = QString("savefile");
    lCommonSettings.addParam(QString("Save HTML file (example: data.html)"), LSimpleDialog::sdtFilePath, key);
    lCommonSettings.setDefValue(key, QString("html.txt"));
}



