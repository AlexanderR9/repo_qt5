#ifndef DSNSFILESTRUCT_H
#define DSNSFILESTRUCT_H

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QObject>
#include <QVector>
#include <QByteArray>


class QDataStream;


//что-то типа кода достоверности значений параметров, лежащих в файлах samba-сервера
enum DSNSQualilies
{
    dqStructError = -3,     //!< -3 (internal) DSNS structure for index is absent
    dqNotMapped = -2,       //!< -2 (internal) No DSNS param mapped DE param
    dqManualNoSign = 35,    //!< 35 Manual signalisation disabled (Valid)
    dqAutoNoSign = 36,      //!< 36 Signalisation disabled by parameter (Valid)
    dqValid = 37,           //!< 37 (?) Valid value
};

//тип данных файла samba-сервера
enum DSNSFileDataTypes {fdtString = 67, fdtFloat = 70, fdtInt = 73};

//заголовок каждого файла samba-сервера с данными (постоянно меняется)
struct DSNSFileHeader
{
    DSNSFileHeader() {reset();}
    DSNSFileHeader(qint8 t, qint8 len) {reset(); datatype=t; elemsize=len;}

    qint16 sec;
    qint16 min;
    qint16 hour;
    qint16 day;
    qint16 month;
    qint16 year;
    qint16 week_day;
    qint16 year_day;
    qint16 summer_time;
    qint32 file_size;
    qint16 checksum;
    qint16 write_mode;

    qint16 server_name_and_status;
    //char   datatype; //!< 'C', 'I', 'F'
    qint8 datatype;
    qint8 elemsize; //!< Size of data element in bytes
    qint16 reserve_2;

    int size() const;
    QString toStr() const;
    QString strDatatype() const {return QString("[%1]/code=%2").arg(QChar(datatype)).arg(datatype);}
    void reset();
    void toStream(QDataStream&);
    void fromStream(QDataStream&);

    static int headerSize();
};


//----------------------------------------------------------------------------

//описание 1-го эмулируемого параметра в файлах samba-сервера
struct MonitParam
{
    MonitParam() {reset();}
    MonitParam(QString x) {reset(); kks=x;}

    QString kks;
    int datatype;
    qint8 def_value;
    float def_value_f;
    QString desc;
    QString syn2;

    //---------------------------------------------------

    void reset() {kks="??"; datatype=-1; def_value=0; def_value_f=-1;}
    bool isInteger() const {return (datatype == fdtInt);}
    bool isFloat() const {return (datatype == fdtFloat);}
    QString strDatatype() const {return (isInteger() ? "int" : (isFloat() ? "float" : "?"));}
    QString strValue() const {return (isInteger() ? QString::number(def_value) : (isFloat() ? QString::number(def_value_f, 'f', 4) : "?"));}
    QString toStr() const {return QString("kks=%1/%2 datatype=%3 value=%4  (%5)").arg(kks).arg(syn2).arg(strDatatype()).arg(strValue()).arg(desc);}
    QString smbPrefixFile() const {return (isInteger() ? "APD" : "APA");}

};

//////////////// DSNSFileStruct ////////////////////////
class DSNSFileStruct : public QObject
{
    Q_OBJECT
public:
    DSNSFileStruct(QObject*);
    virtual ~DSNSFileStruct() {}

    void tryReadComplectFiles();
    void tryGenFiles();
    void nextEmulValues(); //сгенерить очередные файлы VAL (вызывается по таймеру)
    QString checkEmulDir() const;

    inline void setEmulDir(QString s) {m_emulDir = s.trimmed();}

    //длину строкового значения параметра value привести к размеру need_size,
    //т.е. либо отрезать справа лишнее, либо дополнить пробелами
    static void trimStrDataByType(QString &value, qint8 need_size);

protected:
    QTextCodec* m_codec;
    QList<MonitParam> m_params;
    QHash<QString, quint16> kks_positions;
    QString m_emulDir;

    void readFile(const QString&, QByteArray&);
    void parseFileData(QByteArray&);
    void readHeader(const QByteArray&, DSNSFileHeader&);
    void readData(const QByteArray&, const DSNSFileHeader&);
    void parseStringData(const QByteArray&, qint8);
    void parseFloatData(const QByteArray&, qint8);
    void parseIntegerData(const QByteArray&, qint8);
    void loadMonitParams(); //загрузить список параметров из файла smb_params.txt в контейнер m_params, которые будут эмулироваться

    bool hasMonitKKS(QString, QString&) const;
    void loadMonitParamsInfo();
    void writeFile(const QString&, const QByteArray&);
    void initEmulHeader(DSNSFileHeader&, QDateTime&);
    void writeFileStringData(QString f_type, const QStringList &data, DSNSFileHeader&);
    void writeFileIntData(QString f_type, const QList<qint8> &data, DSNSFileHeader&);

private:
    void genFloatFile_PID(); //kks
    void genFloatFile_SYN(); //ZSYN2
    void genFloatFile_DESC(); //ZDESC
    void genFloatFile_VAL(); //VAL
    void genFloatFile_QUAL(); //QUAL

    void genIntFile_PID(); //kks
    void genIntFile_SYN(); //ZSYN2
    void genIntFile_DESC(); //ZDESC
    void genIntFile_VAL(); //VAL
    void genIntFile_QUAL(); //QUAL

};


#endif


