#ifndef FOLDERSSTRUCT_DIALOG_H
#define FOLDERSSTRUCT_DIALOG_H

#include <QDialog>

#include "ui_foldersstructdialog.h"


//FoldersStructDialog
class FoldersStructDialog : public QDialog, public Ui::FoldersStructDialog
{
    Q_OBJECT
public:
    FoldersStructDialog(QWidget*);
    virtual ~FoldersStructDialog() {}


};




#endif //FOLDERSSTRUCT_DIALOG_H


