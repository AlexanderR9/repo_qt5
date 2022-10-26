#ifndef MY_HTMLNODE_H
#define MY_HTMLNODE_H

//#include <QObject>
#include <QList>


//MyHTMLNodeBase
class MyHTMLNodeBase
{
public:
    MyHTMLNodeBase() :m_parent(NULL) {}
    MyHTMLNodeBase(const MyHTMLNodeBase *parent_node) :m_parent(parent_node) {}
    virtual ~MyHTMLNodeBase() {reset();}

    void reset() {qDeleteAll(m_childs); m_childs.clear();}
    virtual bool invalid() const = 0;

protected:
    QList<MyHTMLNodeBase*> m_childs;
    const MyHTMLNodeBase *m_parent;

};

//MyHTMLNode
class MyHTMLNode : public MyHTMLNodeBase
{
public:
    MyHTMLNode(const MyHTMLNodeBase *parent_node) :MyHTMLNodeBase(parent_node), m_data(QString()) {}

    inline bool isEmptyData() const {return m_data.trimmed().isEmpty();}
    inline void setData(const QString &data) {m_data = data.trimmed();}
    inline int dataSize() const {return m_data.length();}
    inline const QString& internalData() const {return m_data;}

    virtual bool invalid() const {return false;}

protected:
    QString m_data; //данные html внутри этого тега (без него самого)

};

//MyHTMLScriptNode
class MyHTMLScriptNode : public MyHTMLNode
{
public:
    MyHTMLScriptNode(const MyHTMLNodeBase *parent_node) :MyHTMLNode(parent_node) {resetPos();}

    inline void setPositions(int a, int b) {m_startPos = a; m_endPos = b;}
    virtual bool invalid() const;

protected:
    int m_startPos; //позиция с которой начинается вся нода в html коде, indexOf("<script")
    int m_endPos; //позиция на которой заканчивается вся нода в html коде, indexOf("</script>") + 8

    void resetPos() {m_startPos = m_endPos = -1;}

};




#endif // MY_HTMLNODE_H
