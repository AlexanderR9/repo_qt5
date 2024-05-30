#ifndef BB_SHADOWEXPLORER_H
#define BB_SHADOWEXPLORER_H


#include "bb_basepage.h"



//BB_ShadowExplorer
class BB_ShadowExplorer : public BB_BasePage
{
    Q_OBJECT
public:
    BB_ShadowExplorer(QWidget*);
    virtual ~BB_ShadowExplorer() {}

    QString iconPath() const {return QString(":/icons/images/candle.png");}
    QString caption() const {return QString("Shadow explorer");}

    void updateDataPage(bool forcibly = false);


public slots:
    void slotJsonReply(int, const QJsonObject&);


};



#endif // BB_SHADOWEXPLORER_H
