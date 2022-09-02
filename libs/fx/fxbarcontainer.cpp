#include "fxbarcontainer.h"
#include "lstatic.h"
#include "lfile.h"
#include "fxenums.h"

#include <QDebug>


//FXBarContainer
FXBarContainer::FXBarContainer(const QString &fname, QObject *parent)
    :LSimpleObject(parent),
    m_fileName(fname.trimmed())
{
    reset();
}
void FXBarContainer::reset()
{
    clearData();
    m_timeframe = -1;
    m_couple.clear();
    m_digist = 2;
}
void FXBarContainer::clearData()
{
    //qDeleteAll(m_data);
    m_data.clear();
}
void FXBarContainer::tryLoadData(QString sep_values)
{
    reset();
    checkFileName();
    if (invalid()) return;

    emit signalMsg(QString("Try load datafile: %1 ......").arg(m_fileName));
    QStringList list;
    QString err = LFile::readFileSL(m_fileName, list);
    if (!err.isEmpty()) {emit signalError(err); return;}

    int invalid_bars = 0;
    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        if (s.isEmpty()) continue;

        FXBar bar(m_digist);
        bar.fromFileLine(s, sep_values);
        if (bar.invalid()) invalid_bars++;
        else m_data.append(bar);
    }

    emit signalMsg(toStr());
}
void FXBarContainer::checkFileName()
{
    if (m_fileName.isEmpty())
    {
        emit signalError("FXBarContainer: datafile name is empty");
        return;
    }

    QString ftype = QString(".%1").arg(FXBarContainer::dataFileFormat());
    QString short_name = LFile::shortDirName(m_fileName);
    if (short_name.right(ftype.length()) != ftype)
    {
        emit signalError(QString("FXBarContainer: datafile(%1) is not [%1] type").arg(m_fileName).arg(ftype));
        return;
    }
    if (!LFile::fileExists(m_fileName))
    {
        emit signalError(QString("FXBarContainer: datafile(%1) not found").arg(m_fileName));
        return;
    }

    QString s = LStatic::strTrimRight(short_name, ftype.length());
    QStringList list = LStatic::trimSplitList(s, QString("_"));
    if (list.count() != 2)
    {
        emit signalError(QString("FXBarContainer: datafile(%1) invalid format, must be: couplename_timeframe.%2").arg(short_name).arg(FXBarContainer::dataFileFormat()));
        return;
    }

    bool ok;
    m_couple = list.first().trimmed();
    m_timeframe = list.at(1).toInt(&ok);
    if (!ok || !FXEnumStaticObj::timeFrames().contains(m_timeframe))
    {
        m_timeframe = -1;
        emit signalError(QString("FXBarContainer: datafile(%1) invalid timeframe %1").arg(short_name).arg(list.at(1)));
        return;
    }

    m_digist = 99;
    if (FXEnumStaticObj::invalidCouple(m_couple))
    {
        emit signalError(QString("FXBarContainer: couple(%1) is invalid, data file: %2").arg(m_couple).arg(short_name));
        return;
    }
    m_digist = FXEnumStaticObj::digistByCouple(m_couple);
    if (m_digist > 8)
        emit signalError(QString("FXBarContainer: invalid digist(%1) for couple %2").arg(m_digist).arg(m_couple));

}
bool FXBarContainer::invalid() const
{
    return (m_couple.isEmpty() || m_timeframe <= 0 || m_digist > 8);
}
QString FXBarContainer::toStr() const
{
    QString s("FXBarContainer:");
    s = QString("%1 couple=%2  timeframe=%3  digist=%4").arg(s).arg(m_couple).arg(m_timeframe).arg(m_digist);
    s = QString("%1  container_size=%2").arg(s).arg(m_data.count());
    return s;
}


