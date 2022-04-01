#ifndef FILEWORKER_H
#define FILEWORKER_H

#include "lsimpleobj.h"

#include <QByteArray>

class QStringList;


//FileWorker
class FileWorker : public LSimpleObject
{
    Q_OBJECT
public:
    FileWorker(QObject *parent = NULL);
    virtual ~FileWorker() {}

    virtual QString name() const {return QString("fworker_obj");}

    void setInputFile(const QString&);
    void saveBuffer(const QByteArray&);
    inline const QByteArray& sendingBuffer() const {return m_writeBuffer;}


protected:
    QByteArray m_writeBuffer;
    //QByteArray m_readBuffer;

    void parseInputFileData(const QStringList&);
    void parseFileLine(const QString&);
    QString saveFileName() const;

};




#endif // FILEWORKER_H
