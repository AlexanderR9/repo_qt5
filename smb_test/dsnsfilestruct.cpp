#include "dsnsfilestruct.h"

#include <QFile>
#include <QTextCodec>
#include <QDir>
#include <QDebug>
#include <QDataStream>


#define DATA_PATH           QString("data")
#define FILE_TYPE           QString("pc3")
#define MONIT_PARAMS_FILE   QString("smb_params.txt") //описание эмулируемых параметров, своего рода конфиг имитатора
#define SPACE_SYMBOL        QString(" ")


// в данной системе эмуляции на один параметр требуется группа из 5-ти файлов, в этой группе можно описать хоть 1, хоть 100 параметров,
// в каждом файле лежит свой тип информации о параметрах, во всех файлах должен быть одинаковый (по количеству) набор значений,
// т.е. для одного (любого) параметра можно получить информацию из 5-ти файлов по одному и тому же смещению в каждом файле.
// все 5 файлов генерятся 1 раз при старте имитатора, далее с определенным интервалом
// перегенириться только файл со значениями параметров (VAL_AFFIX).
#define PID_AFFIX       QString("ZPID")     //окончание имени файла со значениями KKS (datatype: fdtString)
#define SYN_AFFIX       QString("ZSYN2")    //окончание имени файла со алтернативными значениями  KKS (datatype: fdtString)
#define DESC_AFFIX      QString("ZDESC")    //окончание имени файла с кратким описанием параметров (datatype: fdtString)
#define VAL_AFFIX       QString("VAL")      //окончание имени файла с числовыми значениями параметров (datatype: fdtFloat/fdtInt)
#define QUAL_AFFIX      QString("QVAL")     //окончание имени файла с кодами достоверности значений параметров (datatype: fdtInt)


//// DSNSFileStruct
DSNSFileStruct::DSNSFileStruct(QObject *parent)
    :QObject(parent),
      m_codec(NULL),
      m_emulDir(QString())
{
    m_codec = QTextCodec::codecForName("Windows-1251");
    if (!m_codec) qWarning("ERROR: text_codec is NULL");

    loadMonitParams();
}
bool DSNSFileStruct::hasMonitKKS(QString kks, QString &orig_kks) const
{
    orig_kks.clear();
    kks = kks.trimmed();
    if (m_params.isEmpty() || kks.isEmpty()) return false;

    for (int i=0; i<m_params.count(); i++)
        if (kks.indexOf(m_params.at(i).kks) == 0) {orig_kks = m_params.at(i).kks; return true;}

    return false;
}
void DSNSFileStruct::loadMonitParams()
{
    m_params.clear();

    //read f_data
    QString f_name = QString("%1%2%3").arg(DATA_PATH).arg(QDir::separator()).arg(MONIT_PARAMS_FILE);
    QFile f(f_name);
    if (!f.exists())
    {
        qWarning()<<QString("DSNSFileStruct::loadMonitParams   file(%1) not found").arg(f_name);
        return;
    }
    if (!f.open(QIODevice::ReadOnly))
    {
        qWarning()<<QString("DSNSFileStruct::loadMonitParams   file(%1) can't open").arg(f_name);
        return;
    }

    QByteArray ba(f.readAll());
    QString f_data(ba);
    f.close();

    //parse f_data
    bool ok;
    QStringList list = f_data.split(QString("\n"));
    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();

        int pos = s.indexOf("#");
        if (pos >= 0) s = s.left(pos);
        if (s.isEmpty() || !s.contains(";")) continue;

        pos = s.indexOf(";");
        if (pos < 3) continue;
        QString kks = s.left(pos);
        QString value = s.right(s.length()-(kks.length()+1));
        if (kks.isEmpty() || value.isEmpty()) continue;

        //create param
        MonitParam param(kks);
        if (value.length() == 1)
        {
            param.datatype = fdtInt;
            param.def_value = value.toInt(&ok);
            if (!ok)
            {
                qWarning()<<QString("DSNSFileStruct::loadMonitParams()  WARNING invalid int value [%1]").arg(value);
                continue;
            }
        }
        else
        {
            param.datatype = fdtFloat;
            param.def_value_f = value.toFloat(&ok);
            if (!ok)
            {
                qWarning()<<QString("DSNSFileStruct::loadMonitParams()  WARNING invalid float value [%1]").arg(value);
                continue;
            }
        }
        m_params.append(param);
    }

    qDebug()<<QString("loading monit params: %1").arg(m_params.count());
    for (int i=0; i<m_params.count(); i++)
        qDebug()<<QString("Param_%1: %2").arg(i+1).arg(m_params.at(i).toStr());

}
QString DSNSFileStruct::checkEmulDir() const
{
    QString err;
    if (m_emulDir.trimmed().isEmpty()) err = QString("Emulation folder is empty.");
    else
    {
        QDir dir(m_emulDir.trimmed());
        if (!dir.exists())
            err = QString("Emulation folder [%1] not found.").arg(dir.path());
    }
    return err;
}
void DSNSFileStruct::initEmulHeader(DSNSFileHeader &header, QDateTime &dt)
{
    //datetime fields
    header.sec = dt.time().second();
    header.min = dt.time().minute();
    header.hour = dt.time().hour();
    header.day = dt.date().day();
    header.month = dt.date().month();
    header.year = dt.date().year();
    header.week_day = dt.date().dayOfWeek();
    header.year_day = dt.date().dayOfYear();

    //other fields
    header.summer_time = 0;
    header.server_name_and_status = 20545;
    header.reserve_2 = 0;
    header.checksum = 0;
    header.write_mode = 0;
    header.file_size = m_params.count()*header.elemsize;
}


////////////////GENERATION FLOAT FILES/////////////////////////////
void DSNSFileStruct::genFloatFile_PID()
{
    //init header
    DSNSFileHeader header(fdtString, 16);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QStringList list;
    QString f_name;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isFloat()) continue;
        if (f_name.isEmpty())
            f_name = QString("%1-%2").arg(p.smbPrefixFile()).arg(PID_AFFIX);

        QString kks = p.kks;
        DSNSFileStruct::trimStrDataByType(kks, header.elemsize);
        list.append(kks);
    }

    writeFileStringData(f_name, list, header);
}
void DSNSFileStruct::genFloatFile_SYN()
{
    //init header
    DSNSFileHeader header(fdtString, 16);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QStringList list;
    QString f_name;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isFloat()) continue;
        if (f_name.isEmpty())
            f_name = QString("%1-%2").arg(p.smbPrefixFile()).arg(SYN_AFFIX);

        QString kks = QString("%1_SYN2").arg(p.kks);
        DSNSFileStruct::trimStrDataByType(kks, header.elemsize);
        list.append(kks);
    }

    writeFileStringData(f_name, list, header);
}
void DSNSFileStruct::genFloatFile_DESC()
{
    //init header
    DSNSFileHeader header(fdtString, 60);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QStringList list;
    QString f_name;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isFloat()) continue;
        if (f_name.isEmpty())
            f_name = QString("%1-%2").arg(p.smbPrefixFile()).arg(DESC_AFFIX);

        QString desc = QString("%1_DESC").arg(p.kks);
        DSNSFileStruct::trimStrDataByType(desc, header.elemsize);
        list.append(desc);
    }

    writeFileStringData(f_name, list, header);
}
void DSNSFileStruct::genFloatFile_VAL()
{
    //init header
    DSNSFileHeader header(fdtFloat, 4);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QString f_name;
    QList<float> list;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isFloat()) continue;
        if (f_name.isEmpty()) f_name = p.smbPrefixFile();
        double r = double(qrand()%99)/100;
        double d = p.def_value_f*0.01*r*((i%2==0)?1:-1);
        list.append(p.def_value_f+d);
    }

    //write header to byte array
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    header.file_size = list.count()*header.elemsize;
    qDebug()<<header.toStr();

    header.toStream(stream);
    for (int i=0; i<list.count(); i++) stream << list.at(i);

    //write ba to smb_file
    f_name = QString("%1-%2.%3").arg(f_name).arg(VAL_AFFIX).arg(FILE_TYPE);
    qDebug()<<QString("write file: [%1],  data count %2  ba_size=%3").arg(f_name).arg(list.count()).arg(ba.size());
    writeFile(f_name, ba);
}
void DSNSFileStruct::genFloatFile_QUAL()
{
    //init header
    DSNSFileHeader header(fdtInt, 1);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QString f_name;
    QList<qint8> list;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isFloat()) continue;
        if (f_name.isEmpty())
            f_name = QString("%1-%2").arg(p.smbPrefixFile()).arg(QUAL_AFFIX);

        list.append(qint8(dqValid));
    }

    writeFileIntData(f_name, list, header);
}

////////////////GENERATION INTEGER FILES/////////////////////////////
void DSNSFileStruct::genIntFile_PID()
{
    //init header
    DSNSFileHeader header(fdtString, 16);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QStringList list;
    QString f_name;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isInteger()) continue;
        if (f_name.isEmpty())
            f_name = QString("%1-%2").arg(p.smbPrefixFile()).arg(PID_AFFIX);

        QString kks = p.kks;
        DSNSFileStruct::trimStrDataByType(kks, header.elemsize);
        list.append(kks);
    }

    writeFileStringData(f_name, list, header);
}
void DSNSFileStruct::genIntFile_SYN()
{
    //init header
    DSNSFileHeader header(fdtString, 16);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QStringList list;
    QString f_name;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isInteger()) continue;
        if (f_name.isEmpty())
            f_name = QString("%1-%2").arg(p.smbPrefixFile()).arg(SYN_AFFIX);

        QString kks = QString("%1_SYN2").arg(p.kks);
        DSNSFileStruct::trimStrDataByType(kks, header.elemsize);
        list.append(kks);
    }

    writeFileStringData(f_name, list, header);
}
void DSNSFileStruct::genIntFile_DESC()
{
    //init header
    DSNSFileHeader header(fdtString, 60);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QStringList list;
    QString f_name;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isInteger()) continue;
        if (f_name.isEmpty())
            f_name = QString("%1-%2").arg(p.smbPrefixFile()).arg(DESC_AFFIX);

        QString desc = QString("%1_DESC").arg(p.kks);
        DSNSFileStruct::trimStrDataByType(desc, header.elemsize);
        list.append(desc);
    }

    writeFileStringData(f_name, list, header);
}
void DSNSFileStruct::genIntFile_VAL()
{
    //init header
    DSNSFileHeader header(fdtInt, 1);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QString f_name;
    QList<qint8> list;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isInteger()) continue;
        if (f_name.isEmpty())
            f_name = QString("%1-%2").arg(p.smbPrefixFile()).arg(VAL_AFFIX);

        double r = double(qrand()%99)/100;
        qint8 cur_v = p.def_value;
        if (r < 0.1 && cur_v != 0) cur_v = 0;
        else if (r < 0.1 && cur_v == 0) cur_v = 1;
        else cur_v = 0;
        list.append(cur_v);
        m_params[i].def_value = cur_v;
    }

    writeFileIntData(f_name, list, header);
}
void DSNSFileStruct::genIntFile_QUAL()
{
    //init header
    DSNSFileHeader header(fdtInt, 1);
    QDateTime dt(QDateTime::currentDateTime());
    initEmulHeader(header, dt);

    //write data to file
    QString f_name;
    QList<qint8> list;
    for (int i=0; i<m_params.count(); i++)
    {
        const MonitParam &p = m_params.at(i);
        if (!p.isInteger()) continue;
        if (f_name.isEmpty())
            f_name = QString("%1-%2").arg(p.smbPrefixFile()).arg(QUAL_AFFIX);

        list.append(qint8(dqValid));
    }

    writeFileIntData(f_name, list, header);
}


//service funcs
void DSNSFileStruct::trimStrDataByType(QString &value, qint8 need_size)
{
    if (value.length() > need_size)
    {
        value = value.left(need_size);
        return;
    }

    if (value.length() < need_size)
        value.append(QString(need_size-value.length(), QChar(' ')));
}
void DSNSFileStruct::writeFileIntData(QString f_type, const QList<qint8> &data, DSNSFileHeader &header)
{
    //write header to byte array
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    header.file_size = data.count()*header.elemsize;
    qDebug()<<header.toStr();
    header.toStream(stream);
    for (int i=0; i<data.count(); i++) stream << data.at(i);

    //write ba to smb_file
    QString f_name = QString("%1.%2").arg(f_type).arg(FILE_TYPE);
    qDebug()<<QString("write file: [%1],  data count %2  ba_size=%3").arg(f_name).arg(data.count()).arg(ba.size());
    writeFile(f_name, ba);
}
void DSNSFileStruct::writeFileStringData(QString f_type, const QStringList &data, DSNSFileHeader &header)
{
    //write header to byte array
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    header.file_size = data.count()*header.elemsize;
    qDebug()<<header.toStr();
    header.toStream(stream);
    for (int i=0; i<data.count(); i++)
        ba.append(m_codec->fromUnicode(data.at(i)));

    //write ba to smb_file
    QString f_name = QString("%1.%2").arg(f_type).arg(FILE_TYPE);
    qDebug()<<QString("write file: [%1],  data count %2  ba_size=%3").arg(f_name).arg(data.count()).arg(ba.size());
    writeFile(f_name, ba);
}
void DSNSFileStruct::writeFile(const QString &f_name, const QByteArray &ba)
{
    QFile f(QString("%1%2%3").arg(m_emulDir).arg(QDir::separator()).arg(f_name));
    if (!f.open(QIODevice::WriteOnly))
    {
        qWarning()<<QString("DSNSFileStruct::writeFile   file(%1) can't open for writing").arg(f.fileName());
        return;
    }

    f.write(ba.data(), ba.size());
    f.close();
}
void DSNSFileStruct::tryGenFiles()
{
    if (m_params.isEmpty())
    {
        qWarning("WARNING: m_params.isEmpty()");
        return;
    }

    //gen float groups files
    genFloatFile_PID();
    genFloatFile_SYN();
    genFloatFile_DESC();
    genFloatFile_VAL();
    genFloatFile_QUAL();
    return;

    //gen int groups files
    genIntFile_PID();
    genIntFile_SYN();
    genIntFile_DESC();
    genIntFile_VAL();
    genIntFile_QUAL();
}
void DSNSFileStruct::nextEmulValues()
{
    genFloatFile_VAL();
    //genIntFile_VAL();
}
void DSNSFileStruct::tryReadComplectFiles()
{
    kks_positions.clear();
    QDir dir(DATA_PATH);
    if (!dir.exists())
    {
        qWarning()<<QString("DSNSFileStruct::tryReadComplectFiles  DATA_PATH(%1) not found").arg(DATA_PATH);
        return;
    }

    QStringList list = dir.entryList();
    if (list.isEmpty())
    {
        qWarning()<<QString("DSNSFileStruct::tryReadComplectFiles  files list is empty");
        return;
    }

    int n = list.count();
    for (int i=n-1; i>=0; i--)
    {
        QString s = list.at(i).trimmed();
        if (s.length() < 6) continue;
        if (!s.contains(QString(".%1").arg(FILE_TYPE))) continue;

        QByteArray ba;
        s = QString("%1%2%3").arg(DATA_PATH).arg(QDir::separator()).arg(s);
        readFile(s, ba);
        parseFileData(ba);
    }
    qDebug()<<QString("finded kks: %1").arg(kks_positions.count());
}
void DSNSFileStruct::readFile(const QString &f_name, QByteArray &ba)
{
    ba.clear();
    qDebug()<<QString("try read file: %1").arg(f_name);

    QFile f(f_name);
    if (!f.exists())
    {
        qWarning()<<QString("DSNSFileStruct::readFile   file(%1) not found").arg(f_name);
        return;
    }
    if (!f.open(QIODevice::ReadOnly))
    {
        qWarning()<<QString("DSNSFileStruct::readFile   file(%1) can't open").arg(f_name);
        return;
    }

    ba.append(f.readAll());
    f.close();

    qDebug()<<QString("OK, readed %1 bytes.").arg(ba.size());
}
void DSNSFileStruct::parseFileData(QByteArray &ba)
{
    DSNSFileHeader header;
    if (ba.size() < header.size())
    {
        qWarning()<<QString("DSNSFileStruct::parseFileData WARNING:  ba(%1) < head_size(%2)").arg(ba.size()).arg(header.size());
        return;
    }

    readHeader(ba, header);
    qDebug()<<header.toStr();
    qDebug()<<QString("ba size after header: %1 bytes.").arg(ba.size());

    readData(ba, header);
}
void DSNSFileStruct::parseStringData(const QByteArray &ba, qint8 rec_size)
{
    int n = 0;
    int start_pos = DSNSFileHeader::headerSize();
    QStringList data_list;
    while ((start_pos + rec_size) < ba.size())
    {
        n++;
        QString value = m_codec->toUnicode(ba.mid(start_pos, rec_size));
        data_list.append(value.trimmed());
        start_pos += rec_size;
    }
    qDebug()<<QString("readed %1 string values").arg(n);

    for (int i=0; i<5; i++)
        qDebug()<<data_list.at(i);
}
void DSNSFileStruct::parseFloatData(const QByteArray &ba, qint8 rec_size)
{
    int n = 0;
    int start_pos = DSNSFileHeader::headerSize();
    QDataStream stream(ba);
    stream.setByteOrder(QDataStream::LittleEndian);
    qint8 a;
    for (int i=0; i<start_pos; i++) stream >> a;

    QList<float> data_list;
    float v = -1;
    while ((start_pos + rec_size) < ba.size())
    {
        switch (rec_size)
        {
            case 4:
            {
                stream >> v;
                data_list.append(v);
                break;
            }
            default:
            {
                qWarning()<<QString("DSNSFileStruct::parseFloatData: WARNING - unknown rec_size=%1").arg(rec_size);
                break;
            }
        }
        start_pos += rec_size;
        n++;
    }
    qDebug()<<QString("readed %1 float values").arg(n);

    for (int i=0; i<5; i++)
        qDebug()<<QString("float_value[%1]=%2").arg(i).arg(QString::number(data_list.at(i), 'f', 4));
}
void DSNSFileStruct::parseIntegerData(const QByteArray &ba, qint8 rec_size)
{
    int n = 0;
    int start_pos = DSNSFileHeader::headerSize();
    QList<qint8> data_list;

    while ((start_pos + rec_size) < ba.size())
    {
        switch (rec_size)
        {
            case 1:
            {
                data_list.append(qint8(ba.at(start_pos)));
                break;
            }
            default:
            {
                qWarning()<<QString("DSNSFileStruct::parseIntegerData: WARNING - unknown rec_size=%1").arg(rec_size);
                break;
            }
        }
        start_pos += rec_size;
        n++;
    }
    qDebug()<<QString("readed %1 int values").arg(n);

    for (int i=0; i<5; i++)
        qDebug()<<QString("int_value[%1]=%2").arg(i).arg(data_list.at(i));
}
void DSNSFileStruct::readData(const QByteArray &ba, const DSNSFileHeader &header)
{
    switch (header.datatype)
    {
        case fdtString:
        {
            parseStringData(ba, header.elemsize);
            break;
        }
        case fdtFloat:
        {
            parseFloatData(ba, header.elemsize);
            break;
        }
        case fdtInt:
        {
            parseIntegerData(ba, header.elemsize);
            break;
        }
        default:
        {
            qWarning()<<QString("DSNSFileStruct::readData WARNING - unknown datatype %1").arg(header.datatype);
            break;
        }
    }
}
void DSNSFileStruct::readHeader(const QByteArray &ba, DSNSFileHeader &header)
{
    QDataStream stream(ba);
    stream.setByteOrder(QDataStream::LittleEndian);
    header.fromStream(stream);
}
void DSNSFileStruct::loadMonitParamsInfo()
{
    //QStringList keys(kks_positions.keys());
    for (int i=0; i<m_params.count(); i++)
    {
        QString kks = m_params.at(i).kks;
        if (!kks_positions.contains(kks)) continue;

        quint16 pos = kks_positions.value(kks);

        QString desc_file = QString("%1%2%3-%4.%5").arg(DATA_PATH).arg(QDir::separator()).arg(m_params.at(i).smbPrefixFile()).arg(DESC_AFFIX).arg(FILE_TYPE);
        QByteArray ba;
        readFile(desc_file, ba);
        DSNSFileHeader header;
        readHeader(ba, header);
        ba.remove(0, DSNSFileHeader::headerSize());
        m_params[i].desc = m_codec->toUnicode(ba.mid(pos*header.elemsize, header.elemsize)).trimmed();


        QString syn_file = QString("%1%2%3-%4.%5").arg(DATA_PATH).arg(QDir::separator()).arg(m_params.at(i).smbPrefixFile()).arg(SYN_AFFIX).arg(FILE_TYPE);
        readFile(syn_file, ba);
        header.reset();
        readHeader(ba, header);
        ba.remove(0, DSNSFileHeader::headerSize());
        m_params[i].syn2 = m_codec->toUnicode(ba.mid(pos*header.elemsize, header.elemsize)).trimmed();
    }
}


//// DSNSFileHeader
void DSNSFileHeader::reset()
{
    sec = min = day = month = year = -1;
    week_day = year_day = summer_time = file_size = checksum = write_mode = 0;
    server_name_and_status = -1;
    datatype = elemsize = reserve_2 = 0;
}
QString DSNSFileHeader::toStr() const
{
    QString s = QString("HEADER(size: %0)  ").arg(size());
    s.append(QString("%1:%2:%3  %4.%5.%6  ").arg(hour).arg(min).arg(sec).arg(day).arg(month).arg(year));
    s.append(QString("data_size=%1/%2  mode=%3  datatype=%4  ").arg(file_size).arg(elemsize).arg(write_mode).arg(strDatatype()));
    s.append(QString("status=%1  reserve=%2  crc=%3  ").arg(server_name_and_status).arg(reserve_2).arg(checksum));
    s.append(QString("summer_time=%1  week_day=%2  year_day=%3").arg(summer_time).arg(week_day).arg(year_day));

    return s;
}
int DSNSFileHeader::size() const
{
    int n = sizeof(sec);
    n += sizeof(min);
    n += sizeof(hour);
    n += sizeof(day);
    n += sizeof(month);
    n += sizeof(year);
    n += sizeof(week_day);
    n += sizeof(year_day);
    n += sizeof(summer_time);
    n += sizeof(file_size);
    n += sizeof(checksum);
    n += sizeof(write_mode);
    n += sizeof(server_name_and_status);
    n += sizeof(datatype);
    n += sizeof(elemsize);
    n += sizeof(reserve_2);
    return n;
}
void DSNSFileHeader::toStream(QDataStream &stream)
{
    stream << sec;
    stream << min;
    stream << hour;
    stream << day;
    stream << month;
    stream << year;
    stream << week_day;
    stream << year_day;
    stream << summer_time;
    stream << file_size;
    stream << checksum;
    stream << write_mode;
    stream << server_name_and_status;
    stream << datatype;
    stream << elemsize;
    stream << reserve_2;
}
void DSNSFileHeader::fromStream(QDataStream &stream)
{
    stream >> sec;
    stream >> min;
    stream >> hour;
    stream >> day;
    stream >> month;
    stream >> year;
    stream >> week_day;
    stream >> year_day;
    stream >> summer_time;
    stream >> file_size;
    stream >> checksum;
    stream >> write_mode;
    stream >> server_name_and_status;
    stream >> datatype;
    stream >> elemsize;
    stream >> reserve_2;
}
int DSNSFileHeader::headerSize()
{
    int n = sizeof(qint16)*13;
    n += sizeof(qint32);
    n += sizeof(qint8)*2;
    return n;
}

