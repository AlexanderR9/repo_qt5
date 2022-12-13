#ifndef LXMLPACK_VIEW_H
#define LXMLPACK_VIEW_H


#include <QGroupBox>
#include <QTreeWidgetItem>


class LXMLPackObj;
class QTreeWidget;
class LXMLPackElement;


// LXMLPackViewItem
class LXMLPackViewItem : public QTreeWidgetItem
{
public:
    LXMLPackViewItem(LXMLPackElement*, QTreeWidgetItem *parent = NULL, bool kks_used = false);
    virtual ~LXMLPackViewItem() {}

    void changePackValue(int); //значение или отклонение изменилось, необходимо обновить его в пакете
    void setReadOnly(bool); //установить возможность редактирования значения или отклонения в пакете
    void updateValues(); //обновить значения
    void setDoublePrecision(quint8);

    inline bool isEditable() const {return m_editable;}


protected:
    LXMLPackElement *m_node;
    bool m_editable;

    void loadNodeChilds();
    void updateColumnsText();
    void updateColumnsColor();
    void updateValue();

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


    void setPacket(LXMLPackObj*); //установить экземпляр пакета для отображения
    void setPacketByteOrder(int); //устанавливает порядок байт для записи пакета в поток данных в m_packet
    void resizeColumns(); //подогнать размеры столбцов под контент
    void updateValues(); //обновить значения итемов
    void setExpandLevel(int); //раскрывает элементы дерева до заданной глубины
    void nextRandValues(); //обновить значения всех элементов пакета с учетом rand_deviation, (если rand_deviation == 0, то значение не измениться)
    void setSelectionRowsMode();
    bool kksUsed() const;


    //установить значение 1-й ноды по заданному пути.
    //путь указывается в виде набора уровней вложенности, начинается всегда с 'packet'
    //пример: packet/2/0/11
    //если по заданному пути нет ноды или ее тип не соответствует устанавливаему значению то в параметр bool запишется false
    void setIntValueByPath(QString, qint64, bool&);
    void setDoubleValueByPath(QString, double, bool&);


    // DataStream operation (in/out )
    void setPacketData(const QByteArray&, bool&, bool singleFloatPrecision = false); //записать массив байт в пакет
    void fromPacket(QByteArray&, bool singleFloatPrecision = false); //запись пакета в массив байт


    inline const LXMLPackObj* getPacket() const {return m_packet;}
    inline void setReadOnly(bool b) {m_readOnly = b;}
    inline bool isReadOnly() const {return m_readOnly;}
    inline void  setDoublePrecision(quint8 p) {m_doublePrecision = p;}
    inline bool invalid() const {return (m_rootItem == NULL);}


protected:
    QTreeWidget         *m_view;
    LXMLPackViewItem    *m_rootItem;
    LXMLPackObj         *m_packet;
    bool                 m_readOnly;
    quint8               m_doublePrecision;

    void initWidget();
    void reloadView();
    void resetView();

protected slots:
    void slotItemActivate(QTreeWidgetItem*, int);
    void slotItemValueChanged(QTreeWidgetItem*, int);

private:
    bool isEditableCol(int) const;

};



#endif // LXMLPACK_VIEW_H



