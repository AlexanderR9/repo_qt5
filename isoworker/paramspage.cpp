#include "paramspage.h"
#include "lstatic.h"
#include "ltable.h"
#include "lfile.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>




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
    list << "N" << "Name" << "Size, Kb";
    return list;
}
void ParamsPage::initTable()
{
    LTable::fullClearTable(isoTable);
    LTable::setTableHeaders(isoTable, headerLabels());
    LTable::resizeTableContents(isoTable);
}
void ParamsPage::loadISOList(const QString &path)
{
    qDebug() << QString("ParamsPage::loadISOList path: %1").arg(path);
    LTable::removeAllRowsTable(isoTable);

    qDebug("1");
    QStringList list;
    QString err = LFile::dirFiles(path, list, "iso");
    if (!err.isEmpty())
    {
        emit signalError(err);
        return;
    }

    qDebug("2");
    for (int i=0; i<list.count(); i++)
    {
        QStringList row_data;
        row_data.append(QString::number(i+1));
        row_data.append(LFile::shortFileName(list.at(i)));
        row_data.append(QString::number(LFile::fileSizeKB(list.at(i))));
        LTable::addTableRow(isoTable, row_data);
    }

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








