 #ifndef LCOMMONSETTINGS_H
 #define LCOMMONSETTINGS_H

 #include "lsimpledialog.h"

 #include <QSettings>

class QResizeEvent;


////////////struct CommonSettings////////////////
struct LCommonSettingsBase
{
    struct LSettingsParam
    {
        LSettingsParam() :data_type(0), precision(2) {}
        LSettingsParam(QString caption, int t, QString k, QVariant dv, int p = 0) :data_type(t), name(caption), key(k), precision(p), def_value(dv) {}

        int data_type;
        QString name;
        QString key;
        int precision;
        QVariant value;
        QVariant def_value;
        QVariantList combo_list;

        bool invalid() const {return (name.trimmed().isEmpty() || key.trimmed().isEmpty() || data_type < LSimpleDialog::sdtString || data_type > LSimpleDialog::sdtColor);}
        QString toString() const {return QString("key %1,  value %2/%3,  data type %4, combo list size %5").arg(key).arg(value.toString()).arg(def_value.toString()).arg(data_type).arg(combo_list.count());}
    };
    
    LCommonSettingsBase();

    QList<LSettingsParam> params; 
    inline int paramsCount() const {return params.count();} 

    void load(QSettings&);
    void save(QSettings&);

    void addParam(QString caption, int t, QString k, int p = 0, QVariant dv = QVariant());
    void setDefValuesToNullParams();
    QVariant paramValue(QString) const;
    void setComboList(QString, const QStringList&);
    void setDefValue(QString, const QVariant&);
    void removeAt(QString);
    bool hasParam(QString) const;

};


////////////class CommonSettingsDialog////////////////
class LCommonSettingsDialog : public LSimpleDialog
{
    Q_OBJECT
public:
    LCommonSettingsDialog(LCommonSettingsBase&, QWidget *parent = 0);
    virtual ~LCommonSettingsDialog() {}

protected:
    LCommonSettingsBase &m_settings;
    void initWidgets();
    void resizeBySettings();
    void resizeEvent(QResizeEvent*);

protected slots:
    virtual void slotApply();

};

extern LCommonSettingsBase lCommonSettings;



 #endif



