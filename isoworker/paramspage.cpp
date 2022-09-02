#include "paramspage.h"
#include "lstatic.h"
#include "ltable.h"
#include "lfile.h"
#include "ltime.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>


#define MD5_COL             3
#define ISO_NAME_COL        1
#define TIME_COL            1



//ParamsPage
ParamsPage::ParamsPage(QWidget *parent)
    :QWidget(parent)
{
    setupUi(this);
    setObjectName("params_page");

    initTable();
}
bool ParamsPage::isSudo() const
{
    //return isSudoCheckBox->isChecked();
    return false;
}
void ParamsPage::save(QSettings &settings)
{
    Q_UNUSED(settings);
    //settings.setValue("paramspage/command", cmdLineEdit->text());
    //settings.setValue("paramspage/args", argsLineEdit->text());
    //settings.setValue("paramspage/sudo", isSudoCheckBox->isChecked());
}
void ParamsPage::load(QSettings &settings)
{
    Q_UNUSED(settings);
    //cmdLineEdit->setText(settings.value("paramspage/command").toString());
    //argsLineEdit->setText(settings.value("paramspage/args").toString());
    //isSudoCheckBox->setChecked(settings.value("paramspage/sudo").toBool());
}
QStringList ParamsPage::headerLabels() const
{
    QStringList list;
    list << "N" << "Name" << "Size, Kb" << "MD5 sum";
    return list;
}
QStringList ParamsPage::logHeaderLabels() const
{
    QStringList list;
    list << "N" << "Time" << "Command" << "File" << "Result";
    return list;
}
void ParamsPage::initTable()
{
    LTable::fullClearTable(isoTable);
    LTable::setTableHeaders(isoTable, headerLabels());
    LTable::resizeTableContents(isoTable);

    LTable::fullClearTable(logTable);
    LTable::setTableHeaders(logTable, logHeaderLabels());
    LTable::resizeTableContents(logTable);
    logTable->verticalHeader()->hide();
    LTable::resizeTableContents(logTable);


}
void ParamsPage::updateMD5Column(const QString &path)
{
    int n = isoTable->rowCount();
    if (n <= 0) return;

    QString fname = QString("%1%2%3").arg(path).arg(QDir::separator()).arg(ParamsPage::md5File());
    QStringList list;
    QString err = LFile::readFileSL(fname, list);
    if (!err.isEmpty())
    {
        emit signalError(err);
        return;
    }
    if (list.isEmpty())
    {
        emit signalError(QString("Data list of (%1) is empty").arg(ParamsPage::md5File()));
        return;
    }

    for (int i=0; i<n; i++)
    {
        QString iso_file = isoTable->item(i, ISO_NAME_COL)->text().trimmed();
        for (int j=list.count()-1; j>=0; j--)
        {
            QString s = list.at(j).trimmed();
            if (s.contains(iso_file))
            {
                s = LStatic::strTrimLeft(s, iso_file.length()).trimmed();
                isoTable->item(i, MD5_COL)->setText(s);
                break;
            }
        }
    }
}
void ParamsPage::reloadISOList(const QString &path)
{
    qDebug() << QString("ParamsPage::reloadISOList path: %1").arg(path);
    LTable::removeAllRowsTable(isoTable);
    isoBox->setTitle(QString("ISO list   [DIR: %1]").arg(path));

    QStringList list;
    QString err = LFile::dirFiles(path, list, "iso");
    if (!err.isEmpty())
    {
        emit signalError(err);
        return;
    }

    for (int i=0; i<list.count(); i++)
    {
        QStringList row_data;
        row_data.append(QString::number(i+1));
        row_data.append(LFile::shortFileName(list.at(i)));
        row_data.append(QString::number(LFile::fileSizeKB(list.at(i))));
        row_data.append("---");
        LTable::addTableRow(isoTable, row_data);
    }

    updateMD5Column(path);
    LTable::resizeTableContents(isoTable);
}
QString ParamsPage::seletedISOFile() const
{
    QList<int> rows = LTable::selectedRows(isoTable);
    if (rows.count() != 1)
    {
        qWarning()<<QString("ParamsPage::seletedISOFile() WARNING: invalid selected rows count (%1)").arg(rows.count());
        return QString();
    }

    return isoTable->item(rows.first(), 1)->text();
}
void ParamsPage::resetColors()
{
    int n = isoTable->rowCount();
    if (n <= 0) return;

    for (int i=0; i<n; i++)
        isoTable->item(i, MD5_COL)->setTextColor(Qt::black);
}
void ParamsPage::findMD5(const QString &md5_value)
{
    int n = isoTable->rowCount();
    if (n <= 0) return;

    for (int i=0; i<n; i++)
    {
        if (isoTable->item(i, MD5_COL)->text() == md5_value)
        {
            isoTable->item(i, MD5_COL)->setTextColor(Qt::blue);
            break;
        }
    }
}
void ParamsPage::startCommand(QString cmd, QString fname)
{
    QStringList row_data;
    row_data.append(QString::number(logTable->rowCount()+1));
    row_data.append(QString("%1 - ?").arg(LTime::strCurrentTime()));
    row_data.append(cmd);
    row_data.append(fname);
    row_data.append(QString("?"));
    LTable::addTableRow(logTable, row_data);
    LTable::resizeTableContents(logTable);

    int n = logTable->rowCount();
    if (n > 0) logTable->item(n-1, logHeaderLabels().count()-1)->setTextColor(Qt::gray);
}
void ParamsPage::finishedCommand(QString result)
{
    int n = logTable->rowCount();
    if (n <= 0) return;

    QTableWidgetItem *r_item = logTable->item(n-1, logHeaderLabels().count()-1);
    if (!r_item) return;

    r_item->setText(result);
    if (result.contains("ok")) r_item->setTextColor(QColor(50, 200, 0));
    else if (result.contains("fault")) r_item->setTextColor(Qt::red);
    else if (result.contains("break")) r_item->setTextColor(Qt::darkYellow);
    else r_item->setTextColor(Qt::gray);

    QString s_time = logTable->item(n-1, TIME_COL)->text();
    s_time.replace("?", LTime::strCurrentTime());
    logTable->item(n-1, TIME_COL)->setText(s_time);

    LTable::resizeTableContents(logTable);
}








