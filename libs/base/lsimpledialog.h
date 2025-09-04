 #ifndef LSIMPLEDIALOG_H
 #define LSIMPLEDIALOG_H

 #include "ui_lsimpledialog.h"

 #include <QDialog>
 #include <QList>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QToolButton;
class QLabel;




////////////////LSimpleDialog///////////////////////////////////////////
class LSimpleDialog : public QDialog, public Ui::LSimpleDialog
{
    Q_OBJECT
public:
    enum SimpleDataTypes {sdtString = 701, sdtIntLine, sdtDoubleLine, sdtStringCombo, sdtIntCombo, sdtDoubleCombo, 
			    sdtFilePath, sdtDirPath, sdtBool, sdtBoolCombo, sdtColor};

    struct SimpleWidget
    {
    	SimpleWidget() :label(NULL), edit(NULL), comboBox(NULL), checkBox(NULL), button(NULL), caption(QString()), data_type(sdtString), precision(2) {}
    	SimpleWidget(QString s, int t) :label(NULL), edit(NULL), comboBox(NULL), checkBox(NULL), button(NULL), caption(s), data_type(t), precision(2) {}

	QLabel			*label;
    	QLineEdit		*edit;
    	QComboBox		*comboBox;
	QCheckBox		*checkBox;
	QToolButton		*button;

    	QString			 caption;
    	int 			 data_type; //тип данных, в зависимости от него виджет преобретает тот или иной вид
	QString			 key; //уникальное слово виджета для поиска
	int 			 precision; //влияет только на вещественный тип	

	bool invalid() const {return (!edit && !comboBox && !checkBox);}
    };

    LSimpleDialog(QWidget *parent = 0);
    virtual ~LSimpleDialog() {}

    inline bool isApply() const {return m_apply;}
    inline int count() const {return m_widgets.count();}
    
    void addSimpleWidget(QString caption, int dataType, QString key, int precision = 0);
    void setComboList(QString, const QVariantList&, QVariantList data = QVariantList()); //key, data items for combobox, данные могут представлять из себя: QStringList, QList<int>, QList<double>
    void setBoxTitle(QString);
    void setWidgetValue(QString, const QVariant&); //key, value
    QVariant widgetValue(QString) const; //key
    

    void setSizes(int, int);
    void setCaptionsWidth(int);
    void setExpandWidgets();



protected slots:
    virtual void slotApply() {m_apply = true; close();}
    virtual void slotOpenPath();
    virtual void slotOpenColorDialog();

protected:
    QList<SimpleWidget> m_widgets;
    bool m_apply;
//    int m_captionsWidth;

    const SimpleWidget* widgetByKey(QString) const; //key
    void placeSimpleWidget(const SimpleWidget&);
    void addVerticalSpacer();
    void addLineSeparator(quint8 w = 1, QString color = "#696969");
    virtual void save() {}
    virtual void closeEvent(QCloseEvent*) {save();}

    
private:
    void setLineValue(const SimpleWidget*, const QVariant&);
    void setComboValue(const SimpleWidget*, const QVariant&);
    void initSimpleWidgetPath(SimpleWidget&);
    void initSimpleWidgetColor(SimpleWidget&);
    void initSimpleWidgetLine(SimpleWidget&);
    void initSimpleWidgetCombo(SimpleWidget&);


};





 #endif



