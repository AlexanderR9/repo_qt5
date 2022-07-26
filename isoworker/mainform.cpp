#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "processobj.h"
#include "paramspage.h"
#include "lfile.h"
#include "lstatic.h"


#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QProgressBar>
#include <QGroupBox>

#define MAKE_ISO_COMMAND        QString("genisoimage")
#define MD5SUM_COMMAND          QString("md5sum")
#define MD5SUM_FILE             QString("md5_iso.txt")


// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_paramsPage(NULL),
    m_protocol(NULL),
    v_splitter(NULL),
    m_processObj(NULL),
    m_timer(NULL)
{
    m_processObj = new LProcessObj(this);
    m_processObj->setDebugLevel(1);
    connect(m_processObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_processObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));
    connect(m_processObj, SIGNAL(signalFinished()), this, SLOT(slotFinished()));
    connect(m_processObj, SIGNAL(signalReadyRead()), this, SLOT(slotReadyRead()));


    m_timer = new QTimer(this);
    m_timer->setInterval(770);
    m_timer->stop();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atClear);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::initWidgets()
{
    m_paramsPage = new ParamsPage(this);
    m_protocol = new LProtocolBox(false, this);
    v_splitter = new QSplitter(Qt::Vertical, this);
    addWidget(v_splitter, 0, 0);

    v_splitter->addWidget(m_paramsPage);
    v_splitter->addWidget(m_protocol);

    updateActionsEnable(true);
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("source");
    lCommonSettings.addParam(QString("Source dir"), LSimpleDialog::sdtDirPath, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("req_interval");
    lCommonSettings.addParam(QString("Request interval, sec"), LSimpleDialog::sdtIntCombo, key);
    for (int i=1; i<=20; i++) combo_list.append(QString::number(i*3));
    lCommonSettings.setComboList(key, combo_list);

    key = QString("log_max_size");
    lCommonSettings.addParam(QString("Log max size"), LSimpleDialog::sdtIntCombo, key);
    combo_list.clear();
    combo_list << "200" << "500" << "1000" << "2000" << "3000" << "5000";
    lCommonSettings.setComboList(key, combo_list);

}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {start(); break;}
        case LMainWidget::atStop: {stopBreak(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        default: break;
    }
}
void MainForm::slotReadyRead()
{
    qDebug("MainForm::slotReadyRead()");
}
void MainForm::slotFinished()
{
    qDebug("MainForm::slotFinished()");
    QString s_result = (m_processObj->isOk() ? "Ok" : "Fault");
    m_protocol->addText(QString("finished, result=[%1]").arg(s_result));
    //updateActionsEnable(true);

    QStringList debug_list(m_processObj->bufferList());
    qDebug("");
    for (int i=0; i<debug_list.count(); i++)
        qDebug()<<debug_list.at(i);

    checkProcessFinishedResult();
}
void MainForm::checkProcessFinishedResult()
{
    if (!m_processObj->isOk())
    {
        m_stage = isoNeedBreak;
        return;
    }

    if (!m_curISOFile.isEmpty())
    {
        m_stage = isoNeedCalcMD5;
        return;
    }

    if (m_processObj->command() == MD5SUM_COMMAND) parseMD5();


    if (!m_sourceDirISO.isEmpty()) m_stage = isoMakeNext;
    else m_stage = isoFinishedAll;
}
QString MainForm::sourcePath() const
{
    return lCommonSettings.paramValue("source").toString().trimmed();
}
void MainForm::parseMD5()
{
    if (m_processObj->argsEmpty())
    {
        qWarning("MainForm::parseMD5() WARNING m_processObj->argsEmpty()");
        return;
    }

    QString f_line = LFile::shortFileName(m_processObj->args().first());
    QString out = m_processObj->buffer().trimmed();
    QString crc_value;
    int pos = out.indexOf(m_processObj->args().first());
    if (pos < 0) crc_value = "INVALID";
    else crc_value = out.left(pos).trimmed();

    f_line = QString("%1     %2 \n\n").arg(f_line).arg(crc_value);
    m_protocol->addText(QString("append md5sum to file: %1").arg(MD5SUM_FILE));
    m_protocol->addText(QString("crc_value: %1").arg(crc_value));

    QString f_name = QString("%1%2%3").arg(sourcePath()).arg(QDir::separator()).arg(MD5SUM_FILE);
    QString err = LFile::appendFile(f_name, f_line);
    if (!err.isEmpty())
        slotError(err);

}
void MainForm::updateActionsEnable(bool stoped)
{
    getAction(LMainWidget::atStop)->setEnabled(!stoped);
    getAction(LMainWidget::atStart)->setEnabled(stoped);
    getAction(LMainWidget::atSettings)->setEnabled(stoped);
    getAction(LMainWidget::atExit)->setEnabled(stoped);
}
void MainForm::readParamsPage()
{
    m_processObj->setCommand(m_paramsPage->commandName());
    m_processObj->setArgs(m_paramsPage->getArgs());
    m_processObj->setSudo(m_paramsPage->isSudo());
}
void MainForm::start()
{
    m_curISOFile.clear();
    m_protocol->addSpace();
    updateActionsEnable(false);

    //readParamsPage();
    m_protocol->addText("Starting cenarii for maker ISO", LProtocolBox::ttOk);
    m_stage = isoStarting;
    //m_protocol->addText(QString("cmd=[%1]").arg(m_processObj->fullCommand()));
    //m_processObj->startCommand();

    prepareSourceDirISO();
    m_timer->start();
}
void MainForm::prepareSourceDirISO()
{
    m_sourceDirISO.clear();
    m_protocol->addText("preparing source folder list ........");

    QString err = LFile::dirFolders(sourcePath(), m_sourceDirISO);
    if (!err.isEmpty())
    {
        slotError(err);
        m_stage = isoNeedBreak;
        return;
    }

    m_protocol->addSpace();
    m_protocol->addText(QString("ISO source folders: %1").arg(m_sourceDirISO.count()), LProtocolBox::ttData);
    if (m_sourceDirISO.isEmpty())
    {
        m_protocol->addText("ISO source folders is empty", LProtocolBox::ttWarning);
        m_stage = isoNeedBreak;
        return;
    }

    for (int i=0; i<m_sourceDirISO.count(); i++)
       m_protocol->addText(QString("   %1.  %2").arg(i+1).arg(m_sourceDirISO.at(i)));

    m_protocol->addText("source folder list prepared!");
    m_stage = isoMakeNext;
}
void MainForm::slotTimer()
{
    if (m_processObj->isRunning())
    {
        qWarning("MainForm::slotTimer() WARING - m_processObj->isRunning()");
        return;
    }

    switch (m_stage)
    {
        case isoNeedBreak:
        {
            m_protocol->addText("cenarii breaked", LProtocolBox::ttWarning);
            m_timer->stop();
            updateActionsEnable(true);
            break;
        }
        case isoMakeNext:
        {
            makeISO();
            break;
        }
        case isoFinishedAll:
        {
            finishedAll();
            break;
        }
        case isoNeedCalcMD5:
        {
            calcMD5();
            break;
        }
        case isoProcessRun:
        {
            m_protocol->addText("process run ....");
            m_processObj->startCommand();
            break;
        }
        default:
        {
            slotError(QString("unknown cenarii stage %1").arg(m_stage));
            stopBreak();
            break;
        }
    }
}
void MainForm::finishedAll()
{
    m_protocol->addSpace();
    m_protocol->addText("/////////////// CENARII FINISHED//////////////////////", LProtocolBox::ttFile);
    m_timer->stop();
    updateActionsEnable(true);
}
void MainForm::calcMD5()
{
    m_protocol->addSpace();
    m_protocol->addText("[CALC MD5]");
    if (m_curISOFile.trimmed().isEmpty())
    {
        m_protocol->addText("Current ISO file is empty", LProtocolBox::ttWarning);
        m_stage = isoNeedBreak;
        return;
    }

    if (!LFile::fileExists(m_curISOFile))
    {
        m_protocol->addText(QString("Current ISO file [%1] not found").arg(m_curISOFile), LProtocolBox::ttErr);
        m_stage = isoNeedBreak;
        return;
    }
    m_protocol->addText(QString("target file: %1").arg(m_curISOFile));

    QStringList args;
    args.append(m_curISOFile);
    m_processObj->setCommand(MD5SUM_COMMAND);
    m_processObj->setArgs(args);
    printNextProcessCommand();

    m_curISOFile.clear();
    m_stage = isoProcessRun;
}
void MainForm::printNextProcessCommand()
{
    m_protocol->addText(QString("NEXT CMD:   %1").arg(m_processObj->fullCommand()));
}
void MainForm::makeISO()
{
    m_protocol->addSpace();
    m_protocol->addText("[MAKE NEXT ISO]");
    if (m_sourceDirISO.isEmpty())
    {
        m_protocol->addText("ISO source folders is empty", LProtocolBox::ttWarning);
        m_stage = isoNeedBreak;
        return;
    }

    QString source_path = m_sourceDirISO.takeFirst();
    QString iso_file = isoFileNameBySourceName(LFile::shortDirName(source_path));
    m_protocol->addText(QString("source folder: %1").arg(LFile::shortDirName(source_path)));
    m_protocol->addText(QString("target file: %1").arg(iso_file));
    m_processObj->setCommand(MAKE_ISO_COMMAND);
    m_processObj->setArgs(makeArgsBySourcePath(source_path, sourcePath()));
    printNextProcessCommand();

    m_curISOFile = QString("%1%2%3").arg(sourcePath()).arg(QDir::separator()).arg(iso_file);
    m_stage = isoProcessRun;
}
void MainForm::stopBreak()
{
    m_timer->stop();
    m_stage = isoStoped;
    if (m_processObj->isRunning())
        m_processObj->breakCommand();
    m_protocol->addText("Command breaked!", 5);
    updateActionsEnable(true);
}
void MainForm::save()
{
    LMainWidget::save();

    QSettings settings(companyName(), projectName());
    settings.setValue(QString("%1/v_splitter/state").arg(objectName()), v_splitter->saveState());

    m_paramsPage->save(settings);
}
void MainForm::load()
{
    LMainWidget::load();

    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("%1/v_splitter/state").arg(objectName()), QByteArray()).toByteArray());
    if (!ba.isEmpty()) v_splitter->restoreState(ba);

    m_paramsPage->load(settings);
}
QString MainForm::isoFileNameBySourceName(const QString &dir_source) const
{
    QString iso_file = dir_source.trimmed().toLower();
    iso_file = LStatic::removeLongSpaces(iso_file);
    iso_file.replace(LStatic::spaceSymbol(), "_");
    iso_file.replace("-", "_");
    iso_file.replace(".", "_");
    iso_file.replace(";", "_");
    iso_file.replace(":", "_");

    iso_file.remove("!");
    iso_file.remove("@");
    iso_file.remove("#");
    iso_file.remove("%");
    iso_file.remove("*");
    iso_file.remove("&");

    return QString("%1.iso").arg(iso_file);
}
QString MainForm::isoLabelBySourceName(const QString &dir_source) const
{
    QString label = dir_source.trimmed();
    return QString("%1 (%2)").arg(label).arg(LStatic::strCurrentDate());
}
QStringList MainForm::makeArgsBySourcePath(const QString &source_path, const QString &target_path) const
{
    QStringList args;
    args.append("-V");
    args.append(isoLabelBySourceName(LFile::shortDirName(source_path)));
    args.append("-r");
    args.append("-o");
    args.append(QString("%1%2%3").arg(target_path).arg(QDir::separator()).arg(isoFileNameBySourceName(LFile::shortDirName(source_path))));
    args.append(source_path);
    return args;
}

void MainForm::slotError(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttErr);
}
void MainForm::slotMessage(const QString &text)
{
    m_protocol->addText(text, LProtocolBox::ttText);
}

/*
void MainForm::slotAppSettingsChanged(QStringList keys)
{
    qDebug("MainForm::slotAppSettingsChanged");
    LMainWidget::slotAppSettingsChanged(keys);

    QString key = QString("log_max_size");
    if (keys.contains(key))
    {
        int n = lCommonSettings.paramValue(key).toInt();
        qDebug()<<QString("MainForm::slotAppSettingsChanged  n=%1").arg(n);
        LogPage *page = qobject_cast<LogPage*>(m_pages.value(BasePage::ptLog));
        if (!page)
        {
            qWarning()<<QString("MainForm::slotAppSettingsChanged() ERR: invalid convert to LogPage from m_pages");
            return;
        }
        page->setMaxSize(n);
        page->updatePage();
    }
}
*/

