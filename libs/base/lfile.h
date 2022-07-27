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

    //возвращает список папок лежащих в dir_path(полные пути), кроме пустых строк или типа (. или ..)
    //если filter_text не пустая строка, то вернет только список папок, имя которых содержит filter_text
    static QString dirFolders(QString dir_path, QStringList &list, QString filter_text = QString());


    //проверка существования файла по имени (указывать полный путь)
    static bool fileExists(QString);
    //создать пустой файл (указывать полный путь)
    static QString fileCreate(QString);
    //возращает имя файла без полного пути
    static QString shortFileName(QString);
    //возращает имя папки без полного пути
    static QString shortDirName(QString);


    //возвращает размер файла, в случае ошибки вернет -1
    static int fileSizeB(QString);
    static int fileSizeKB(QString);
    static int fileSizeMB(QString);

};

 #endif



