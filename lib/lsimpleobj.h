#ifndef LSIMPLEOBJ_H
#define LSIMPLEOBJ_H


#include <QObject>
#include <QString>

class  LSimpleObject : public QObject
{
    Q_OBJECT
public:
    LSimpleObject(QObject *parent = NULL);
    virtual ~LSimpleObject() {}

    virtual QString name() const {return QString("simple_obj");}

signals:
    void signalError(const QString&);
    void signalMsg(const QString&);

};


#endif // LSIMPLEOBJ_H


