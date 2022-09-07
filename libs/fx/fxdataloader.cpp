#include "fxdataloader.h"
#include "fxbarcontainer.h"
#include "lfile.h"

#include <QDebug>


//FXDataLoader
FXDataLoader::FXDataLoader(QObject *parent)
    :LSimpleObject(parent),
    m_sourceDir(QString())
{

}
void FXDataLoader::reloadData()
{
    clearData();

    if (m_sourceDir.trimmed().isEmpty())
    {
        emit signalError(QString("FXDataLoader: source dir name is empty"));
        return;
    }
    if (!LFile::dirExists(m_sourceDir))
    {
        emit signalError(QString("FXDataLoader: source dir [%1] not found").arg(m_sourceDir));
        return;
    }

    QStringList list;
    QString err = LFile::dirFiles(m_sourceDir, list, FXBarContainer::dataFileFormat());
    if (!err.isEmpty())
    {
        emit signalError(QString("FXDataLoader: %1").arg(err));
        return;
    }

    //load files
    for (int i=0; i<list.count(); i++)
        tryLoadFile(list.at(i));

    emit signalMsg(QString("FXDataLoader: loaded valid %1 files").arg(count()));
}
void FXDataLoader::tryLoadFile(const QString &fname)
{
    qDebug()<<QString("FXDataLoader::tryLoadFile  fname: [%1]").arg(fname);
    FXBarContainer *container = new FXBarContainer(fname, this);
    connect(container, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(container, SIGNAL(signalMsg(const QString&)), this, SIGNAL(signalMsg(const QString&)));

    container->tryLoadData();
    if (container->invalid())
    {
        qWarning()<<QString("     WARNING: invalid loading file %1").arg(fname);
        qDebug()<<QString("   ")<<container->toStr();
        delete container;
        container = NULL;
        return;
    }

    m_data.append(container);
    qWarning()<<QString("loaded ok, bar count %1").arg(container->barCount());
}
void FXDataLoader::clearData()
{
    qDeleteAll(m_data);
    m_data.clear();
}
const FXBarContainer* FXDataLoader::containerAt(int i) const
{
    if (i < 0 || i >= count()) return NULL;
    return m_data.at(i);
}
const FXBarContainer* FXDataLoader::container(const QString &name, int t) const
{
    foreach (FXBarContainer *v, m_data)
        if (v->couple() == name && v->timeframe() == t) return v;

    return NULL;
}



