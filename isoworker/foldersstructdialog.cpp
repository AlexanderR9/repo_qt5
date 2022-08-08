#include "foldersstructdialog.h"
#include "lfile.h"

#include <QDebug>
#include <QFileSystemModel>
#include <QStandardItemModel>

//FoldersStructDialog
FoldersStructDialog::FoldersStructDialog(const QString &path, QWidget *parent)
    :QDialog(parent),
      cd_path(path.trimmed())
{
    setupUi(this);
    setObjectName("folders_struct_dialog");
    if (cd_path.isEmpty()) return;

    initView();
    view->expandAll();
    view->resizeColumnToContents(0);

    resize(800, 600);
}
void FoldersStructDialog::initView()
{
    QStandardItemModel *model = new QStandardItemModel(NULL);
    model->setColumnCount(3);
    model->setHeaderData(0, Qt::Horizontal, "Name", Qt::DisplayRole);
    model->setHeaderData(1, Qt::Horizontal, "Type", Qt::DisplayRole);
    model->setHeaderData(2, Qt::Horizontal, "Size", Qt::DisplayRole);

    QStandardItem *root_item = new QStandardItem(cd_path);
    model->appendRow(root_item);

    QDir dir(cd_path);
    if (!dir.exists())
    {
        QStandardItem *err_item = new QStandardItem("invalid path");
        err_item->setForeground(QBrush(Qt::red));
        model->setItem(0, 1, err_item);
    }
    else
    {
        model->setItem(0, 1, new QStandardItem("CD"));
        model->item(0, 0)->setForeground(Qt::blue);
        model->item(0, 1)->setForeground(Qt::blue);
        fillModel(root_item, cd_path);
    }

    view->setModel(model);
}
void FoldersStructDialog::fillModel(QStandardItem *root_item, const QString &cur_path)
{
    if (!root_item) return;
    QList<QStandardItem*> child_folders_items;


    //find dirs
    QStringList dir_list;
    QString err = LFile::dirFolders(cur_path, dir_list);
    if (!err.isEmpty())
    {
        qWarning()<<QString("FoldersStructDialog::fillModel: WARNING - %1").arg(err);
        return;
    }
    for (int i=0; i<dir_list.count(); i++)
    {
        QList<QStandardItem*> i_list;
        i_list << new QStandardItem(LFile::shortDirName(dir_list.at(i)));
        i_list << new QStandardItem("folder");
        child_folders_items.append(i_list.first());
        root_item->appendRow(i_list);
    }


    //find files
    QStringList f_list;
    err = LFile::dirFiles(cur_path, f_list);
    if (!err.isEmpty())
    {
        qWarning()<<QString("FoldersStructDialog::fillModel: WARNING - %1").arg(err);
        return;
    }
    qDebug()<<QString("find files %1, parent_path: %2").arg(f_list.count()).arg(cur_path);
    QFont f_font;
    f_font.setItalic(true);
    for (int i=0; i<f_list.count(); i++)
    {
        QList<QStandardItem*> i_list;
        i_list << new QStandardItem(LFile::shortFileName(f_list.at(i)));
        i_list << new QStandardItem("file");
        i_list << new QStandardItem(QString::number(LFile::fileSizeB(f_list.at(i))));

        foreach (QStandardItem *it, i_list)
        {
            it->setForeground(Qt::gray);
            it->setFont(f_font);
        }
        root_item->appendRow(i_list);
    }


    //start recursive
    if (child_folders_items.isEmpty()) return;
    foreach (QStandardItem *it, child_folders_items)
        fillModel(it, QString("%1%2%3").arg(cur_path).arg(QDir::separator()).arg(it->text()));

}










