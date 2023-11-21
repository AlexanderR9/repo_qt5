#ifndef LSEARCH_OBJECT_H
#define LSEARCH_OBJECT_H

#include <QObject>

class QTableWidget;
class QLabel;
class QListWidget;
class QLineEdit;
class QTimer;


///////////LSearch//////////////////////////
class LSearch: public QObject
{
    Q_OBJECT
public:
    struct LStructSearch
    {
        LStructSearch() :table(NULL), list(NULL), label(NULL), caption(QString()) {}
        LStructSearch(QLabel *l, QTableWidget *tw = NULL, QListWidget *lw = NULL, QString s = QString())
            :table(tw), list(lw), label(l), caption(s) {}

        QTableWidget 		*table;
        QListWidget  		*list;
        QLabel				*label;
        QString				 caption;
    };

    LSearch(const QLineEdit*, QObject *parent = NULL);
    virtual ~LSearch() {}

    void exec();
    void addTable(QTableWidget *tw, QLabel *l = NULL, QString s = QString());
    void addList(QListWidget *lw, QLabel *l = NULL, QString s = QString());
    void updateText();
    void setDelay(int);

    inline void setColIndex(int i) {m_colIndex = i;}

protected:
    const QLineEdit *m_edit;
    QTimer	*m_timer;
    QList<LStructSearch> 	m_structs;
    int	m_colIndex;
    int m_delay;
    bool m_timeout;

    int visibleRows(int) const;
    int rowCount(int) const;
    void setText(int);
    void search(const QString&, LStructSearch&);
    void serchOnTable(const QStringList&, QTableWidget *tw);
    void serchOnList(const QStringList&, QListWidget *tw);

protected slots:
    void slotSearch(const QString&);
    void slotTimer();

signals:
    void signalSearched();  //выполняется каждый раз после произведенного фильтра

};



#endif // LSEARCH_H


