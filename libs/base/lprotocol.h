 #ifndef LPROTOCOL_H
 #define LPROTOCOL_H

 #include <QTextEdit>
 #include <QGroupBox>



///////////LProtocolBox/////////////////////
class LProtocolBox : public QGroupBox
{
    Q_OBJECT
public:
    enum TextType {ttText = 0, ttErr, ttWarning, ttOk, ttFile, ttData, ttUnknow = -1};

    LProtocolBox(bool nt, QWidget *parent = 0);
    virtual ~LProtocolBox() {}

    void addText(QString s, int t = 0);

    inline void clearProtocol() {m_protocol->clear();}
    inline void addSpace() {m_protocol->append(QString());}

protected:
    QTextEdit           *m_protocol;
    bool                m_needTime;


};




#endif



