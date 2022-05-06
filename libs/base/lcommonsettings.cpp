 #include "lcommonsettings.h"

 #include <QLineEdit>
 #include <QComboBox>
 #include <QDebug>

LCommonSettingsBase lCommonSettings;


/////////////LCommonSettings/////////////////////////
LCommonSettingsBase::LCommonSettingsBase()
//    :iconSize(0)
{
    params.clear();

    QString key(QString("icon_size"));
    addParam(QString("Icon size"), LSimpleDialog::sdtIntCombo, key);    
    QStringList combo_list;
    for (int i=0; i<10; i++)
    {
	int x = 24 + i*4;
	combo_list.append(QString::number(x));
    }
    setComboList(key, combo_list);
    setDefValue(key, QVariant(int(0)));
    
    key = QString("dialog_width");
    addParam(QString("Settings dialog width"), LSimpleDialog::sdtIntLine, key);    
    setDefValue(key, QVariant(int(500)));

    key = QString("dialog_height");
    addParam(QString("Settings dialog height"), LSimpleDialog::sdtIntLine, key);    
    setDefValue(key, QVariant(int(500)));


}
void LCommonSettingsBase::addParam(QString caption, int t, QString k, int p, QVariant dv)
{
    LCommonSettingsBase::LSettingsParam param(caption, t, k, dv, p);
    if (!param.invalid()) params.append(param);
    else qWarning()<<QString("LCommonSettingsBase::addParam ERR invalid parameter, key=%1").arg(k);

}
void LCommonSettingsBase::load(QSettings &settings)
{
//    iconSize = settings.value("app_settings/icon_size", -1).toInt();

    for (int i=0; i<params.count(); i++)
	params[i].value = settings.value(QString("app_settings/%1").arg(params.at(i).key), QVariant());

    setDefValuesToNullParams();
}
bool LCommonSettingsBase::hasParam(QString key) const
{
    if (key.trimmed().isEmpty()) return false;
    for (int i=0; i<params.count(); i++)
	if (params.at(i).key == key) return true;
    return false;
}
void LCommonSettingsBase::save(QSettings &settings)
{
    for (int i=0; i<params.count(); i++)
	settings.setValue(QString("app_settings/%1").arg(params.at(i).key), params.at(i).value);
}
QVariant LCommonSettingsBase::paramValue(QString key) const
{
    if (key.trimmed().isEmpty()) return QVariant();
    for (int i=0; i<params.count(); i++)
	if (params.at(i).key == key) return params.at(i).value;
    return QVariant();
}
void LCommonSettingsBase::removeAt(QString key)
{
    if (key.trimmed().isEmpty()) return;

    int index = -1;
    for (int i=0; i<params.count(); i++)
	if (params.at(i).key == key)
	{
	    index = i;
	    break;
	}

    if (index >= 0) 
	params.removeAt(index);
}
void LCommonSettingsBase::setDefValue(QString key, const QVariant &def_v)
{
    if (key.trimmed().isEmpty()) return;

    for (int i=0; i<params.count(); i++)
	if (params.at(i).key == key)
	{
	    params[i].def_value = def_v;
	    return;
	}
}
void LCommonSettingsBase::setComboList(QString key, const QStringList &list)
{
    if (key.trimmed().isEmpty()) return;

    for (int i=0; i<params.count(); i++)
	if (params.at(i).key == key)
	{
	    params[i].combo_list.clear();
	    for (int j=0; j<list.count(); j++)
		params[i].combo_list.append(QVariant(list.at(j)));
	    return;
	}
}
void LCommonSettingsBase::setDefValuesToNullParams()
{
    bool ok;
    for (int i=0; i<params.count(); i++)
    {
	QVariant v = params.at(i).value;
	QVariant def_v = params.at(i).def_value;
	if (!v.isNull() && v.isValid()) continue;

	switch (params.at(i).data_type)
	{
	    case LSimpleDialog::sdtColor:
	    {
		params[i].value = QVariant();
		break;
	    }
	    case LSimpleDialog::sdtString:
	    case LSimpleDialog::sdtStringCombo:
	    case LSimpleDialog::sdtFilePath:
	    case LSimpleDialog::sdtDirPath:
	    {
		if (!def_v.isNull() && def_v.isValid()) params[i].value = def_v;
		else params[i].value = QVariant(QString());
		break;
	    }
	    case LSimpleDialog::sdtDoubleLine:
	    case LSimpleDialog::sdtDoubleCombo:
	    {
		ok = false;
		if (!def_v.isNull() && def_v.isValid())
		{
		    def_v.toDouble(&ok);
		    if (ok) params[i].value = def_v;
		}
		if (!ok) params[i].value = QVariant(double(0));
		break;
	    }
	    case LSimpleDialog::sdtIntLine:
	    case LSimpleDialog::sdtIntCombo:
	    {
		ok = false;
		if (!def_v.isNull() && def_v.isValid())
		{
		    def_v.toInt(&ok);
		    if (ok) params[i].value = def_v;
		}
		if (!ok) params[i].value = QVariant(int(0));
		break;
	    }
	    case LSimpleDialog::sdtBool:
	    case LSimpleDialog::sdtBoolCombo:
	    {
		if (!def_v.isNull() && def_v.isValid()) params[i].value = def_v;
		else params[i].value = QVariant(bool(0));
		break;
	    }
	    default: break;
	}
    }
}

/////////////LCommonSettingsDialog/////////////////////////
LCommonSettingsDialog::LCommonSettingsDialog(LCommonSettingsBase &settings, QWidget *parent)
    :LSimpleDialog(parent),
    m_settings(settings)
{
    setWindowTitle(tr("Application settings"));
    setBoxTitle(tr("Values"));

    initWidgets();
    resizeBySettings();
    setExpandWidgets();
}
void LCommonSettingsDialog::resizeBySettings()
{
    int w = 500;
    QString key = QString("dialog_width");
    if (m_settings.hasParam(key))
	w = m_settings.paramValue(key).toInt();
	
    int h = 500;
    key = QString("dialog_height");
    if (m_settings.hasParam(key))
	h = m_settings.paramValue(key).toInt();

    resize(w, h);
    setCaptionsWidth(width()*0.5);
}
void LCommonSettingsDialog::resizeEvent(QResizeEvent*)
{
    QString key = QString("dialog_width");
    if (m_settings.hasParam(key))
	setWidgetValue(key, QVariant(width()));     

    key = QString("dialog_height");
    if (m_settings.hasParam(key))
	setWidgetValue(key, QVariant(height()));     

//    setCaptionsWidth(width()*0.5);

}
void LCommonSettingsDialog::initWidgets()
{
    qDebug("----------LCommonSettingsDialog::initWidgets()-------------");
    for (int i=0; i<m_settings.params.count(); i++)
    {
        qDebug()<<m_settings.params.at(i).toString();
        const LCommonSettingsBase::LSettingsParam &param = m_settings.params.at(i);
        addSimpleWidget(param.name, param.data_type, param.key, param.precision);
        if (!param.combo_list.isEmpty()) setComboList(param.key, param.combo_list);
        setWidgetValue(param.key, param.value);
    }

    addVerticalSpacer();
}
void LCommonSettingsDialog::slotApply()
{
    for (int i=0; i<m_settings.params.count(); i++)
	m_settings.params[i].value = widgetValue(m_settings.params.at(i).key);

    LSimpleDialog::slotApply();
}




