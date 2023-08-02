#ifndef LXMLPACK_VIEW_H
#define LXMLPACK_VIEW_H


#include <QGroupBox>
#include <QTreeWidgetItem>


class LXMLPackObj;
class QTreeWidget;
class LXMLPackElement;
class QDomDocument;

// LXMLPackViewItem
class LXMLPackViewItem : public QTreeWidgetItem
{
public:
    LXMLPackViewItem(LXMLPackElement*, QTreeWidgetItem *parent = NULL, bool kks_used = false);
    virtual ~LXMLPackViewItem() {}

    void changePackValue(int); //значение или отклонение изменилось пользователем, необходимо обновить его в пакете (т.е. item_text => m_node)
    void setReadOnly(bool); //установить возможность редактирования значения или отклонения в пакете
    void updateValues(); //обновить значения свое и всех своих детей рекурсивно
    void setDoublePrecision(quint8);
    QString userData() const;
    void resetEditingMode(); //сброс режима редактирования для всех итемов

    inline bool isEditable() const {return m_editable;}
    inline void setEditing(bool b) {is_editing = b;}
    inline bool isEditing() const {return is_editing;}



protected:
    LXMLPackElement *m_node;
    bool m_editable;
    bool is_editing; //признак того что редактируется в данный момент

    void loadNodeChilds();
    void updateColumnsText();
    void updateColumnsColor();

    //обновить значение и отколоние этого итема, которые берутся из m_node.
    //итем обновляется только если он является типом isData или isTime.
    void updateValue();

};

//LXMLPackTreeWidget
class LXMLPackTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    LXMLPackTreeWidget(QWidget *parent) :QTreeWidget(parent) {setObjectName("lxmlpack_treewidget");}


protected slots:
    //выполняется в момент завершения редактирования пользователем итема
    virtual void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

signals:
    void signalCloseEditor(); //эмитится в момент завершения редактирования пользователем итема


};


//класс для отображения структуры одного экземпляра пакета LXMLPackObj.
//позволяет просматривать текущие значения полей пакета.
//m_readOnly задает режим вкл/выкл редактирования значений пакета
class LXMLPackView : public QGroupBox
{
    Q_OBJECT
public:
    LXMLPackView(const QString&, QWidget *parent = NULL); //params: title and parent
    virtual ~LXMLPackView() {resetView();}



    //подразумевается что будет выполнена 1 из 2-х функций (setPacket/initPacket)
    virtual void setPacket(LXMLPackObj*); //установить экземпляр пакета для отображения
    virtual void initPacket(const QDomDocument&); //инициализировать m_packet


    virtual void setPacketByteOrder(int); //устанавливает порядок байт для записи пакета в поток данных в m_packet
    virtual void resizeColumns(); //подогнать размеры столбцов под контент

    //обновить значения итемов (ВСЕХ), т.е. вытащить их из m_packet заполнить итемы дерева соответствующими значениями нод
    virtual void updateValues();

    //обновить значения всех элементов пакета m_packet с учетом rand_deviation, (если rand_deviation == 0, то значение не измениться)
    //если update_items == true то сразу же  и выполнится updateValues()
    virtual void nextRandValues(bool update_items = true);

    virtual void setSelectionRowsMode();
    virtual bool kksUsed() const;
    virtual int packetSize() const; //вернет размер загруженного пакета в байтах или -1
    virtual void setExpandLevel(int); //раскрывает элементы дерева до заданной глубины


    //установить значение 1-й ноды в пакете по заданному пути.
    //путь указывается в виде набора уровней вложенности, начинается всегда с 'packet'
    //пример: packet/2/0/11
    //если по заданному пути нет ноды или ее тип не соответствует устанавливаему значению то в параметр bool запишется false
    void setIntValueByPath(QString, qint64, bool&);
    void setDoubleValueByPath(QString, double, bool&);

    //возвращает значение ноды соответствующего типа
    qint64 getIntValueByPath(QString, bool&);
    double getDoubleValueByPath(QString, bool&);


    // DataStream operation (in/out )
    void setPacketData(const QByteArray&, bool&, bool singleFloatPrecision = false); //записать массив байт в пакет
    void fromPacket(QByteArray&, bool singleFloatPrecision = false); //запись пакета в массив байт


    inline const LXMLPackObj* getPacket() const {return m_packet;}
    inline void setReadOnly(bool b) {m_readOnly = b;}
    inline bool isReadOnly() const {return m_readOnly;}
    inline void  setDoublePrecision(quint8 p) {m_doublePrecision = p;}
    inline bool invalid() const {return (m_rootItem == NULL);}


protected:
    LXMLPackTreeWidget  *m_view;
    LXMLPackViewItem    *m_rootItem;
    LXMLPackObj         *m_packet;
    bool                 m_readOnly;
    quint8               m_doublePrecision;

    virtual void initWidget();
    virtual void reloadView();
    virtual void resetView();
    virtual void resetEditingMode(); //сброс режима редактирования для всех итемов
    virtual void prepareView(); //выполняется при инициализации пакета, подключение сигналов m_view и уставнока режима чтения/записи

protected slots:

    //выполняется когда пользователь двойным щелчком по итему ввел его в режим редактирования,
    //и в этот момент у этого итема взводится флаг is_editing.
    virtual void slotItemActivate(QTreeWidgetItem*, int);

    //выполняется когда значение(текст) итема измелось.
    //не важно каким способом, пользователем вручную или программно автоматом,
    //но проверяется признак режима редактирования пользователе в текущий момент и по факту
    //если редактировал пользователь, то произойдет установка значения в соответствующей ноде m_packet
    virtual void slotItemValueChanged(QTreeWidgetItem*, int);

    //выполняется после завершения редактирования(или выход из режима редактирования) пользователем любого итема (редактируемого)
    virtual void slotCloseEditor();

private:
    bool isEditableCol(int) const;

};



#endif // LXMLPACK_VIEW_H



