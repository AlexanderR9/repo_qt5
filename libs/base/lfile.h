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
    static QString writeFileSL(QString fname, const QStringList &list);
    static QString writeFile(QString fname, const QString &data);
    static QString writeFileBA(QString fname, const QByteArray &ba);
    static QString appendFile(QString fname, const QString &data);
    static QString appendFileSL(QString fname, const QStringList &list);

    //возвращает список файлов лежащих в dir_path(полные пути), кроме пустых строк или типа (. или ..)
    //если ftype не пустая строка, то вернет только список файлов с рассширением ftype,
    //рассширение type указывается без точки, пример: 'xml' или 'csv'
    static QString dirFiles(QString dir_path, QStringList &list, QString ftype = QString());

    //возвращает список папок лежащих в dir_path(полные пути), кроме пустых строк или типа (. или ..)
    //если filter_text не пустая строка, то вернет только список папок, имя которых содержит filter_text
    static QString dirFolders(QString dir_path, QStringList &list, QString filter_text = QString());


    //проверка существования файла по имени (указывать полный путь)
    static bool fileExists(QString);
    //проверка существования папки по имени (указывать полный путь)
    static bool dirExists(QString);
    //создать пустой файл (указывать полный путь)
    static QString fileCreate(QString);
    //возращает имя файла без полного пути
    static QString shortFileName(QString);
    //возращает имя папки без полного пути
    static QString shortDirName(QString);
    //возращает родительский путь папки в которой лежит указанная папка или файл (без / в конце)
    static QString rootPath(QString);
    //возращает рассширение файла без точки, вернет пустую строку если рассширения нет
    static QString fileExtension(QString);
    //возращает путь к файлу или пустую строку если файл указан некорректно
    static QString filePath(QString);

    //возвращает путь запуска своего приложения
    static QString appPath();



    //возвращает размер файла, в случае ошибки вернет -1
    static int fileSizeB(QString);
    static int fileSizeKB(QString);
    static int fileSizeMB(QString);

};

 #endif



