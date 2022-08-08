#ifndef FOLDERSSTRUCT_DIALOG_H
#define FOLDERSSTRUCT_DIALOG_H

#include <QDialog>

#include "ui_foldersstructdialog.h"


class QStandardItem;

//FoldersStructDialog
class FoldersStructDialog : public QDialog, public Ui::FoldersStructDialog
{
    Q_OBJECT
public:
    FoldersStructDialog(const QString&, QWidget*);
    virtual ~FoldersStructDialog() {}

protected:
    QString cd_path;

    void initView();
    void fillModel(QStandardItem*, const QString&);

};




#endif //FOLDERSSTRUCT_DIALOG_H


