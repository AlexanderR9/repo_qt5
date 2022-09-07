#ifndef LSIMPLE_WIDGET_H
#define LSIMPLE_WIDGET_H


#include <QWidget>
#include <QString>

class QSplitter;
class QSettings;


//простой виджет-заготовка с двумя сплитерами.
//функции load и save сами не вызываются, необходимо вызвать из класса выше.
//функции load и save завязаны на objectName() объекта этого класса,
//поэтому если в проекте таких объектов несколько тогда каждом надо задать уникальное имя методом setObjectName()

//type  - параметр который указывает наличие сплитеров и их вложенность,
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


#endif // LSIMPLE_WIDGET_H


