#include "lsearch.h"
#include "lstring.h"

#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QListWidget>
#include <QTimer>
#include <QDebug>


/////////////LSearch/////////////////////////
LSearch::LSearch(const QLineEdit *edit, QObject *parent)
    :QObject(parent),
    m_edit(edit),
    m_timer(NULL),
    m_colIndex(-1),
    m_delay(-1),
    m_timeout(false)
{
    if (!m_edit) return;

    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearch(const QString&)));

}
void LSearch::setSearchEdit(const QLineEdit *edit)
{
    disconnect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearch(const QString&)));
    m_edit = NULL;
    if (!edit) return;

    m_edit = edit;
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearch(const QString&)));
}
void LSearch::setDelay(int v)
{
    m_timeout = false;
    if (v < 100) return;
    m_delay = v;
    if (m_timer) return;

    m_timer = new QTimer(this);
    m_timer->setInterval(m_delay);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

}
void LSearch::slotTimer()
{
    m_timer->stop();
    m_timeout = true;
    exec();
}
void LSearch::slotSearch(const QString &s)
{
    if (m_timer)
    {
        if (m_timeout)
        {
            m_timeout = false;
        }
        else
        {
            if (m_timer->isActive()) m_timer->stop();
            m_timer->start();
            return;
        }
    }

    //// try search   
    for (int i=0; i<m_structs.count(); i++)
    {
        search(s, m_structs[i]);
        setText(i);
    }
    emit signalSearched();
}
QStringList LSearch::searchElemets(const QString &s) const
{
    QStringList list;
    if (s.trimmed().isEmpty()) return list;
    if (!s.contains(";;")) {list.append(s); return list;}

    list = LString::trimSplitList(s, ";;", false);
    int n = list.count();
    for (int i=n-1; i>=0; i--)
    {
        if (list.at(i).trimmed().isEmpty() || list.at(i) == "!!")
            list.removeAt(i);
    }

    return list;
}
bool LSearch::inverseElement(const QString &s) const
{
    return (s.length() > 2 && s.left(2) == "!!");
}
bool LSearch::searchConditionalOk(const QString &baseTxt, QString searchTxt) const
{
    if (!searchTxt.isEmpty())
    {
        //qDebug()<<QString("LSearch::searchConditionalOk:  baseTxt[%1]  searchTxt[%2]").arg(baseTxt).arg(searchTxt);
        if (inverseElement(searchTxt)) //this NOT search_element
        {
            searchTxt = LString::strTrimLeft(searchTxt, 2);
            qDebug()<<QString("     this inverce item, NOT text [%1]").arg(searchTxt);
            return !baseTxt.contains(searchTxt);
        }
        return baseTxt.contains(searchTxt);
    }
    return true;
}
void LSearch::search(const QString &s, LStructSearch &obj)
{
    //qDebug()<<QString("LSearch::search, s=[%1]").arg(s);
    QStringList list(searchElemets(s));
    if (obj.table)
    {
        serchOnTable(list, obj.table);
        obj.table->resizeColumnsToContents();
    }
    else if (obj.list) serchOnList(list, obj.list);
}
void LSearch::serchOnTable(const QStringList &list, QTableWidget *tw)
{
    if (!tw) return;

    for (int i=0; i<tw->rowCount(); i++)
    {
        int n_vOk = 0;
        foreach (const QString &v, list) //iterable search elements
        {
            bool inv = inverseElement(v);
            bool v_ok = (inv ? true : false);
            for (int j=0; j<tw->columnCount(); j++)
            {
                QString i_text(tw->item(i, j)->text());
                if (!inv && searchConditionalOk(i_text, v)) {v_ok = true; break;}
                if (inv && !searchConditionalOk(i_text, v)) {v_ok = false; break;}
            }
            if (v_ok) n_vOk++;
        }
        tw->setRowHidden(i, (n_vOk < list.count()));
    }
}
void LSearch::serchOnList(const QStringList &list, QListWidget *lw)
{
    if (!lw) return;

    int rows = lw->count();
    for (int i=0; i<rows; i++)
    {
        int n_vOk = 0;
        foreach (const QString v, list)
        {
            QString i_text(lw->item(i)->text());
            if (searchConditionalOk(i_text, v)) n_vOk++;
        }
        lw->item(i)->setHidden(n_vOk < list.count());
    }
}
void LSearch::exec()
{
    if (m_edit)
        slotSearch(m_edit->text());
}
void LSearch::updateText()
{
    for (int i=0; i<m_structs.count(); i++)
    {
        setText(i);
        if (m_structs.at(i).table)
            m_structs.at(i).table->resizeColumnsToContents();
    }
}
void LSearch::addTable(QTableWidget *tw, QLabel *l, QString s)
{
    if (!tw) return;
    m_structs.append(LStructSearch(l, tw, NULL, s));
    tw->resizeColumnsToContents();
    tw->resizeRowsToContents();
}
void LSearch::addList(QListWidget *lw, QLabel *l, QString s)
{
    if (!lw) return;
    m_structs.append(LStructSearch(l, NULL, lw, s));
}
int LSearch::visibleRows(int struct_index) const
{
    if (struct_index < 0 || struct_index >= m_structs.count()) return -1;

    int n = 0;
    int rows = rowCount(struct_index);

    if (m_structs.at(struct_index).table)
    {
        for (int i=0; i<rows; i++)
            if (!m_structs.at(struct_index).table->isRowHidden(i)) n++;
    }
    else if (m_structs.at(struct_index).list)
    {
        for (int i=0; i<rows; i++)
            if (!m_structs.at(struct_index).list->isRowHidden(i)) n++;
    }

    return n;
}
int LSearch::rowCount(int struct_index) const
{
    if (struct_index < 0 || struct_index >= m_structs.count()) return -1;
    if (m_structs.at(struct_index).table)
        return m_structs.at(struct_index).table->rowCount();
    if (m_structs.at(struct_index).list)
        return m_structs.at(struct_index).list->count();
    return -1;
}
void LSearch::setText(int struct_index)
{
    if (struct_index < 0 || struct_index >= m_structs.count()) return;
    if (!m_structs.at(struct_index).label) return;
    QString s = QString("%1: %2/%3").arg(tr("Record number")).arg(visibleRows(struct_index)).arg(rowCount(struct_index));
    if (!m_structs.at(struct_index).caption.isEmpty()) s = QString("%1. %2").arg(m_structs.at(struct_index).caption).arg(s);
    m_structs.at(struct_index).label->setText(s);
}



