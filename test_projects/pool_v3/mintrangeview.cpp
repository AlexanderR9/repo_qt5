#include "mintrangeview.h"


#include <QDebug>

/////////////////// HandleItem ///////////////////////////
HandleItem::HandleItem(qreal y, qreal height, QColor color, qreal minX, qreal maxX, QGraphicsItem* parent)
    :QGraphicsObject(parent),
      m_y(y),
      m_height(height),
      m_color(color),
      m_minX(minX),
      m_maxX(maxX)
{
    setFlags(ItemIsMovable | ItemSendsScenePositionChanges | ItemIgnoresTransformations);
    setCursor(Qt::SizeHorCursor);
    setPos(m_minX, m_y); // начальное положение

}
QRectF HandleItem::boundingRect() const
{
    return QRectF(-2, -m_height/2, 6, m_height);
}
QVariant HandleItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange)
    {
        QPointF p = value.toPointF();
        // фиксируем по Y, ограничиваем по X
        p.setY(m_y);
        if (p.x() < m_minX) p.setX(m_minX);
        if (p.x() > m_maxX) p.setX(m_maxX);
        return p;
    }
    else if (change == ItemPositionHasChanged)
    {
        emit priceChanged(scenePos().x());
    }

    return QGraphicsObject::itemChange(change, value);
}
void HandleItem::paint(QPainter *p, const QStyleOptionGraphicsItem*, QWidget*)
{
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(m_color);
    p->drawRoundedRect(boundingRect(), 2, 2);

    // «носик» вниз
    QPolygonF tri;
    tri << QPointF(0,-m_height/2) << QPointF(6, -m_height/2-8) << QPointF(-6,-m_height/2-8);
    p->drawPolygon(tri);
}


/////////////////// CurrentPriceItem ///////////////////////////
CurrentPriceItem::CurrentPriceItem(qreal y, qreal height, QColor color)
    :m_y(y),
    m_h(height),
    m_color(color)
{
    setZValue(-1);
}
QRectF CurrentPriceItem::boundingRect() const
{
    return QRectF(-0.5, -m_h/2, 1.0, m_h);
}
void CurrentPriceItem::paint(QPainter *p, const QStyleOptionGraphicsItem*, QWidget*)
{
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(QPen(m_color, 1, Qt::DashLine));
    p->drawLine(QPointF(0,-m_h/2), QPointF(0, m_h/2));
}



/////////////////// PriceScaleItem ///////////////////////////
PriceScaleItem::PriceScaleItem(qreal y, qreal width, qreal height)
    :m_y(y),
    m_w(width),
    m_h(height)
{
    setZValue(-2);
}
QRectF PriceScaleItem::boundingRect() const
{
    return QRectF(0, m_y - m_h/2, m_w, m_h);
}
void PriceScaleItem::paint(QPainter *p, const QStyleOptionGraphicsItem*, QWidget*)
{
    p->setRenderHint(QPainter::TextAntialiasing);
    const qreal baseY = m_y;
    p->setPen(QPen(Qt::gray, 0));
    p->drawLine(QPointF(0, baseY), QPointF(m_w, baseY));

    // Подбираем шаг делений в сценовых единицах (цена == x)
    qreal range = m_w;
    qreal desiredPx = 80; // хотим подпись примерно каждые 80px
    // т.к. 1 сц.ед = 1 px при единичном масштабе, это упрощённо
    // можно усложнить, учитывая transform().
    qreal roughStep = qMax<qreal>(1, qRound(range / (range / desiredPx)));

    // Округлим шаг в «красивые» числа
    QList<qreal> steps = {1, 2, 5, 10, 20, 25, 50, 100, 200, 250, 500, 1000};
    qreal step = steps.back();
    for (qreal s: steps)
    {
        if (s >= roughStep) {step = s; break;}
    }

    QFont f = p->font();
    f.setPointSizeF(f.pointSizeF()*0.9);
    p->setFont(f);

    for (qreal x = 0; x <= m_w + 0.001; x += step)
    {
        p->setPen(QPen(Qt::darkGray, 0));
        p->drawLine(QPointF(x, baseY-6), QPointF(x, baseY+6));
        p->setPen(Qt::gray);
        QString label = QString::number(x, 'f', (step<1)?2:0);
        p->drawText(QPointF(x+3, baseY-8), label);
    }
}



/////////////////// MintRangeView ///////////////////////////
MintRangeView::MintRangeView(QWidget *parent)
    :QGraphicsView(parent),
    m_scene(NULL),
    m_minPrice(0.0),
    m_maxPrice(1.5),
    m_left(NULL),
    m_right(NULL)
{
    initScene();
    //initClamps();

/*
    const qreal height = 80;      // высота «линейки»
    const qreal centerY = 0;      // фиксируем всё по Y=0
    const qreal width = m_maxPrice - m_minPrice;

    // Сцена в координатах цен:
    //m_scene->setSceneRect(m_minPrice, -height/2 - 20, width, height + 40);
    //m_scene->setSceneRect(0, 0, 500, 100);

    // Чтобы 1px = 1цена по X, положим transform по умолчанию и впишем вид
    //fitInView(m_scene->sceneRect(), Qt::KeepAspectRatioByExpanding);

    // Шкала
    auto scale = new PriceScaleItem(centerY, width, height);
    scale->setPos(m_minPrice, 0);
  //  m_scene->addItem(scale);

    // Маркер текущей цены
    m_current = new CurrentPriceItem(centerY, height);
    m_current->setPriceX(width/3);
 //   m_scene->addItem(m_current);



    // Взаимные ограничения (не пересекаться)
    connect(m_left, SIGNAL(priceChanged(qreal)), this, SLOT(slotPriceChanged(p)));
    connect(m_right, SIGNAL(priceChanged(qreal)), this, SLOT(slotPriceChanged(p)));
    */

    /*

    connect(m_left, &HandleItem::priceChanged, this, [this](qreal px){
        if (px > m_right->price()) {
            m_left->setPos(m_right->price(), 0);
        }
        // обновим clamp правого ползунка (левая граница = позиция левого)
        m_right->setClamp(px, m_maxPrice);
        emit rangeChanged(m_left->price(), m_right->price());
    });
    connect(m_right, &HandleItem::priceChanged, this, [this](qreal px){
        if (px < m_left->price()) {
            m_right->setPos(m_left->price(), 0);
        }
        m_left->setClamp(m_minPrice, px);
        emit rangeChanged(m_left->price(), m_right->price());
    });
    */
}
void MintRangeView::initClamps()
{
    const qreal centerY = 0;      // фиксируем всё по Y=0
    const qreal height = 80;      // высота «линейки»

    // Два ползунка (левая/правая границы)
    m_left  = new HandleItem(centerY, height, QColor("#7CDC00"), m_minPrice, m_maxPrice);
    m_right = new HandleItem(centerY, height, QColor("#3B5757"), m_minPrice, m_maxPrice);
    m_left->setObjectName("left_clamp");
    m_right->setObjectName("right_clamp");
    m_left->setPos(20, centerY);
    m_right->setPos(60, centerY);

    m_scene->addItem(m_left);
    m_scene->addItem(m_right);
}
void MintRangeView::initScene()
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    m_scene->setBackgroundBrush(QColor("#FFF8DC"));

    /*
    m_scene->setSceneRect(-10, -10, 110, 110);
    QGraphicsTextItem * i_text = m_scene->addText("test");
    i_text->setPos(80, 50);
    m_scene->addRect(0, 0, 15, 10);


    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(ScrollHandDrag);
    setMouseTracking(true);
    */

}
void MintRangeView::slotPriceChanged(qreal p)
{
    qDebug()<<QString("MintRangeView::slotPriceChanged   sender[%1]  p=%2").arg(sender()->objectName()).arg(p);

}
void MintRangeView::slotSetCurrentPrice(qreal price)
{
    price = qBound(m_minPrice, price, m_maxPrice);
    m_current->setPriceX(price);
    emit currentPriceChanged(price);
}
void MintRangeView::slotSetRange(qreal minPrice, qreal maxPrice)
{
    if (minPrice >= maxPrice) return;

    m_minPrice = minPrice; m_maxPrice = maxPrice;
    const qreal width = m_maxPrice - m_minPrice;
    QRectF r(m_minPrice, sceneRect().y(), width, sceneRect().height());
    scene()->setSceneRect(r);
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);

    // обновить клампы
    m_left->setClamp(m_minPrice, m_right->price());
    m_right->setClamp(m_left->price(), m_maxPrice);
}



