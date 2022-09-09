#ifndef LSIMPLE_WIDGET_H
#define LSIMPLE_WIDGET_H


#include <QWidget>
#include <QString>
#include <QGroupBox>


class QSplitter;
class QSettings;
class QTableWidget;
class QListWidget;



//простой виджет-заготовка с двумя сплитерами.
//функции load и save сами не вызываются, необходимо вызвать из класса выше.
//функции load и save завязаны на objectName() объекта этого класса,
//поэтому если в проекте таких объектов несколько тогда каждом надо задать уникальное имя методом setObjectName()

//в конструкторе параметр type указывает наличие сплитеров и их вложенность,
//параметр должен быть двузначный, например 31,
//первая цифра указывает на то, сколько (и какие) сплитеров создать на виджете (1-только вертикальный, 2-только горизонтальный, 3-оба),
//вторая цифра указывает на то, какой сплитер основной, т.е первым ложится на виджет, а затем к нему цепляется второй (1-вертикальный, 2-горизонтальный),
//если первая цифра указывает на то, что будет создан только один сплитер, то вторая цифра не важна.
//в случае неверного указания отрабатывает вариант поумолчанию: создаются оба сплитера, основной - вертикальный.

//LSimpleWidget
class  LSimpleWidget : public QWidget
{
    Q_OBJECT
public:
    LSimpleWidget(QWidget *parent = NULL, int type = -1);
    virtual ~LSimpleWidget() {}

    //сохранение/восстановление сплитеров
    virtual void load(QSettings&);
    virtual void save(QSettings&);

    virtual QString caption() const {return QString("Simple page!!!");} //некая надпись соответствующая этому виджету
    virtual QString iconPath() const {return QString();} //некая иконка соответствующая этому виджету


protected:
    int             m_spliterType;
    QSplitter       *v_splitter;
    QSplitter       *h_splitter;

    void init();

signals:
    void signalError(const QString&);
    void signalMsg(const QString&);

private:
    bool invalidType() const;
    bool onlyVertical() const;
    bool onlyHorizontal() const;

};



//виджет-заготовка, представляет из себя групбокс, содержащий QTableWdiget.
//в конструкторе параметр type указывает тип layout, на котором размещается QTableWdiget (1-QVBoxLayout, 2-QHBoxLayout)
//при некорректном значении type используется  QVBoxLayout

//LTableWidgetBox
class LTableWidgetBox : public QGroupBox
{
    Q_OBJECT
public:
    LTableWidgetBox(QWidget *parent = NULL, int type = 1);
    virtual ~LTableWidgetBox() {}

    void setHeaderLabels(const QStringList&);
    void vHeaderHide();


protected:
    QTableWidget    *m_table;

    void init();
};




//виджет-заготовка, представляет из себя групбокс, содержащий QListWdiget.
//в конструкторе параметр type указывает тип layout, на котором размещается QListWdiget (1-QVBoxLayout, 2-QHBoxLayout)
//при некорректном значении type используется  QVBoxLayout

//LListWidgetBox
class LListWidgetBox : public QGroupBox
{
    Q_OBJECT
public:
    LListWidgetBox(QWidget *parent = NULL, int type = 1);
    virtual ~LListWidgetBox() {}

protected:
    QListWidget    *m_listWidget;

    void init();
};




#endif // LSIMPLE_WIDGET_H


