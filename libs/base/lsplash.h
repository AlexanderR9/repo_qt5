#ifndef LSPLASH_H
#define LSPLASH_H

#include <QWidget>

class QLabel;

///////////LSplash//////////////////////////
class LSplash: public QWidget
{
    Q_OBJECT
public:
    LSplash(QWidget *parent = 0);
    virtual ~LSplash() {}

    void startDelay(const QString &s = QString());
    void stopDelay();

protected:
    QLabel *m_label;


};



#endif // LSPLASH_H
