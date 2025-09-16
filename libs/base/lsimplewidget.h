#ifndef LSIMPLE_WIDGET_H
#define LSIMPLE_WIDGET_H


#include <QWidget>
#include <QString>
#include <QGroupBox>
#include <QTableWidget>
#include <QMap>


class QSplitter;
class QSettings;
class QTabWidget;
class QListWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QJsonObject;
class QJsonValue;
class QJsonArray;
class QTreeWidgetItem;
class QTableWidgetItem;
class QListWidgetItem;
class QMouseEvent;

class LSearch;
class QLabel;
class QLineEdit;
class QAction;


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

//LSimpleWidget (with 2 splitters)
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
    virtual void resetPage() {} //некая очистка страницы или сброс в начальное состояние
    virtual int userSign() const {return m_userSign;} //некий вспомогательный признак, может не использоваться
    virtual QString userData() const {return m_userData;} //некие вспомогательные данные, может не использоваться

    void setSpacing(int); //задать отступы для внутренних виджетов (друг от друга)
    void addWidgetToSplitter(QWidget*, int orintation = Qt::Horizontal); //добавить виджет к сплитеру

protected:
    int             m_spliterType;
    QSplitter       *v_splitter;
    QSplitter       *h_splitter;
    int             m_userSign; //default: -1
    QString         m_userData; //default: empty

    void init();

signals:
    void signalError(const QString&);
    void signalMsg(const QString&);

private:
    bool invalidType() const;
    bool onlyVertical() const;
    bool onlyHorizontal() const;

public slots:
    virtual void slotEnableControls(bool) {}


};



//виджет-заготовка, представляет из себя групбокс, содержащий QTableWdiget.
//в конструкторе параметр type указывает тип layout, на котором размещается QTableWdiget (1-QVBoxLayout, 2-QHBoxLayout)
//при некорректном значении type используется  QVBoxLayout

//LTableWidgetBox (groupbox with table)
class LTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    LTableWidget(QWidget *parent) :QTableWidget(parent) {}
    virtual ~LTableWidget() {}
protected:
    void mouseDoubleClickEvent(QMouseEvent*) override; //переопределяем функцию клика мыши по области таблицы
signals:
    void signalDoubleClickEmptyArea(); //имитится при двойном клике по пустой области таблицы
};
class LTableWidgetBox : public QGroupBox
{
    Q_OBJECT
public:
    enum SortingDataType {sdtString = 555, sdtNumeric, sdtDate, sdtCustom};

    LTableWidgetBox(QWidget *parent = NULL, int type = 1);
    virtual ~LTableWidgetBox() {destroyPopupMenu();}

    void setHeaderLabels(const QStringList&, int orintation = Qt::Horizontal);
    void vHeaderHide();
    void resizeByContents();
    void setSelectionMode(int, int); //params: SelectionBehavior, SelectionMode
    void setSelectionColor(QString background_color, QString text_color = "#000000");
    void sortingOn(); //активировать возможность сортировки столбцов по клику на соответствующем заголовке
    void removeAllRows();
    void addSortingData(quint8, SortingDataType); //добавить данные для сортировки по указанному столбцу
    int curSelectedRow() const; //индекс текущей выделенной строки или -1, если выделено несколько строк, то вернет индекс 1-й из них

    //реализация всплывающего меню для таблицы по клику ПКМ  (поумолчанию эта функциональность отключена)
    virtual void popupMenuActivate(const QList< QPair<QString, QString> >&, bool need_row_selection = true); //активация всплывающего меню, необходимо передать набор пар <название, путь к иконке> , путь к иконке может быть пустым
    inline int popupMenuSize() const {return m_popupMenuActions.count();} //количество инициализированных пуктов меню
    void connectSlotToPopupAction(int, QObject*, const char*); //для соединения со слотами внешнего объекта-хозяина необходимо вызвать этот метод для каждого пункта меню, указав индекс пункта

    QTableWidget* table() const;

protected:
    QTableWidget    *m_table;
    QMap<quint8, SortingDataType> m_sortingData; //данные которые указывают какие столбцы должны реагировать на сортировку
    QList<QAction*> m_popupMenuActions; //пункты всплывающего меню, поумолчанию список пуст, активируются методом popumMenuActivate
    bool m_popupNeedSelection; // признак, нужно ли выделять строки при активации всплывающего меню (иначе меню работает для всей таблицы)

    void init();
    virtual void slotSortString(quint8 col, int order);
    virtual void slotSortNumeric(quint8 col, int order);
    virtual void slotSortDate(quint8 col, int order);
    virtual void slotSortCustom(quint8, int) {} //какая-то необычная сортировка, нужно переопределить в классе своего проекта

    virtual void destroyPopupMenu(); //уничтожает реализацию вплывающего меню (если она была активирована)
    virtual void connectPopupActionSlots() {/*qDebug("invoke connectPopupActionSlots()");*/} //эту функцию необходимо переопределить если активировано всплывающее меню, нужно подключить свои слоты к элементам m_popupMenuActions

protected slots:
    virtual void slotItemDoubleClicked(QTableWidgetItem*); //в базовом класе копируется содержимое итема в буфер обмена
    virtual void slotSortByColumn(int); //сортировка столбца
    virtual void slotContextMenu(QPoint); //вызывается при вызове всплывающего меню(при условии что оно активировано для этого экземпляра)
    virtual void slotDoubleClickEmptyArea() {} //вызывается при двойном клике по пустой области таблицы

};
class LSearchTableWidgetBox : public LTableWidgetBox
{
    Q_OBJECT
public:
    LSearchTableWidgetBox(QWidget *parent = NULL);
    virtual ~LSearchTableWidgetBox() {}

    virtual void searchExec();
    virtual void searchReset();
    virtual void setTextLabel(const QString&);
    virtual void searchEditHide();
    virtual void setSearchEdit(const QLineEdit*);

protected:
    LSearch     *m_searchObj;
    QLineEdit   *m_searchEdit;
    QLabel      *m_searchLabel;

signals:
    void signalSearched();  //выполняется каждый раз после произведенного фильтра

};



//виджет-заготовка, представляет из себя групбокс, содержащий QListWdiget.
//в конструкторе параметр type указывает тип layout, на котором размещается QListWdiget (1-QVBoxLayout, 2-QHBoxLayout)
//при некорректном значении type используется  QVBoxLayout

//LListWidgetBox (groupbox with listwidget)
class LListWidgetBox : public QGroupBox
{
    Q_OBJECT
public:
    LListWidgetBox(QWidget *parent = NULL, int type = 1);
    virtual ~LListWidgetBox() {}

    QListWidget* listWidget() const;
    void setRowColor(quint16, QString);
    void setRowTextColor(quint16, QString);
    void setSelectionColor(QString background_color, QString text_color = "#000000");
    void setBaseColor(QString background_color, QString text_color = "#000000");
    void setFontSizeItems(int);
    void addItem(QString text, QString icon_path = QString());
    void removeItemByValue(QString text);
    void setSelectionMode(int, int); //params: SelectionBehavior, SelectionMode
    void clearSelection();
    QStringList selectedValues() const;
    QList<int> selectedRows() const;
    bool valueContain(const QString&) const;

protected:
    QListWidget    *m_listWidget;

    void init();

protected slots:
    virtual void slotItemDoubleClicked(QListWidgetItem*); //в базовом класе копируется содержимое итема в буфер обмена

};
class LSearchListWidgetBox : public LListWidgetBox
{
    Q_OBJECT
public:
    LSearchListWidgetBox(QWidget *parent = NULL);
    virtual ~LSearchListWidgetBox() {}

    virtual void searchExec();
    virtual void searchReset();
    virtual void setTextLabel(const QString&);
    virtual void searchEditHide();
    virtual void setSearchEdit(const QLineEdit*);

protected:
    LSearch     *m_searchObj;
    QLineEdit   *m_searchEdit;
    QLabel      *m_searchLabel;

signals:
    void signalSearched();  //выполняется каждый раз после произведенного фильтра

};



//виджет-заготовка, представляет из себя групбокс, содержащий QTreeWidget
//в конструкторе параметр type указывает тип layout, на котором размещается QListWdiget (1-QVBoxLayout, 2-QHBoxLayout)
//при некорректном значении type используется  QVBoxLayout

//LTreeWidgetBox (groupbox with treewidget)
class LTreeWidgetBox : public QGroupBox
{
    Q_OBJECT
public:
    LTreeWidgetBox(QWidget *parent = NULL, int type = 1);
    virtual ~LTreeWidgetBox() {}

    QTreeWidget* view() const {return m_view;}
    void setHeaderLabels(const QStringList&);
    QTreeWidgetItem* rootItem() const; //возвращает рутовый итем или null
    void clearView(); //удаляются все итемы, заголовок остается
    void clearRoot(); //удаляются все итемы, кроме рутового
    void addRootItem(const QStringList&); //добавить рутовый элемент, (при условии что на текущий момент его нет)
    void setRootItemAttrs(QColor, int col = -1, bool bold = false, bool italic = false, int size = -1);
    void loadJSON(const QJsonObject&, QString root_title = QString()); //загрузка QJsonObject во вьюху, предварительно вьюха будет полностью очищена
    void resizeByContents();
    void expandAll();
    void expandLevel(); //expand by m_expandLevel
    void setSelectionMode(int, int); //params: SelectionBehavior, SelectionMode

    inline void setExpandLevel(int a) {m_expandLevel = a;}

    //установить свойства текста итема.
    //если col=-1 то для всех кололнок
    //если size=-1 то размер шрифта не менять
    static void setAttrsItem(QTreeWidgetItem*, QColor, int col = -1, bool bold = false, bool italic = false, int size = -1);

protected:
    QTreeWidget    *m_view;
    int m_expandLevel;

    void init();
    void loadJSONValue(const QString&, const QJsonValue&, QTreeWidgetItem*); //загрузка элемента QJsonObject во вьюху, (функция для реализации рекурсии)
    void loadJSONValueArray(const QJsonArray&, QTreeWidgetItem*); //загрузка элемента QJsonObject который является QJsonArray во вьюху, (функция для реализации рекурсии)
    void loadJSONValueObj(const QJsonObject&, QTreeWidgetItem*); //загрузка элемента QJsonObject который является объектом во вьюху, (функция для реализации рекурсии)

private:
    void getJSONValueType(QStringList&, const QJsonValue&); //добавить в контейнер значение и тип QJsonValue

protected slots:
    virtual void slotItemDoubleClicked(QTreeWidgetItem*, int); //в базовом класе копируется содержимое итема в буфер обмена

};


//LTabWidgetBox (groupbox with tabwidget)
class LTabWidgetBox : public QGroupBox
{
    Q_OBJECT
public:
    LTabWidgetBox(QWidget *parent = NULL, int type = 1);
    virtual ~LTabWidgetBox() {}

    QTabWidget* tab() const {return m_tab;}
    void removeAllPages(); //уничтожает объекты-страницы из памяти и очищает сам m_tab
    void setBorderNone(); // удаляет рамку QGroupBox и отступы margin, растягивая m_tab на всю площадь
    int pageCount() const;
    inline bool hasPages() const {return (pageCount() > 0);}

protected:
    QTabWidget    *m_tab;
    void init();
};


#endif // LSIMPLE_WIDGET_H


