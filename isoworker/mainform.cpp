#include "mainform.h"
#include "lcommonsettings.h"
#include "lprotocol.h"
#include "processobj.h"
#include "paramspage.h"
#include "lfile.h"
#include "lstatic.h"
#include "foldersstructdialog.h"


#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QProgressBar>
#include <QGroupBox>
#include <QMessageBox>


#define MAKE_ISO_COMMAND        QString("genisoimage")
#define MD5SUM_COMMAND          QString("md5sum")
#define CDRECORD_COMMAND        QString("cdrecord")
#define UMOUNT_COMMAND          QString("umount")
#define ISOINFO_COMMAND         QString("isoinfo")
#define MAKE_ISO_INTERVAL       1700




// MainForm
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_paramsPage(NULL),
    m_protocol(NULL),
    v_splitter(NULL),
    m_processObj(NULL),
    m_timer(NULL),
    m_cdBlockSize(-1)
{
    m_processObj = new LProcessObj(this);
    m_processObj->setDebugLevel(1);
    connect(m_processObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_processObj, SIGNAL(signalMsg(const QString&)), this, SLOT(slotMessage(const QString&)));
    connect(m_processObj, SIGNAL(signalFinished()), this, SLOT(slotFinished()));
    connect(m_processObj, SIGNAL(signalReadyRead()), this, SLOT(slotReadyRead()));


    m_timer = new QTimer(this);
    m_timer->setInterval(MAKE_ISO_INTERVAL);
    m_timer->stop();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

}
void MainForm::initActions()
{
    addAction(LMainWidget::atISO);
    addAction(LMainWidget::atBurn);
    addAction(LMainWidget::atCDErase);
    addAction(LMainWidget::atStop);
    addAction(LMainWidget::atEject);
    addAction(LMainWidget::atRemove);
    addAction(LMainWidget::atCalcCRC);
    addAction(LMainWidget::atFoldersStruct);
    addToolBarSeparator();
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

    setActionTooltip(atRemove, "Umount CD");
    setActionTooltip(atStop, "Break process");
    setActionTooltip(atFoldersStruct, "Show CD struct");
    updateActionsEnable(true);

    connect(m_processObj, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;

    QString key = QString("source");
    lCommonSettings.addParam(QString("Source dir"), LSimpleDialog::sdtDirPath, key);
    lCommonSettings.setDefValue(key, QString(""));

    key = QString("speed");
    lCommonSettings.addParam(QString("Write speed"), LSimpleDialog::sdtStringCombo, key);
    combo_list.clear();
    combo_list << "none" << "4x" << "8x" << "12x" << "16x" << "24x" << "36x" << "48x";
    lCommonSettings.setComboList(key, combo_list);

    key = QString("cd_dev");
    lCommonSettings.addParam(QString("CDRW device"), LSimpleDialog::sdtString, key);
    lCommonSettings.setDefValue(key, QString("/dev/cdrom"));


}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atISO: {startMakerISO(); break;}
        case LMainWidget::atStop: {stopBreak(); break;}
        case LMainWidget::atClear: {m_protocol->clearProtocol(); break;}
        case LMainWidget::atBurn: {tryBurn(); break;}
        case LMainWidget::atEject: {eject(); break;}
        case LMainWidget::atRemove: {umount(); break;}
        case LMainWidget::atCDErase: {tryErase(); break;}
        case LMainWidget::atCalcCRC: {calcMD5_CD(); break;}
        case LMainWidget::atFoldersStruct: {showFoldersStructCD(); break;}
        default: break;
    }
}
void MainForm::tryBurn()
{
    prepareCommand("BURN CD");

    QString f_name = m_paramsPage->seletedISOFile();
    if (!f_name.contains(".iso"))
    {
        slotError("invalid selection ISO file");
        return;
    }

    f_name = QString("%1%2%3").arg(sourcePath()).arg(QDir::separator()).arg(f_name);
    if (!LFile::fileExists(f_name))
    {
        slotError(QString("file not exest: %1").arg(f_name));
        return;
    }

    QString q_text = QString("Source file: %1.\n%2 ").arg(f_name).arg("Are you sure you want to burn a disc?");
    int res = QMessageBox::question(this, "Burning CD!!!", q_text, QMessageBox::Ok, QMessageBox::Cancel);
    if (res == QMessageBox::Ok)
    {
        startBurning(f_name);
    }
    else
    {
        m_protocol->addText("burnig canceled", LProtocolBox::ttWarning);
    }
}
void MainForm::startBurning(const QString &iso_file)
{
    m_protocol->addText("start burning ........");
    m_protocol->addText(QString("ISO file: %1").arg(iso_file));
    m_protocol->addText(QString("Burning speed: %1").arg(burnSpeed()));
    m_protocol->addText(QString("CDROM: %1").arg(cdDevice()));

    if (cdDevice().isEmpty())
    {
        slotError("invalid CDROM device");
        return;
    }

    m_processObj->setCommand(CDRECORD_COMMAND);
    m_paramsPage->startCommand("BURN", iso_file);

    QStringList args;
    args << "-v" << "-eject" << "-dao";
    bool ok;
    quint8 speed = LStatic::strTrimRight(burnSpeed(), 1).trimmed().toUInt(&ok);
    if (speed > 2 && ok && speed < 100) args.append(QString("speed=%1").arg(speed));
    args.append(QString("dev=%1").arg(cdDevice()));
    args.append(iso_file);
    m_processObj->setArgs(args);

    runProcess(isoBurningCD);
}
void MainForm::tryErase()
{
    prepareCommand("ERASE CD");

    QString q_text = QString("CDRW device: %1.\n%2 ").arg(cdDevice()).arg("Are you sure you want erase a disc?");
    int res = QMessageBox::question(this, "Erase CD!!!", q_text, QMessageBox::Ok, QMessageBox::Cancel);
    if (res == QMessageBox::Ok)
    {
        startErase();
    }
    else
    {
        m_protocol->addText("erase canceled", LProtocolBox::ttWarning);
    }
}
void MainForm::startErase()
{
    m_protocol->addText("start erase ........");
    m_protocol->addText(QString("CDROM: %1").arg(cdDevice()));

    if (cdDevice().isEmpty())
    {
        slotError("invalid CDROM device");
        return;
    }

    m_processObj->setCommand(CDRECORD_COMMAND);

    QStringList args;
    args << "-v" << QString("dev=%1").arg(cdDevice()) << "blank=fast";
    m_processObj->setArgs(args);
    m_paramsPage->startCommand("ERASE CD");

    runProcess(isoEraseCD);
}
void MainForm::eject()
{
    prepareCommand("EJECT CD");

    if (cdDevice().isEmpty())
    {
        slotError("invalid CDROM device");
        return;
    }

    m_paramsPage->startCommand("EJECT");
    m_processObj->setCommand(CDRECORD_COMMAND);
    QStringList args;
    args << "-v" << "-eject" << QString("dev=%1").arg(cdDevice());
    m_processObj->setArgs(args);
    runProcess(isoEjectCDROM);
}
void MainForm::umount()
{
    prepareCommand("UMOUNT CD");
    if (cdDevice().isEmpty())
    {
        slotError("invalid CDROM device");
        return;
    }

    m_processObj->setCommand(UMOUNT_COMMAND);
    QStringList args;
    args << cdDevice();
    m_processObj->setArgs(args);
    runProcess(isoUmountCD);
}
void MainForm::runProcess(int cmd_type)
{
    m_stage = cmd_type;
    printNextProcessCommand();
    updateActionsEnable(false);
    m_processObj->startCommand();
}
void MainForm::prepareCommand(QString title)
{
    m_paramsPage->resetColors();
    m_protocol->addSpace();
    m_protocol->addText(QString("[%1]").arg(title), LProtocolBox::ttFile);
}
void MainForm::showFoldersStructCD()
{
    FoldersStructDialog d(this);
    d.exec();

}
void MainForm::slotReadyRead()
{
    qDebug("MainForm::slotReadyRead()");
}
void MainForm::slotFinished()
{
    qDebug("MainForm::slotFinished()");
    QString s_result = (m_processObj->isOk() ? "Ok" : "Fault");
    int p_type = (m_processObj->isOk() ? LProtocolBox::ttText : LProtocolBox::ttErr);
    m_protocol->addText(QString("process finished, result_code=[%1]").arg(s_result), p_type);

    QStringList debug_list(m_processObj->bufferList());
    for (int i=0; i<debug_list.count(); i++) qDebug()<<debug_list.at(i);

    checkProcessFinishedResult();
}
void MainForm::checkProcessFinishedResult()
{
    switch (m_stage)
    {
        case isoBurningCD: {checkBurnProcessFinishedResult(); break;}
        case isoEraseCD: {checkEraseProcessFinishedResult(); break;}
        case isoEjectCDROM: {checkEjectProcessFinishedResult(); break;}
        case isoNeedCalcMD5_CD: {checkMD5CDProcessFinishedResult(); break;}
        case isoUmountCD: {checkUmountProcessFinishedResult(); break;}
        default: {checkISOProcessFinishedResult(); break;}
    }
}
void MainForm::checkBurnProcessFinishedResult()
{
    QString buff(m_processObj->buffer().trimmed().toLower());
    if (!m_processObj->isOk())
    {
        slotError(buff);
        m_paramsPage->finishedCommand("fault");
    }
    else m_paramsPage->finishedCommand("ok");
    stopOk();
}
void MainForm::checkMD5CDProcessFinishedResult()
{
    QString buff(m_processObj->buffer().trimmed().toLower());
    if (!m_processObj->isOk()) slotError(buff);
    if (buff.contains("no such file or directory"))
    {
        m_protocol->addText(QString("CD disk not found"), LProtocolBox::ttWarning);
        stopOk();
        return;
    }

    //check m_cdBlockSize
    if (m_cdBlockSize < 0)
    {
        QStringList debug_list(m_processObj->bufferList());
        for (int i=0; i<debug_list.count(); i++)
        {
            QString s = debug_list.at(i).trimmed().toLower();
            if (s.contains("volume size is:"))
            {
                bool ok;
                m_cdBlockSize = LStatic::strTrimLeft(s, QString("volume size is:").length()).trimmed().toInt(&ok);
                if (!ok || m_cdBlockSize <= 0)
                {
                    m_protocol->addText(QString("volume size not found"), LProtocolBox::ttWarning);
                    stopBreak();
                    return;
                }
            }
        }
        if (m_cdBlockSize > 0)
        {
            m_protocol->addText(QString("finded volume size: %1").arg(m_cdBlockSize), LProtocolBox::ttOk);
            m_protocol->addSpace();
            m_protocol->addText(" -------- stage 2 ----------- ");


            //command: dd if=/dev/sr0 bs=2048 count=354854 conv=notrunc,noerror | md5sum -b
            m_processObj->setCommand("dd");
            QStringList args;
            args << QString("if=%1").arg(cdDevice()) << QString("bs=2048") << QString("count=%1").arg(m_cdBlockSize);
            args << QString("conv=notrunc,noerror") << QString("|") << QString("md5sum") << QString("-b");
            m_processObj->setArgs(args);
            printNextProcessCommand();
            m_processObj->startCommand();
        }
        else
        {
            m_protocol->addText(QString("volume size: %1").arg(m_cdBlockSize), LProtocolBox::ttWarning);
            stopBreak();
            return;
        }
    }
    else
    {
        QString s_crc = buff.trimmed();
        if (s_crc.isEmpty())
        {
            m_protocol->addText(QString("debug of process is empty"), LProtocolBox::ttWarning);
        }
        else
        {
            int pos = s_crc.indexOf(LStatic::spaceSymbol());
            if (pos < 0) pos = s_crc.indexOf("*");
            if (pos > 0) s_crc = s_crc.left(pos).trimmed();
            m_protocol->addText(QString("MD5 sum of CD:  %1").arg(s_crc), LProtocolBox::ttOk);
            m_paramsPage->findMD5(s_crc);
        }
        stopOk();
    }
}
void MainForm::checkEjectProcessFinishedResult()
{
    QString buff(m_processObj->buffer().trimmed().toLower());
    if (!m_processObj->isOk())
    {
        slotError(buff);
        m_paramsPage->finishedCommand("fault");
    }
    else m_paramsPage->finishedCommand("ok");
    stopOk();


    if (buff.contains("cannot load media with this drive"))
    {
        m_protocol->addText(QString("CDROM is opened"), LProtocolBox::ttWarning);
    }
    if (buff.contains("device or resource busy"))
    {
        m_protocol->addText(QString("device %1 is busy,  need umount %1").arg(cdDevice()), LProtocolBox::ttWarning);
    }
}
void MainForm::checkUmountProcessFinishedResult()
{
    QString buff(m_processObj->buffer().trimmed().toLower());
    if (!m_processObj->isOk()) slotError(buff);
    stopOk();

    if (buff.contains("process_err")) m_protocol->addText(buff, LProtocolBox::ttWarning);
    else m_protocol->addText(buff);
}
void MainForm::checkEraseProcessFinishedResult()
{
    QString buff(m_processObj->buffer().trimmed().toLower());
    if (!m_processObj->isOk())
    {
        slotError(buff);
        m_paramsPage->finishedCommand("fault");
    }
    else m_paramsPage->finishedCommand("ok");
    stopOk();

    if (buff.contains("Error trying to open") || buff.contains("device or resource busy"))
    {
        m_protocol->addText(QString("device %1 is busy,  need umount %1").arg(cdDevice()), LProtocolBox::ttWarning);
    }
    if (buff.contains("cannot load media with this drive"))
    {
        m_protocol->addText(QString("CDROM is opened"), LProtocolBox::ttWarning);
    }

}
void MainForm::checkISOProcessFinishedResult()
{
    if (!m_processObj->isOk())
    {
        m_stage = isoNeedBreak;
        m_paramsPage->finishedCommand("fault");
        return;
    }

    if (!m_curISOFile.isEmpty())
    {
        m_stage = isoNeedCalcMD5;
        m_paramsPage->finishedCommand("ok");
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
QString MainForm::burnSpeed() const
{
    return lCommonSettings.paramValue("speed").toString().trimmed();
}
QString MainForm::cdDevice() const
{
    return lCommonSettings.paramValue("cd_dev").toString().trimmed();
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
    m_protocol->addText(QString("append md5sum to file: %1").arg(ParamsPage::md5File()));
    m_protocol->addText(QString("crc_value: %1").arg(crc_value));

    QString f_name = QString("%1%2%3").arg(sourcePath()).arg(QDir::separator()).arg(ParamsPage::md5File());
    QString err = LFile::appendFile(f_name, f_line);
    if (!err.isEmpty())
        slotError(err);

}
void MainForm::updateActionsEnable(bool stoped)
{
    getAction(LMainWidget::atStop)->setEnabled(!stoped);
    getAction(LMainWidget::atISO)->setEnabled(stoped);
    getAction(LMainWidget::atBurn)->setEnabled(stoped);
    getAction(LMainWidget::atEject)->setEnabled(stoped);
    getAction(LMainWidget::atRemove)->setEnabled(stoped);
    getAction(LMainWidget::atCDErase)->setEnabled(stoped);
    //getAction(LMainWidget::atCalcCRC)->setEnabled(false);
    getAction(LMainWidget::atCalcCRC)->setEnabled(stoped);
    getAction(LMainWidget::atSettings)->setEnabled(stoped);
    getAction(LMainWidget::atExit)->setEnabled(stoped);
}
void MainForm::readParamsPage()
{
    /*
    m_processObj->setCommand(m_paramsPage->commandName());
    m_processObj->setArgs(m_paramsPage->getArgs());
    m_processObj->setSudo(m_paramsPage->isSudo());
    */
}
void MainForm::startMakerISO()
{
    m_paramsPage->resetColors();

    m_curISOFile.clear();
    m_protocol->addSpace();
    m_protocol->addText("Starting cenarii for maker ISO", LProtocolBox::ttOk);

    QString q_text = QString("Working directory: %1.").arg(sourcePath());
    q_text = QString("%1\n%2").arg(q_text).arg(QString("All ISO files in the working directory will be overwritten."));
    q_text = QString("%1\n%2").arg(q_text).arg(QString("Are you sure you want to continue?"));
    int res = QMessageBox::question(this, "Maker ISO!!!", q_text, QMessageBox::Ok, QMessageBox::Cancel);
    if (res == QMessageBox::Ok)
    {
        updateActionsEnable(false);
        m_stage = isoStarting;
        prepareSourceDirISO();
        m_timer->start();
    }
    else
    {
        m_protocol->addText("maker ISO canceled", LProtocolBox::ttWarning);
    }
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

    m_paramsPage->reloadISOList(sourcePath());
}
void MainForm::calcMD5_CD()
{
    prepareCommand("CALC MD5 (CD-ROM)");
    m_cdBlockSize = -1;

    if (cdDevice().isEmpty())
    {
        slotError("invalid CDROM device");
        return;
    }

    m_processObj->setCommand(ISOINFO_COMMAND);

    QStringList args;
    args << "-d" << "-i" << cdDevice();
    m_processObj->setArgs(args);

    runProcess(isoNeedCalcMD5_CD);
}
void MainForm::calcMD5()
{
    prepareCommand("CALC MD5 (FILE)");
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
    m_protocol->addText("[MAKE NEXT ISO]", LProtocolBox::ttFile);
    if (m_sourceDirISO.isEmpty())
    {
        m_protocol->addText("ISO source folders is empty", LProtocolBox::ttWarning);
        m_stage = isoNeedBreak;
        return;
    }

    QString source_path = m_sourceDirISO.takeFirst();
    QString iso_file = isoFileNameBySourceName(LFile::shortDirName(source_path));
    m_paramsPage->startCommand("MAKE ISO", iso_file);
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
    m_paramsPage->resetColors();

    m_timer->stop();
    m_stage = isoStoped;
    if (m_processObj->isRunning())
        m_processObj->breakCommand();

    m_protocol->addText("Command breaked!", 5);
    updateActionsEnable(true);
    m_paramsPage->finishedCommand("breaked");
}
void MainForm::stopOk()
{
    m_timer->stop();
    m_stage = isoStoped;
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
    m_paramsPage->reloadISOList(sourcePath());
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
    args.append("-J");
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


void MainForm::slotAppSettingsChanged(QStringList keys)
{
    qDebug("MainForm::slotAppSettingsChanged");
    LMainWidget::slotAppSettingsChanged(keys);

    if (keys.contains("source"))
    {
        m_protocol->addSpace();
        m_protocol->addText(QString("Changed source dir: %1").arg(sourcePath()));
        m_protocol->addText(QString("reloading ISO file list ...."));
        m_paramsPage->reloadISOList(sourcePath());
        m_protocol->addText(QString("done!"));
    }
}


