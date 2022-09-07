#ifndef FXDATA_LOADER_WIDGET_H
#define FXDATA_LOADER_WIDGET_H

#include <QGroupBox>
#include <QList>

class QTableWidget;


//FXCoupleDataParams
struct FXCoupleDataParams
{
    FXCoupleDataParams() :couple(QString()), timeframe(-1), bar_count(0) {}
    FXCoupleDataParams(const QString &s, int t) :couple(s), timeframe(t), bar_count(0) {}

    QString couple;
    int timeframe;
    quint16 bar_count;

    QString toStr() const {return QString("FXCoupleDataParams: (%1),  tf=%2,  size=%3").arg(couple).arg(timeframe).arg(bar_count);}
};


//представляет из себя QGroupBox содержащий таблицу.
//служит для отображения списка загруженных инструментов.
//FXDataLoaderWidget
class FXDataLoaderWidget : public QGroupBox
{
    Q_OBJECT
public:
    FXDataLoaderWidget(QWidget*);
    virtual ~FXDataLoaderWidget() {}

    void reloadData(const QList<FXCoupleDataParams>&); //перезаписать в таблицу информацию о загруженных данных
    int count() const; //количество успешно загруженных файлов (строк в таблице)
    void getSelection(QList<FXCoupleDataParams>&); //получить текущие выделенные инструменты

    inline bool emptyData() const {return (count() == 0);}

protected:
    QTableWidget *m_table;

    void initTable(); //инициализация таблицы
    void clearTable(); //удалить все строки из таблицы
    void updateTitle(); //обновить caption у QGroupBox

signals:
    void signalSelectionChanged(); //эмитится когда пользователь выделяет очередную строку или снимает выделение

};



#endif //FXDATA_LOADER_WIDGET_H

