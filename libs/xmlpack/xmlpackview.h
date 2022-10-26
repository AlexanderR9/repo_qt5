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
    LXMLPackViewItem(const LXMLPackElement*, QTreeWidgetItem *parent = NULL);
    virtual ~LXMLPackViewItem() {}

protected:
    const LXMLPackElement     *m_node;

    void loadNodeChilds();
    void updateColumnsText();
    void updateColumnsColor();

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


    void setPacket(LXMLPackObj*);
    void resizeColumns();

    inline const LXMLPackObj* getPacket() const {return m_packet;}
    inline void setReadOnly(bool b) {m_readOnly = b;}
    inline bool isReadOnly() const {return m_readOnly;}


protected:
    QTreeWidget         *m_view;
    LXMLPackViewItem    *m_rootItem;
    LXMLPackObj         *m_packet;
    bool                 m_readOnly;


    void initWidget();
    void reloadView();
    void resetView();

};



#endif // LXMLPACK_VIEW_H






