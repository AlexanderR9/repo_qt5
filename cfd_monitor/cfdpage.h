#ifndef CFDPAGE_H
#define CFDPAGE_H

#include "basepage.h"

class QTableWidget;
class LSearch;
class QLineEdit;

//CFDPage
class CFDPage : public BasePage
{
    Q_OBJECT
public:
    CFDPage(QWidget*);
    virtual ~CFDPage() {}

    QString iconPath() const {return QString(":/icons/images/b_scale.svg");}
    QString caption() const {return QString("CFD stat");}

    void updatePage();

protected:
    QTableWidget    *m_table;
    QLineEdit       *m_searchEdit;
    LSearch         *m_search;

    void initSearch();
    QStringList headerLabels() const;
    void init();
    void removeRowByTicker(const QString&); //удалить строку из таблицы с заданным тикером, если такой строки нет, то ничего не произойдет
    void updateCellColors();
    void decreaseSortNum(int); //сортировка числового столбца на уменьшение
    void increaseSortNum(int); //сортировка числового столбца на возрастание

public slots:
    void slotNewPrice(const QStringList&); //пришла цена для одного тикера, QStringList - это значения строки таблицы
    void slotSetCurrentPrices(QMap<QString, double>&); //записать в контейнер все пары тикер-цена, присутствующие в таблице на данный момент

protected slots:
    void slotSortByColumn(int);

private:
    void getCellValue(const QString&, double&, bool&);
    QColor getColorByLimits(const double&, double) const;

signals:
    void signalGetInstaPtr(const QString&, bool&);


};


#endif //CFDPAGE_H


