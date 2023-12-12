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
    inline void setDebugLevel(quint8 level) {m_debugLevel = level;}
    virtual int userSign() const {return m_userSign;} //некий вспомогательный признак, может не использоваться

protected:
    quint8 m_debugLevel;
    int    m_userSign; //default: -1

signals:
    void signalError(const QString&);
    void signalMsg(const QString&);

};


#endif // LSIMPLEOBJ_H


