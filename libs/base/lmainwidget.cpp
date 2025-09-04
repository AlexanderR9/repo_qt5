 #include "lmainwidget.h"
 #include "lcommonsettings.h"

 #include <QToolBar>
 #include <QAction>
 #include <QDebug>
 #include <QSettings>
 #include <QVBoxLayout>
 #include <QGridLayout>
     
 #define DEF_SPACING	2
 #define DEF_MARGIN     4
 
     
// class LMainWidget
LMainWidget::LMainWidget(QWidget *parent)
    :QWidget(parent),
    m_toolBar(NULL),
    m_widget(NULL),
    m_appDialogCaptionWidth(-1)
{
    if (layout()) delete layout();
    QVBoxLayout *lay = new QVBoxLayout(0);
    lay->setSpacing(DEF_SPACING);
    lay->setContentsMargins(DEF_MARGIN, DEF_MARGIN, DEF_MARGIN, DEF_MARGIN);
    setLayout(lay);
    
    m_toolBar = new QToolBar(0);
    m_toolBar->setIconSize(QSize(defIconSize(), defIconSize()));
    lay->addWidget(m_toolBar);
    
    m_widget = new QWidget();
    lay->addWidget(m_widget);
    if (m_widget->layout()) delete m_widget->layout();
    m_widget->setLayout(new QGridLayout(0)); 
    m_widget->layout()->setSpacing(DEF_SPACING);
    m_widget->layout()->setContentsMargins(DEF_MARGIN, DEF_MARGIN, DEF_MARGIN, DEF_MARGIN);

    connect(this, SIGNAL(signalAction(int)), this, SLOT(slotAction(int)));
    connect(this, SIGNAL(signalAppSettingsChanged(QStringList)), this, SLOT(slotAppSettingsChanged(QStringList)));

}
void LMainWidget::addToolBarSeparator()
{
    if (m_toolBar)
	m_toolBar->addSeparator(); 
}
void LMainWidget::init()
{
    initCommonSettings();
    initActions();
    initWidgets();

    load();

    QString key = QString("icon_size");
    lCommonSettings.setDefValue(key, QVariant(LMainWidget::defIconSize()));        

    updateIconSize();
    updateWindowTitle();
}
void LMainWidget::updateIconSize()
{
    QString key = QString("icon_size");
    bool ok;
    int is = lCommonSettings.paramValue(key).toInt(&ok);
    if (!ok || is < 12 || is > 100) is = LMainWidget::defIconSize();
    m_toolBar->setIconSize(QSize(is, is));
}
void LMainWidget::save()
{
    QSettings settings(companyName(), projectName());    
    settings.setValue(QString("mainwidget/geometry"), saveGeometry());
    
    lCommonSettings.save(settings);
}
void LMainWidget::updateWindowTitle()
{
    setWindowTitle(mainTitle());
}
void LMainWidget::load()
{    
    QSettings settings(companyName(), projectName());
    QByteArray ba(settings.value(QString("mainwidget/geometry"), QByteArray()).toByteArray());
    if (!ba.isEmpty()) restoreGeometry(ba);

    lCommonSettings.load(settings);
}
void LMainWidget::slotTriggered()
{
    const QAction *action = qobject_cast<const QAction*>(sender());
    if (!action) return;
    
    bool ok;
    int type = action->data().toInt(&ok);
    if (!ok) {qWarning()<<QString("LMainWidget::slotTriggered()  err data, action %1").arg(action->objectName()); return;}	
    
    if (type == atExit) close();
    else emit signalAction(type);
}
void LMainWidget::addWidget(QWidget *w, int row, int col, int rows, int cols)
{
    QGridLayout *lay = qobject_cast<QGridLayout*>(m_widget->layout());
    if (!lay || !w) return;

    lay->addWidget(w, row, col, rows, cols);
} 
void LMainWidget::setActionTooltip(int type, QString text)
{
    QAction *action = getAction(type);
    if (!action) qWarning()<<QString("LMainWidget::setActionTooltip WARNING - not found action %1 (%2)").arg(type).arg(actionIconName(type));
    else action->setText(text);
}
void LMainWidget::addAction(int type)
{
    QString icon_name(actionIconName(type));
    if (icon_name.isEmpty()) return;
    
    QString path = QString(":/icons/images");
    if (icon_name.right(4) == ".png") path = QString("%1/%2").arg(path).arg(icon_name);
    else path = QString("%1/%2.svg").arg(path).arg(icon_name);

    QAction *action = new QAction(QIcon(path), actionText(type), this);    
    action->setObjectName(QString("action_%1").arg(type));
    action->setData(type);
    m_toolBar->addAction(action);
    
    connect(action, SIGNAL(triggered()), this, SLOT(slotTriggered()));
}
QAction* LMainWidget::getAction(int type) const
{
    QList<QAction*> list = m_toolBar->actions();
    for (int i=0; i<list.count(); i++)
	if (list.at(i)->data().toInt() == type) return list.at(i);
    return NULL;
}
QString LMainWidget::actionIconName(int type)
{
    switch (type)
    {
        case atSettings: return  QString("applications-system");
        case atExit: return QString("system-log-out");
        case atStart: return QString("media-play");
        case atStop: return QString("media-stop");
        case atPause: return QString("media-pause");
        case atAdd: return QString("list-add");
        case atRemove: return QString("list-remove");
        case atOk: return QString("emblem-ok");
        case atCancel: return QString("emblem-cancel");
        case atClear: return QString("edit-clear");
        case atRedo: return QString("edit-redo");
        case atUndo: return QString("edit-undo");
        case atChart: return QString("chart");
        case atClock: return QString("clock");
        case atOpen: return QString("document-open");
        case atSave: return QString("document-save");
        case atNewProject: return QString("document-new");
        case atSearch: return QString("system-search");
        case atUp: return QString("up");
        case atDown: return QString("down");
        case atLeft: return QString("left");
        case atRight: return QString("right");
        case atLoadData: return QString("down");
        case atData: return QString("system-file-manager");
        case atRefresh: return QString("view-refresh");
        case atMonitoring: return QString("utilities-system-monitor");
        case atBuy: return QString("ball_green");
        case atSell: return QString("ball_red");
        case atBag: return QString("bag");
        case atBScale: return QString("b_scale");
        case atRScale: return QString("r_scale");
        case atSScale: return QString("octane");
        case atChain: return QString("chain");
        case atCoin: return QString("coin");
        case atSendMsg: return QString("send_msg.png");
        case atHouse: return QString("house");
        case atPerimetr: return QString("perimetr");
        case atEject: return QString("eject.png");
        case atISO: return QString("isofile.png");
        case atISOCD: return QString("iso_cd.png");
        case atBurn: return QString("burn.png");
        case atCDErase: return QString("cd_erase.png");
        case atCalcCRC: return QString("crc.png");
        case atFoldersStruct: return QString("folders_struct.png");
        case atJS: return QString("js.png");



        default: break;
    }
    
    return QString();	
}
QString LMainWidget::actionText(int type)
{
    switch (type)
    {
        case atSettings: return  QString("Application settings");
        case atExit: return QString("Exit");
        case atStart: return QString("Start");
        case atStop: return QString("Stop");
        case atPause: return QString("Pause");
        case atAdd: return QString("Add");
        case atRemove: return QString("Remove");
        case atOk: return QString("Ok");
        case atCancel: return QString("Cancel");
        case atClear: return QString("Clear");
        case atRedo: return QString("Redo");
        case atUndo: return QString("Undo");
        case atChart: return QString("Chart");
        case atClock: return QString("Clock");
        case atOpen: return QString("Open");
        case atSave: return QString("Save");
        case atNewProject: return QString("New project");
        case atSearch: return QString("Search");
        case atUp: return QString("Up");
        case atDown: return QString("Down");
        case atLeft: return QString("Left");
        case atRight: return QString("Right");
        case atLoadData: return QString("Load data");
        case atData: return QString("General data");
        case atRefresh: return QString("Refresh");
        case atMonitoring: return QString("Execute test");
        case atBuy: return QString("Buy");
        case atSell: return QString("Sell");
        case atBag: return QString("Bag");
        case atBScale: return QString("Bar scale");
        case atRScale: return QString("Round scale");
        case atSScale: return QString("Scale");
        case atChain: return QString("Chain");
        case atCoin: return QString("Coin");
        case atSendMsg: return QString("Send message");
        case atHouse: return QString("House");
        case atPerimetr: return QString("Perimetr");
        case atEject: return QString("Eject");
        case atISO: return QString("Make ISO of data");
        case atISOCD: return QString("Make ISO by CD");
        case atBurn: return QString("Burn CD");
        case atCDErase: return QString("Erase CD");
        case atCalcCRC: return QString("Calc CRC");
        case atFoldersStruct: return QString("Folders struct");
        case atJS: return QString("JavaScript code executing");

        default: break;
    }
    
    return QString();	
}
void LMainWidget::slotAppSettingsChanged(QStringList keys)
{
    for (int i=0; i<keys.count(); i++)
	qDebug()<<QString("LMainWidget::slotAppSettingsChanged  param %1").arg(keys.at(i));

    QString key = QString("icon_size");
    if (keys.contains(key)) updateIconSize();

}
void LMainWidget::actCommonSettings()
{
    qDebug("MainForm::actCommonSettings()");
    LCommonSettingsBase oldSettings = lCommonSettings;

    LCommonSettingsDialog d(lCommonSettings, this);
    if (m_appDialogCaptionWidth > 0)
    {
	qDebug()<<QString("LMainWidget::actCommonSettings() setCaptionsWidth(%1)").arg(m_appDialogCaptionWidth);
	d.setCaptionsWidth(m_appDialogCaptionWidth);
    }
//    d.setExpandWidgets();
    d.exec();


    if (d.isApply())
    {
	QStringList keys;
	for (int i=0; i<lCommonSettings.paramsCount(); i++)
	    if (lCommonSettings.params.at(i).value != oldSettings.params.at(i).value)	
		keys.append(lCommonSettings.params.at(i).key);	

	if (!keys.isEmpty())
	    emit signalAppSettingsChanged(keys);
    }

//    if (oldSettings.iconSize != lCommonSettings.iconSize)
//        m_toolBar->setIconSize(QSize(lCommonSettings.iconSize, lCommonSettings.iconSize));
}


                                                

