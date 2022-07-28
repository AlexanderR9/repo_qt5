#include "paramspage.h"
#include "lstatic.h"
#include "ltable.h"
#include "lfile.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>


#define MD5_COL             3
#define ISO_NAME_COL        1



//ParamsPage
ParamsPage::ParamsPage(QWidget *parent)
    :QWidget(parent)
{
    setupUi(this);
    setObjectName("params_page");

    initTable();
}
QString ParamsPage::commandName() const
{
    return cmdLineEdit->text().trimmed();
}
QStringList ParamsPage::getArgs() const
{
    QString s = argsLineEdit->text().trimmed();
    if (s.isEmpty()) return QStringList();
    return LStatic::trimSplitList(s, ";");
}
bool ParamsPage::isSudo() const
{
    return isSudoCheckBox->isChecked();
}
void ParamsPage::save(QSettings &settings)
{
    settings.setValue("paramspage/command", cmdLineEdit->text());
    settings.setValue("paramspage/args", argsLineEdit->text());
    settings.setValue("paramspage/sudo", isSudoCheckBox->isChecked());
}
void ParamsPage::load(QSettings &settings)
{
    cmdLineEdit->setText(settings.value("paramspage/command").toString());
    argsLineEdit->setText(settings.value("paramspage/args").toString());
    isSudoCheckBox->setChecked(settings.value("paramspage/sudo").toBool());
}
QStringList ParamsPage::headerLabels() const
{
    QStringList list;
    list << "N" << "Name" << "Size, Kb" << "MD5 sum";
    return list;
}
void ParamsPage::initTable()
{
    LTable::fullClearTable(isoTable);
    LTable::setTableHeaders(isoTable, headerLabels());
    LTable::resizeTableContents(isoTable);
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








