#ifndef MINTRANGEVIEW_H
#define MINTRANGEVIEW_H

#include <QGraphicsView>
#include <QGraphicsObject>
#include <QGraphicsItem>
#include <QPen>
#include <QBrush>

// HandleItem
class HandleItem : public QGraphicsObject
{
    Q_OBJECT
public:
    HandleItem(qreal, qreal, QColor, qreal, qreal, QGraphicsItem *parent = NULL);

    QRectF boundingRect() const override; // Рисуем «флажок» 10×(height) в сценовых единицах, но игнорируем масштаб (ItemIgnoresTransformations)

    inline void setClamp(qreal minX, qreal maxX) {m_minX = minX; m_maxX = maxX;}
    inline qreal price() const {return scenePos().x();}

protected:
    qreal m_y, m_height;
    QColor m_color;
    qreal m_minX, m_maxX;

    QVariant itemChange(GraphicsItemChange, const QVariant&) override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

signals:
    void priceChanged(qreal price);

};


// CurrentPriceItem
class CurrentPriceItem : public QGraphicsItem
{
public:
    CurrentPriceItem(qreal, qreal, QColor color=Qt::red);

    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

    inline void setPriceX(qreal x) {setPos(x, m_y);}

protected:
    qreal m_y, m_h;
    QColor m_color;

};


// PriceScaleItem
class PriceScaleItem : public QGraphicsItem
{
public:
    PriceScaleItem(qreal, qreal, qreal);

    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

protected:
    qreal m_y, m_w, m_h;

};


// MintRangeView
class MintRangeView : public QGraphicsView
{
    Q_OBJECT
public:
    MintRangeView(QWidget *parent = NULL);

protected:
    QGraphicsScene* m_scene;
    qreal m_minPrice, m_maxPrice;
    HandleItem *m_left;
    HandleItem *m_right;
    CurrentPriceItem *m_current;

    void initScene();
    void initClamps();


public slots:
    void slotSetCurrentPrice(qreal);
    void slotSetRange(qreal, qreal);

protected slots:
    void slotPriceChanged(qreal);

signals:
    void rangeChanged(qreal leftPrice, qreal rightPrice);
    void currentPriceChanged(qreal price);


};





#endif // MINTRANGEVIEW_H


