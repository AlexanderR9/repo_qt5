#ifndef GBOT_PAGE_H
#define GBOT_PAGE_H

#include "lsimplewidget.h"

#include <QTime>


class QLineEdit;
class QComboBox;
class QGroupBox;
class QSettings;
class GBotObj;


//GBotPage
class GBotPage : public LSimpleWidget
{
    Q_OBJECT
public:
    GBotPage(QWidget*);
    virtual ~GBotPage() {reset();}

    virtual void load(QSettings&);
    virtual void save(QSettings&);

    void exec();

protected:
    QLineEdit *m_inputSumEdit;
    QComboBox *m_leverageEdit;
    QLineEdit *m_gridCountEdit;
    QLineEdit *m_rangeEdit;
    QLineEdit *m_curPriceEdit;
    QComboBox *m_directionEdit;
    LTableWidgetBox *m_table;
    GBotObj *m_bot;

    void initPage();
    void initParamBox(QGroupBox*);
    void reset();
    void setBotParams();

private:
    void parseRange(QPair<float, float>&);

protected slots:
    void slotReceivedTableResult(const QStringList&);

};


#endif // GBOT_PAGE_H
