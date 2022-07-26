#include "paramspage.h"
#include "lstatic.h"

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








