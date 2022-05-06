 #include "lsimpledialog.h"
 #include "lstatic.h"

 #include <QStringList>
 #include <QLineEdit>
 #include <QComboBox>
 #include <QCheckBox>
 #include <QLabel>
 #include <QDialogButtonBox>
 #include <QPushButton>
 #include <QToolButton>
 #include <QDebug>
 #include <QVBoxLayout>
 #include <QHBoxLayout>
 #include <QFileDialog>
 #include <QIntValidator>
 #include <QDoubleValidator>
 #include <QSpacerItem>
 #include <QColorDialog>


/////////////LSimpleDialog/////////////////////////
LSimpleDialog::LSimpleDialog(QWidget *parent)
    :QDialog(parent),
     m_apply(false)
//    m_captionsWidth(-1);
{
    setupUi(this);
    setWindowTitle("Simple dialog");
    setObjectName("app_settings_dialog");

    if (groupBox->layout()) delete groupBox->layout();
    QVBoxLayout *lay = new QVBoxLayout(0);
    int lay_spacing = 4;
    int lay_margin = 10;
    lay->setSpacing(lay_spacing);
    lay->setContentsMargins(lay_margin, lay_margin, lay_margin, lay_margin);
    groupBox->setLayout(lay);


    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(slotApply()));
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));
}
void LSimpleDialog::setCaptionsWidth(int w)
{
    if (w < 10 || w > 0.8*width()) return;
    for (int i=0; i<count(); i++)
    {
	//qDebug()<<QString("LSimpleDialog::setCaptionsWidth  label: %1,  width = %2").arg(m_widgets.at(i).label->text()).arg(w);
	m_widgets.at(i).label->setMinimumWidth(w);
    }
}
void LSimpleDialog::setExpandWidgets()
{
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Preferred);
    for (int i=0; i<count(); i++)
    {
	const SimpleWidget &sw = m_widgets.at(i);
	if (sw.comboBox) sw.comboBox->setSizePolicy(sp);
	if (sw.edit) sw.edit->setSizePolicy(sp);
	if (sw.checkBox) sw.checkBox->setSizePolicy(sp);
    }
}
void LSimpleDialog::setSizes(int w, int h)
{
    if (w < 50) w = 500;
    if (h < 50) h = 500;
    resize(w, h);
}
void LSimpleDialog::addVerticalSpacer()
{
    QVBoxLayout *lay = qobject_cast<QVBoxLayout*>(groupBox->layout());
    if (!lay) return;

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 30, QSizePolicy::Minimum, QSizePolicy::Expanding);
    lay->addItem(verticalSpacer);
}
void LSimpleDialog::addSimpleWidget(QString caption, int dataType, QString key, int precision)
{
    if (key.trimmed().isEmpty() || caption.trimmed().isEmpty()) return;
    if (dataType < sdtString || dataType > sdtColor) return;
    if (precision < 0 || precision > 10) precision = 1;
    key = key.trimmed().toLower();

    for (int i=0; i<m_widgets.count(); i++)
    {
	if (m_widgets.at(i).key == key)
	{
	    qWarning()<<QString("LSimpleDialog::addSimpleWidget ERR key[%1] allready exist.").arg(key);
	    return;
	}
    }

    m_widgets.append(SimpleWidget(caption, dataType));
    m_widgets.last().key = key;
    m_widgets.last().precision = precision;
    
    switch (dataType)
    {
	case sdtFilePath:
	case sdtDirPath:
	{
	    initSimpleWidgetPath(m_widgets.last());
	    break;
	}
	case sdtColor:
	{
	    initSimpleWidgetColor(m_widgets.last());
	    //setWidgetValue(key, QVariant()); 
	    break;
	}
	case sdtString:
	case sdtIntLine:
	case sdtDoubleLine: 
	{
	    initSimpleWidgetLine(m_widgets.last());
	    break;
	}
	case sdtBoolCombo:
	case sdtStringCombo:
	case sdtIntCombo:
	case sdtDoubleCombo: 
	{
	    initSimpleWidgetCombo(m_widgets.last());
	    break;
	}
	case sdtBool:
	{
	    m_widgets.last().checkBox = new QCheckBox(QString("  "), this);
	    m_widgets.last().checkBox->setObjectName(QString("checkbox_%1").arg(key));
	    break;
	}
	default: return;
    }

    m_widgets.last().label = new QLabel(m_widgets.last().caption, this);
    placeSimpleWidget(m_widgets.last());
}
void LSimpleDialog::initSimpleWidgetColor(SimpleWidget &sw)
{
    sw.edit = new QLineEdit(this);
    sw.edit->setObjectName(QString("edit_%1").arg(sw.key));
    sw.edit->setReadOnly(true);
    sw.edit->setAlignment(Qt::AlignCenter);
    sw.edit->clear();

    int h_edit = sw.edit->height();
    sw.button = new QToolButton(this);
    sw.button->setObjectName(QString("button_%1").arg(sw.key));
    sw.button->setText(QString());
    sw.button->setFixedSize(QSize(h_edit, h_edit));

    QString icon_name = QString(":/icons/images/octane.svg");
    sw.button->setIcon(QIcon(icon_name));
	    
    connect(sw.button, SIGNAL(clicked()), this, SLOT(slotOpenColorDialog()));
}
void LSimpleDialog::initSimpleWidgetPath(SimpleWidget &sw)
{
    sw.edit = new QLineEdit(this);
    sw.edit->setObjectName(QString("edit_%1").arg(sw.key));
    sw.edit->clear();

    int h_edit = sw.edit->height();
    sw.button = new QToolButton(this);
    sw.button->setObjectName(QString("button_%1").arg(sw.key));
    sw.button->setText(QString());
    sw.button->setFixedSize(QSize(h_edit, h_edit));

    QString icon_name = QString(":/icons/images/document-open.svg");
    sw.button->setIcon(QIcon(icon_name));
	    
    connect(sw.button, SIGNAL(clicked()), this, SLOT(slotOpenPath()));
}
void LSimpleDialog::initSimpleWidgetLine(SimpleWidget &sw)
{
    sw.edit = new QLineEdit(this);
    sw.edit->setObjectName(QString("edit_%1").arg(sw.key));
    sw.edit->clear();
	    
    if (sw.data_type == sdtIntLine)
    {
	QIntValidator *iv = new QIntValidator(this);
	sw.edit->setValidator(iv);
    }
    else if (sw.data_type == sdtDoubleLine)
    {
	QDoubleValidator *iv = new QDoubleValidator(this);
	sw.edit->setValidator(iv);
    }
}
void LSimpleDialog::initSimpleWidgetCombo(SimpleWidget &sw)
{
    sw.comboBox = new QComboBox(this);
    sw.comboBox->setObjectName(QString("combobox_%1").arg(sw.key));
    sw.comboBox->clear();
}
void LSimpleDialog::slotOpenColorDialog()
{
    if (!sender()) return;
    QString obj_name = sender()->objectName();
    int pos = obj_name.indexOf("_");
    if (pos < 0) return;
    QString key = obj_name.right(obj_name.length()-pos-1);

    QColor wc = LStatic::strToColor((widgetValue(key).toString()));
    QColor c = QColorDialog::getColor(wc, this); 

    QVariant value(LStatic::fromColor(c));
    setWidgetValue(key, value);    
}
void LSimpleDialog::slotOpenPath()
{
    if (!sender()) return;
    QString obj_name = sender()->objectName();
    int pos = obj_name.indexOf("_");
    if (pos < 0) return;
    QString key = obj_name.right(obj_name.length()-pos-1);

    const SimpleWidget *sw = widgetByKey(key);
    if (!sw) return;

    QString path;
    if (sw->data_type == sdtFilePath)
    {
	path = QFileDialog::getOpenFileName(this, tr("Open file"), widgetValue(sw->key).toString());
    }
    else if (sw->data_type == sdtDirPath)
    {
	path = QFileDialog::getExistingDirectory(this, tr("Open directory"), widgetValue(sw->key).toString());
    }

    if (!path.isEmpty())
	sw->edit->setText(path.trimmed());
}
void LSimpleDialog::placeSimpleWidget(const SimpleWidget &sw)
{
    if (sw.invalid()) {qWarning()<<QString("LSimpleDialog::placeSimpleWidget ERR sw.invalid(), key=%1").arg(sw.key); return;}
//    qDebug()<<QString("placeSimpleWidget  %1").arg(sw.key);

    QWidget *w = new QWidget(this); 
    if (w->layout()) delete w->layout();
    QHBoxLayout *lay = new QHBoxLayout(0);
    int lay_spacing = 6;
    int lay_margin = 2;
    lay->setSpacing(lay_spacing);
    lay->setContentsMargins(lay_margin, lay_margin, lay_margin, lay_margin);
    w->setLayout(lay);

    //sw.label = new QLabel(sw.caption, this);
    //lay->addWidget(sw.label);

    if (sw.label) lay->addWidget(sw.label);
    if (sw.edit) lay->addWidget(sw.edit);
    if (sw.comboBox) lay->addWidget(sw.comboBox);
    if (sw.checkBox) lay->addWidget(sw.checkBox);
    if (sw.button) lay->addWidget(sw.button);

    groupBox->layout()->addWidget(w);
}
void LSimpleDialog::setBoxTitle(QString s)
{
    groupBox->setTitle(s);
}
const LSimpleDialog::SimpleWidget* LSimpleDialog::widgetByKey(QString key) const
{
    QString s = key.trimmed();
    if (s.isEmpty()) return NULL;	

    for (int i=0; i<m_widgets.count(); i++)
	if (m_widgets.at(i).key == s) return &m_widgets.at(i);
    return NULL;
}
QVariant LSimpleDialog::widgetValue(QString key) const
{
    const SimpleWidget *sw = widgetByKey(key);
    if (!sw) return QVariant();
    if (sw->invalid()) return QVariant();
    
    switch (sw->data_type)
    {
	case sdtFilePath:
	case sdtDirPath:
	case sdtColor:
	case sdtString: return QVariant(sw->edit->text());
	case sdtStringCombo: return QVariant(sw->comboBox->currentText());
	case sdtIntLine: return QVariant(sw->edit->text().toInt());
	case sdtIntCombo: return QVariant(sw->comboBox->currentText().toInt());
	case sdtDoubleLine: return QVariant(sw->edit->text().toDouble());
	case sdtDoubleCombo: return QVariant(sw->comboBox->currentText().toDouble());
	case sdtBoolCombo: return QVariant(bool(sw->comboBox->currentIndex() == 1));
	case sdtBool: return QVariant(sw->checkBox->isChecked());
	default: break;
    }

    return QVariant();
}
void LSimpleDialog::setWidgetValue(QString key, const QVariant &value)
{
    const SimpleWidget *sw = widgetByKey(key);
    if (!sw || value.isNull()) return;

    switch (sw->data_type)
    {
	case sdtFilePath:
	case sdtDirPath:
	case sdtString:
	case sdtIntLine:
	case sdtDoubleLine: 
	case sdtColor: 
	{
	    setLineValue(sw, value);
	    break;
	}
	case sdtBoolCombo: // index == 0 (false);   index == 1 (true)
	case sdtIntCombo: //поиск заданного значения value в имеющемся списке
	case sdtDoubleCombo: //поиск заданного значения value в имеющемся списке 
	case sdtStringCombo: //поиск заданного значения value в имеющемся списке 
	{
	    setComboValue(sw, value);
	    break;
	}
	case sdtBool: 
	{
	    if (sw->checkBox)
		sw->checkBox->setChecked(value.toBool());
	    break;
	}
	default: break;
    }
}
void LSimpleDialog::setLineValue(const SimpleWidget *sw, const QVariant &value)
{
    if (!sw->edit) return;

    bool ok;
    switch (sw->data_type)
    {
	case sdtFilePath:
	case sdtDirPath:
	case sdtString:
	{
	    sw->edit->setText(value.toString());
	    break;
	}
	case sdtColor:
	{
	    //QColor c = LStatic::strToColor(value.toString());
	    sw->edit->setText(value.toString());

	    //QPalette palette;
	    //palette.setBrush(QPalette::Background, QBrush(c));
	    //sw->edit->setPalette(palette);
	    break;
	}
	case sdtIntLine:
	{
	    int v = value.toInt(&ok);	
	    if (!ok) qWarning()<<QString("LSimpleDialog::setLineValue  ERR set int value (%1),  result !ok").arg(value.toString());
	    else sw->edit->setText(QString::number(v));
	    break;
	}
	case sdtDoubleLine: 
	{
	    double v = value.toDouble(&ok);	
	    if (!ok) qWarning()<<QString("LSimpleDialog::setLineValue  ERR set double value (%1),  result !ok").arg(value.toString());
	    else sw->edit->setText(QString::number(v, 'f', sw->precision));
	    break;
	}
	default: break;
    }
}
void LSimpleDialog::setComboValue(const SimpleWidget *sw, const QVariant &value)
{
    if (!sw->comboBox) return;

    bool ok;
    bool need_find = true;	
    QString find_text;
    switch (sw->data_type)
    {
	case sdtBoolCombo:
	{
	    need_find = false;	
	    bool v = value.toBool();
	    if (sw->comboBox->count() > 1)
		sw->comboBox->setCurrentIndex(v ? 1 : 0);
	    break;
	}
	case sdtIntCombo:
	{
	    int v = value.toInt(&ok);	
	    if (!ok) 
	    {
		qWarning()<<QString("LSimpleDialog::setComboValue  ERR set int value (%1),  result !ok").arg(value.toString()); 
		return;
	    }
	    find_text = QString::number(v);
	    break;
	}
	case sdtDoubleCombo:
	{
	    double v = value.toDouble(&ok);	
	    if (!ok) 
	    {
		qWarning()<<QString("LSimpleDialog::setComboValue  ERR set double value (%1),  result !ok").arg(value.toString()); 
		return;
	    }
	    find_text = QString::number(v, 'f', sw->precision);
	    break;
	}
	case sdtStringCombo: 
	{
	    find_text = value.toString();
	    break;
	}
	default: break;
    }

    if (need_find)
    {
	int pos = sw->comboBox->findText(find_text);
	if (pos >= 0) sw->comboBox->setCurrentIndex(pos);
    }
}
void LSimpleDialog::setComboList(QString key, const QVariantList &combo_data, QVariantList user_data)
{
    const SimpleWidget *sw = widgetByKey(key);
    if (!sw) return;
    if (!sw->comboBox) return;

    sw->comboBox->clear();

    int n_data = combo_data.count();
    if (n_data < 2) return;


    bool ok;
    QStringList s_list;
    QString s_value;
    for (int i=0; i<n_data; i++)
    {
	const QVariant &value = combo_data.at(i);
	s_value = value.toString();
	switch (sw->data_type)
	{
	    case sdtIntCombo:
	    {
		int v = value.toInt(&ok);	
		if (!ok) 
		{
		    qWarning()<<QString("LSimpleDialog::setComboList  ERR convert int value (%1),  result !ok").arg(value.toString()); 
		    return;
		}
		s_value = QString::number(v);
		break;
	    }
	    case sdtDoubleCombo: 
	    {
		double v = value.toDouble(&ok);	
		if (!ok) 
		{
		    qWarning()<<QString("LSimpleDialog::setComboList  ERR convert double value (%1),  result !ok").arg(value.toString()); 
		    return;
		}
		s_value = QString::number(v, 'f', sw->precision);
		break;
	    }
	    default: break;	
	}
	s_list.append(s_value);
    }

    if (s_list.isEmpty()) return;


    bool need_data = (user_data.count() == s_list.count());
    
    for (int i=0; i<s_list.count(); i++)
	if (need_data) sw->comboBox->addItem(s_list.at(i), user_data.at(i));
	else sw->comboBox->addItem(s_list.at(i));

}





