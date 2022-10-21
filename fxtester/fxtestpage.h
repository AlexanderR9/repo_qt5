#ifndef FX_TESTPAGE_H
#define FX_TESTPAGE_H

#include "lsimplewidget.h"


class QComboBox;
class FXDataLoader;
class FXBarContainer;
class FXTesterObj;


// FXInputParamsWidget
class FXInputParamsWidget : public LSimpleWidget
{
    Q_OBJECT
public:
    FXInputParamsWidget(QWidget *parent = 0);
    virtual ~FXInputParamsWidget() {}

    virtual QString caption() const {return QString("input_widget");} //некая надпись соответствующая этому виджету
    void setYearsRange(int, int); //установить полный временной диапазон загруженных данных
    int currentTest() const;

    void load(QSettings&);
    void save(QSettings&);

protected:
    LTableWidgetBox     *m_tableBox;
    QComboBox           *m_testTypeBox;
    QComboBox           *m_yearStartBox;
    QComboBox           *m_yearEndBox;

    void initWidgets();
    void fillTestsBox();

signals:
    void signalTestChanged();

};


//страница-интерфес для проведения тестирования

// FXTestPage
class FXTestPage : public LSimpleWidget
{
    Q_OBJECT
public:
    FXTestPage(QWidget *parent = 0);
    virtual ~FXTestPage() {}

    virtual QString caption() const {return QString("Testing page");} //некая надпись соответствующая этому виджету
    virtual QString iconPath() const {return QString(":/icons/images/r_scale.svg");} //некая иконка соответствующая этому виджету

    void updateLoadedData(const FXDataLoader*); //обновились загружженные данные, необходимо обновить виджет FXInputParamsWidget
    inline int currentTest() const {return m_inputWidget->currentTest();}

    void load(QSettings&);
    void save(QSettings&);

protected:
    FXInputParamsWidget     *m_inputWidget;
    LTableWidgetBox         *m_resultWidget;
    LTableWidgetBox         *m_historyWidget;
    FXTesterObj             *m_tester;

    void initWidgets();
    void initResultsTable();
    void addDataContainer(const FXBarContainer*);
    void reinitTests(const FXDataLoader*);

protected slots:
    void slotTestChanged();

};



#endif //FX_TESTPAGE_H


