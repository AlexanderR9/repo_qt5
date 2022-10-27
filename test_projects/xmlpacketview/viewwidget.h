#ifndef VIEW_WIDGET_H
#define VIEW_WIDGET_H

#include "lsimplewidget.h"

#include <QMap>


class FXDataLoader;
class QStackedWidget;
class FXDataLoaderWidget;
class QListWidget;
class QSettings;
struct FXCoupleDataParams;
class FXBarContainer;
struct FXChartSettings;
class LXMLPackView;
class LProtocolBox;



// ViewWidget
class ViewWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    ViewWidget(QWidget *parent = 0);
    virtual ~ViewWidget() {}

    void load(QSettings&);
    void save(QSettings&);

    void loadInPack(const QString&);
    void loadOutPack(const QString&);
    void toLeft(int, bool); //переслать пакет из правой вьюхи в левую
    void toRight(int, bool); //переслать пакет из левой вьюхи в правую
    void setDoublePrecision(quint8);

protected:
    LXMLPackView            *m_inView;
    LXMLPackView            *m_outView;
    LProtocolBox            *m_protocol;

    void initWidget();
    void loadPack(LXMLPackView*, const QString&);
    void setPacketData(LXMLPackView*, const QByteArray&, int, bool);

public slots:
    void slotError(const QString&);
    void slotMessage(const QString&);

};




#endif //VIEW_WIDGET_H

