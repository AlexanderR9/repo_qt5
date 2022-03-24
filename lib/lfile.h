 #ifndef LFILE_H
 #define LFILE_H

 #include <QFile>
 #include <QStringList>

class LFile
{
public:
    static QString readFileBA(QString fname, QByteArray &ba); 
    static QString readFileStr(QString fname, QString &str); 
    static QString readFileSL(QString fname, QStringList &list, QString spliter = QString("\n")); 
    static QString writeFileSL(QString fname, QStringList &list); 
    static QString writeFile(QString fname, const QString &data);
    static QString appendFile(QString fname, const QString &data);

    //возвращает список файлов лежащих в dir_path(полные пути), кроме пустых строк или типа (. или ..)
    //если ftype не пустая строка, то вернет только список файлов с рассширением ftype
    static QString dirFiles(QString dir_path, QStringList &list, QString ftype = "txt");
    //проверка существования файла по имени (указывать полный путь)
    static bool fileExists(QString);
    //создать пустой файл (указывать полный путь)
    static QString fileCreate(QString);

};

 #endif



