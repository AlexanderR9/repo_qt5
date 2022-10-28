#ifndef DIANA_VIEW_WIDGET_H
#define DIANA_VIEW_WIDGET_H

#include "lsimplewidget.h"


class QSettings;
class LXMLPackView;
class DianaObject;
class MQ;


// DianaViewWidget
class DianaViewWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    DianaViewWidget(const QString&, QWidget *parent = 0);
    virtual ~DianaViewWidget() {}


    void loadMQPacket(const QString&); //загрузить указанный пакет
    QString name() const {return objectName();}
    void setExpandLevel(int);
    void setDoublePrecision(quint8);
    void updateMQState();


    //void load(QSettings&);
    //void save(QSettings&);

protected:
    LXMLPackView            *m_inView;
    LXMLPackView            *m_outView;
    DianaObject             *m_dianaObj;

    void initWidget();

    void loadPack(LXMLPackView*, const QString&);

signals:
    void signalMQCreated(const QString&, quint32, const MQ*);



};




#endif //DIANA_VIEW_WIDGET_H

