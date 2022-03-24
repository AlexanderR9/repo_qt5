 #include "lchart.h"

 #include <QtCore/qmath.h>
 #include <QDialogButtonBox>
 #include <QPushButton>
 #include <QVBoxLayout>
 #include <QDateTime>
 #include <QSettings>
 #include <QPainter>
 #include <QPainterPath>
 #include <QDebug>
 #include <QMouseEvent>
 #include <QDragMoveEvent>
 #include <QDrag>
 #include <QPixmap>
 #include <QMimeData>
 #include <QDropEvent>

 #define DEFAULT_BACKGROUND_COLOR	QColor(240, 240, 240)

     
//////////////// LChartWidget ///////////////////////////////
LChartWidget::LChartWidget(QWidget *parent)
    :QWidget(parent)
{
    init();
    setBackgroundColor(DEFAULT_BACKGROUND_COLOR);
    setMouseTracking(true);
//    setAcceptDrops(true);
}
void LChartWidget::init()
{
    m_pointSize = 1;
    m_lineWidth = 1;
    m_onlyPoints = false;

    m_axis.reset();
    m_mousePos.reset();
}
void LChartWidget::setBackgroundColor(const QColor &color)
{
    QPalette p;
    p.setBrush(QPalette::Background, color);
    setPalette(p);
}
void LChartWidget::setChartPointsColor(const QColor &color, int chart_index)
{
    if (chart_index < 0)
    {
	for (int i=0; i<m_charts.count(); i++)
	    m_charts[i].pointsColor = color;
    }
    else
    {
	if (chart_index < m_charts.count())
	    m_charts[chart_index].pointsColor = color;	    
    }
}
void LChartWidget::setChartLineColor(const QColor &color, int chart_index)
{
    if (chart_index < 0)
    {
	for (int i=0; i<m_charts.count(); i++)
	    m_charts[i].lineColor = color;
    }
    else
    {
	if (chart_index < m_charts.count())
	    m_charts[chart_index].lineColor = color;	    
    }
}
void LChartWidget::addChartPoints(const QList<QPointF> &list, int chart_index)
{
    if (chart_index < 0 || chart_index >= m_charts.count()) return;
    m_charts[chart_index].points.append(list);
}
void LChartWidget::clearChartPoints(int chart_index)
{
    if (chart_index < 0)
    {
	for (int i=0; i<m_charts.count(); i++)
	    m_charts[i].points.clear();
    }
    else
    {
	if (chart_index < m_charts.count())
	    m_charts[chart_index].points.clear();
    }
}
void LChartWidget::removeChart(int chart_index)
{
    if (chart_index < 0)
    {
	m_charts.clear();
    }
    else
    {
	if (chart_index < m_charts.count())
	    m_charts.removeAt(chart_index);
    }
}
void LChartWidget::updateAxis()
{
    recalcMinMax();
    recalcMinMaxScale();
    recalcCrossPoint();
    update();
}
void LChartWidget::clearPoints()
{
    clearChartPoints();
    updateAxis();
}
void LChartWidget::fullClear()
{
    if (m_charts.isEmpty()) return;

    clearChartPoints();
    removeChart();
    updateAxis();        
}
void LChartWidget::recalcMinMax()
{
    if (m_axis.min_max_fixed) return;

    m_axis.resetMinMax();
    if (m_charts.isEmpty()) return;

    for (int i=0; i<m_charts.count(); i++)
	for (int j=0; j<m_charts.at(i).points.count(); j++)
	    m_axis.updateMinMax(m_charts.at(i).points.at(j), (i==0 && j==0));

//    qDebug()<<QString("LChartWidget::recalcMinMax()  %1 scale_step=%2").arg(m_axis.strMinMax()).arg(m_mousePos.scale_step);
}
void LChartWidget::rescaleByDrag()
{
//    qDebug("LChartWidget::rescaleByDrag()");
    m_mousePos.scale_step = 0;
    int x_len = qAbs(m_mousePos.drag_point.x() - m_mousePos.drop_point.x());
    int y_len = qAbs(m_mousePos.drag_point.y() - m_mousePos.drop_point.y());
    double xf_len = double(x_len)/double(m_axis.xLen);
    double yf_len = double(y_len)/double(m_axis.yLen);

    int drag_x = qMin(m_mousePos.drag_point.x(), m_mousePos.drop_point.x());
    int drag_y = qMax(m_mousePos.drag_point.y(), m_mousePos.drop_point.y());
//    int drop_x = qMax(m_mousePos.drag_point.x(), m_mousePos.drop_point.x());
//    int drop_y = qMin(m_mousePos.drag_point.y(), m_mousePos.drop_point.y());
    double xf_start = double(drag_x - m_axis.xOffset())/double(m_axis.xLen);
    double yf_start = double(m_axis.yLen - (drag_y - m_axis.rtOffset()))/double(m_axis.yLen);
    
    double rdx = m_axis.cur_max.x() - m_axis.cur_min.x();
    double rdy = m_axis.cur_max.y() - m_axis.cur_min.y();
    double new_min_x = rdx*xf_start + m_axis.cur_min.x();
    double new_min_y = rdy*yf_start + m_axis.cur_min.y();
    m_axis.cur_min.setX(new_min_x);
    m_axis.cur_min.setY(new_min_y);
    m_axis.cur_max.setX(new_min_x + rdx*xf_len);
    m_axis.cur_max.setY(new_min_y + rdy*yf_len);
}
void LChartWidget::recalcPosFactors()
{
    if (m_mousePos.invalidPos()) return;

    int pdx = m_mousePos.x - m_axis.xOffset();            
    int pdy = m_axis.yLen - (m_mousePos.y - m_axis.rtOffset());            
    if (pdx <= 0 || pdy <= 0) return;

    m_mousePos.dx_factor = double(pdx)/double(m_axis.xLen);
    m_mousePos.dy_factor = double(pdy)/double(m_axis.yLen);
}
void LChartWidget::recalcCrossPoint()
{
    m_mousePos.resetCrossPoint();
    if (m_mousePos.invalidPos()) return;
 
    double dx = m_axis.cur_max.x() - m_axis.cur_min.x();
    double dy = m_axis.cur_max.y() - m_axis.cur_min.y();
    int pdx = m_mousePos.x - m_axis.xOffset();            
    int pdy = m_axis.yLen - (m_mousePos.y - m_axis.yOffset());            
    if (pdx <= 0 || pdy <= 0) return;

    m_mousePos.cross_point.setX(m_axis.cur_min.x() + dx*m_mousePos.dx_factor);
    m_mousePos.cross_point.setY(m_axis.cur_min.y() + dy*m_mousePos.dy_factor);
}
void LChartWidget::recalcMinMaxScale()
{
//    qDebug("LChartWidget::recalcMinMaxScale()");
    if (m_mousePos.scale_step == 0 || m_mousePos.invalidPos())
    {
	m_axis.syncCurMinMax();
//	qDebug()<<QString("LChartWidget::recalcMinMaxScale()  %1 scale_step=%2").arg(m_axis.strMinMax()).arg(m_mousePos.scale_step);
	return;
    }

    double sf = m_mousePos.scaleFactor();

    double dx = m_axis.max.x() - m_axis.min.x();
    double px = m_axis.min.x() + dx*m_mousePos.dx_factor;
    m_axis.cur_min.setX(px - (dx*m_mousePos.dx_factor/sf));
    m_axis.cur_max.setX(px + (dx*(1-m_mousePos.dx_factor)/sf));
    	
    double dy = m_axis.max.y() - m_axis.min.y();
    double py = m_axis.min.y() + dy*m_mousePos.dy_factor;
    m_axis.cur_min.setY(py - (dy*m_mousePos.dy_factor/sf));
    m_axis.cur_max.setY(py + (dy*(1-m_mousePos.dy_factor)/sf));
}
void LChartWidget::repaintChart(int index, QPainter &painter)
{
    if (index < 0 || index >= m_charts.count()) return;

    QList<QPointF> converted_points;
    convertPoints(m_charts.at(index).points, converted_points);
    if (converted_points.isEmpty()) return;

    QPen pen;
    if (!m_onlyPoints)
    {
	pen.setColor(m_charts.at(index).lineColor);
	pen.setWidth(m_lineWidth);
	painter.setPen(pen);

	bool isOut = false;
	QPainterPath p_path(converted_points.first());
	for (int i=1; i<converted_points.count(); i++)
	{
	    if (m_axis.isOutPoint(m_charts.at(index).points.at(i)))
	    {
		isOut = true;
	    }
	    else
	    {
		if (isOut) p_path.moveTo(converted_points.at(i));
		else p_path.lineTo(converted_points.at(i));
		isOut = false;
	    }
	}

	painter.drawPath(p_path);
    }

    if (m_pointSize <= 0) return;

    pen.setColor(m_charts.at(index).pointsColor);
    pen.setWidth(m_pointSize);
    painter.setPen(pen);
    for (int i=0; i<converted_points.count(); i++)
    {
	if (!m_axis.isOutPoint(m_charts.at(index).points.at(i)))
	    painter.drawPoint(converted_points.at(i));
    }
}
void LChartWidget::repaintCharts(QPainter &painter)
{
    int n = chartsCount();
    for (int i=0; i<n; i++)
	repaintChart(i, painter);
}
void LChartWidget::repaintAxisText(QPainter &painter)
{
    if (!m_axis.visible) return;

    int x_text = 0;
    int y_text = 0;
    int sw = LChartAxisParams::symbolWidth();

    // x values
    QList<int> x_marks = m_axis.marksXList();	
    QList<double> x_values = m_axis.marksXValues();	
    y_text = m_axis.zero_point.y() + m_axis.marksTextXOffset();
    for (int i=0; i<x_marks.count(); i++)
    {
	QString str_value = m_axis.valueByXType(x_values.at(i));
	int symbols = str_value.length();
	x_text = x_marks.at(i) - int(sw*double(symbols)/2);
    	painter.drawText(x_text, y_text, str_value);
    }

    // y values
    QList<int> y_marks = m_axis.marksYList();	
    QList<double> y_values = m_axis.marksYValues();	
    x_text = m_axis.zero_point.x() - m_axis.marksTextYOffset();
    for (int i=0; i<y_marks.count(); i++)
    {
	QString str_value = QString::number(y_values.at(i), 'f', m_axis.precisionY);
	int y_text = y_marks.at(i) + sw/2;
    	painter.drawText(x_text, y_text, str_value);
    }
}

void LChartWidget::repaintAxis(QPainter &painter)
{
    if (!m_axis.visible) return;

    QPen pen(m_axis.color);    
    pen.setWidth(m_axis.width);
    painter.setPen(pen);

    const QPoint &zp = m_axis.zero_point;
    int x2 = zp.x() + m_axis.xLen;
    int y2 = zp.y() - m_axis.yLen;
    int mlh = m_axis.marksLen()/2;

    //axis
    painter.drawLine(zp, QPoint(zp.x(), y2));
    painter.drawLine(zp, QPoint(x2, zp.y()));
    
    // arrows
    if (m_axis.arrowLen() > 0)
    {
	painter.drawLine(QPoint(zp.x(), y2), QPoint(zp.x()-m_axis.arrowDeviation(), y2+m_axis.arrowLen()));
	painter.drawLine(QPoint(zp.x(), y2), QPoint(zp.x()+m_axis.arrowDeviation(), y2+m_axis.arrowLen()));
	painter.drawLine(QPoint(x2, zp.y()), QPoint(x2-m_axis.arrowLen(), zp.y()-m_axis.arrowDeviation()));
	painter.drawLine(QPoint(x2, zp.y()), QPoint(x2-m_axis.arrowLen(), zp.y()+m_axis.arrowDeviation()));
    }

    //marks
    if (m_axis.marksLen() > 0)
    {
	QList<int> x_marks = m_axis.marksXList();	
	for (int i=0; i<x_marks.count(); i++)
	    painter.drawLine(QPoint(x_marks.at(i), zp.y()-mlh), QPoint(x_marks.at(i), zp.y()+mlh));

	QList<int> y_marks = m_axis.marksYList();	
	for (int i=0; i<y_marks.count(); i++)
	    painter.drawLine(QPoint(zp.x()-mlh, y_marks.at(i)), QPoint(zp.x()+mlh, y_marks.at(i)));
    }
}
void LChartWidget::repaintDragRect(QPainter &painter)
{
//    if (m_mousePos.invalidPos()) return;
//    if (!m_mousePos.visible) return;
    
    QPen pen(m_mousePos.cross_color);    
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawRect(QRect(m_mousePos.drag_point, QPoint(m_mousePos.x, m_mousePos.y)));

}
void LChartWidget::repaintMouseCross(QPainter &painter)
{
    if (m_mousePos.invalidPos()) return;
    if (!m_mousePos.visible) return;

    QPen pen(m_mousePos.cross_color);    
    pen.setWidth(1);
    painter.setPen(pen);

    int x = m_mousePos.x;
    int y = m_mousePos.y;
    int dx = m_axis.xOffset();
    int dy = m_axis.yOffset();
    int sw = LChartAxisParams::symbolWidth();

    //draw cross
    painter.drawLine(dx, y, dx+m_axis.xLen, y);
    painter.drawLine(x, dy, x, dy+m_axis.yLen);

    //draw text values of cross point
    QString str_value = m_axis.valueByXType(m_mousePos.cross_point.x());
    int x_text = x - int(sw*double(str_value.length())/2);

    int y_text = m_axis.zero_point.y() + m_axis.marksTextXOffset();;
    switch(m_mousePos.mode_axis_x_value)
    {
	case 1: {y_text -= 12; break;}
	case 2: {y_text += 12; break;}
	default: break;
    }

    painter.drawText(x_text, y_text, str_value);
    str_value = QString::number(m_mousePos.cross_point.y(), 'f', m_axis.precisionY);
    x_text = m_axis.zero_point.x() - m_axis.marksTextYOffset();
    y_text = y + sw;
    painter.drawText(x_text, y_text, str_value);
}

//////////////EVENTS///////////////////////
void LChartWidget::resizeEvent(QResizeEvent*)
{
    m_axis.recalc(width(), height());
}
void LChartWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    repaintAxis(painter);
    repaintAxisText(painter);
    repaintCharts(painter);

    if (m_mousePos.needRepaintDragRect()) repaintDragRect(painter);
    else repaintMouseCross(painter);
}
void LChartWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = m_axis.xOffset();
    int dy = m_axis.yOffset();
    int drt = m_axis.rtOffset();
    
    if (event->x() < dx || event->x() > (width() - drt) || event->y() < drt || event->y() > (height() - dy))
    { 
	m_mousePos.outRange();
	m_mousePos.resetDragDrop();
    }
    else
    {
	m_mousePos.x = event->x();
	m_mousePos.y = event->y();
	recalcPosFactors();
    }

    recalcCrossPoint();
    update();
}
void LChartWidget::wheelEvent(QWheelEvent *event)
{
    if (m_mousePos.invalidPos()) return;

    if (event->delta() > 0) 
    {
	m_mousePos.upScale();
    }
    else if (event->delta() < 0) 
    {
	m_mousePos.downScale();
    }
    else return;

    recalcMinMaxScale();
    update();	    
}
void LChartWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    //qDebug("LChartWidget::mouseDoubleClickEvent(QMouseEvent*)");
    m_mousePos.scale_step = 0; 
    m_mousePos.resetDragDrop();
    recalcMinMaxScale();
    update();
}
void LChartWidget::mouseReleaseEvent(QMouseEvent *event)
{
//    qDebug("LChartWidget::mouseReleaseEvent(QMouseEvent*)");

    if (m_mousePos.invalidPos() || event->button() != Qt::LeftButton || !m_mousePos.needRepaintDragRect())
    {
	m_mousePos.resetDragDrop();
	return;
    }

//    qDebug()<<QString("drop");
    m_mousePos.drop();
    rescaleByDrag();
    m_mousePos.resetDragDrop();
    update();

}
void LChartWidget::mousePressEvent(QMouseEvent *event)
{
//    qDebug("LChartWidget::mousePressEvent(QMouseEvent*)");

    if (m_mousePos.invalidPos()) return;
    if (event->button() != Qt::LeftButton) return;

//    qDebug("\n start drag");
    m_mousePos.drag();
}






//private funcs
void LChartWidget::convertPoint(const QPointF &p1, QPointF &p2)
{
    double dx = m_axis.cur_max.x() - m_axis.cur_min.x();
    double dy = m_axis.cur_max.y() - m_axis.cur_min.y();
    double dpx = ((dx == 0) ? 0.5 : (p1.x() - m_axis.cur_min.x())/dx);
    double dpy = ((dy == 0) ? 0.5 : (p1.y() - m_axis.cur_min.y())/dy);
    p2.setX(m_axis.zero_point.x() + m_axis.xLen*dpx);
    p2.setY(m_axis.zero_point.y() - m_axis.yLen*dpy);
}
void LChartWidget::convertPoints(const QList<QPointF> &list1, QList<QPointF> &list2)
{
    list2.clear();
    if (list1.isEmpty()) return;

    QPointF convert_point;
    for (int i=0; i<list1.count(); i++)
    {
	convertPoint(list1.at(i), convert_point);
	list2.append(convert_point);
    }
}



///////////////LChartDialog//////////////////////////////
LChartDialog::LChartDialog(QWidget *parent)
    :QDialog(parent),
    m_chart(NULL)
{
    setupUi(this);

    setWindowTitle(QObject::tr("Chart dialog!"));

    m_chart = new LChartWidget(this);    
    if (groupBox->layout()) delete groupBox->layout();
    QVBoxLayout *lay = new QVBoxLayout(0);
    groupBox->setLayout(lay);
    lay->addWidget(m_chart);
    groupBox->setTitle(QObject::tr("Chart"));

    buttonBox->button(QDialogButtonBox::Cancel)->setVisible(false);
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(slotApply()));
}
void LChartDialog::save(QSettings &settings)
{
    settings.setValue("chartdialog/geometry", saveGeometry());
}
void LChartDialog::load(QSettings &settings)
{
    restoreGeometry(settings.value("chartdialog/geometry").toByteArray());
}
        


/////////////////LChartAxisParams////////////////////////////
void LChartAxisParams::recalcAxisLen(int w, int h)
{
    xLen = w - xOffset() - rtOffset();
    if (xLen < 0) xLen = 0;
    yLen = h - yOffset() - rtOffset();
    if (yLen < 0) yLen = 0;
}
void LChartAxisParams::reset() 
{
    xLen = yLen = 0; 
    color.setRgb(20, 20, 100); 
    width = 1;
    visible = true;
    xType = LChartAxisParams::xvtSimple;

    x_offset = 60;
    y_offset = 30;
    rt_offset = 10;
    zero_point.setX(xOffset()); 

    text_offset_x = 18;
    text_offset_y = 50;

    precisionX = 0;
    precisionY = 2;
    marksXInterval = 150;
    marksYInterval = 100;
    min_max_fixed = false;
    resetMinMax();
}
void LChartAxisParams::resetMinMax()
{
    min = QPointF(0, 0);
    max = QPointF(1, 1);
    syncCurMinMax();
}
void LChartAxisParams::syncCurMinMax()
{
    cur_min.setX(min.x());
    cur_min.setY(min.y());
    cur_max.setX(max.x());
    cur_max.setY(max.y());
}
bool LChartAxisParams::isOutPoint(const QPointF &p) const
{
    if (p.x() < cur_min.x() || p.x() > cur_max.x()) return true;
    if (p.y() < cur_min.y() || p.y() > cur_max.y()) return true;
    return false;
}
void LChartAxisParams::recalc(int w, int h)
{
//    qDebug("LChartAxisParams::recalc");
    recalcAxisLen(w, h);
    recalcZeroPoint(h);
}
void LChartAxisParams::setFixedMinMax(const double &x_min, const double &x_max, const double &y_min, const double &y_max)
{
    min.setX(x_min);
    min.setY(y_min);
    max.setX(x_max);
    max.setY(y_max);
    syncCurMinMax();
    min_max_fixed = true;
}
QList<int> LChartAxisParams::marksXList() const
{
    QList<int> list;
    int x = xOffset();
    for (;;)
    {
	x += marksXInterval;
	if (x > (xOffset() + xLen)) break;
	list.append(x);
    }
    return list;
}
QList<int> LChartAxisParams::marksYList() const
{
    QList<int> list;
    int y = yOffset() + yLen;
    for (;;)
    {
        y -= marksYInterval;
        if (y < yOffset()) break;
        list.append(y);
    }
    return list;
}
void LChartAxisParams::updateMinMax(const QPointF &p, bool isFirst)
{
    if (isFirst)
    {
        min = p;
        max = p;
    }
    else
    {
        if (min.x() > p.x()) min.setX(p.x());
        if (min.y() > p.y()) min.setY(p.y());
        if (max.x() < p.x()) max.setX(p.x());
        if (max.y() < p.y()) max.setY(p.y());
    }
}
QList<double> LChartAxisParams::marksXValues() const
{
    QList<double> list;
    if (xLen < marksXInterval) return list;

    double d = cur_max.x() - cur_min.x();
    double factor = double(marksXInterval)/double(xLen);
    int n_marks = xMarksCount();
    for (int i=0; i<n_marks; i++)
	list.append(cur_min.x() + (i+1)*d*factor);

    return list;
}
QList<double> LChartAxisParams::marksYValues() const
{
//    qDebug()<<QString("LChartAxisParams::marksYValues() : ")<<strMinMax();
    QList<double> list;
    if (yLen < marksYInterval) return list;

    double d = cur_max.y() - cur_min.y();
    double factor = double(marksYInterval)/double(yLen);
    int n_marks = yMarksCount();
    for (int i=0; i<n_marks; i++)
	list.append(cur_min.y() + (i+1)*d*factor);

    return list;
}
QString LChartAxisParams::valueByXType(const double &x_value) const
{
    switch(xType)
    {
	case xvtSimple: {return QString::number(x_value, 'f', precisionX);}
	case xvtDateTime: 
	{
	    QDateTime dt = QDateTime::fromTime_t(uint(x_value));
	    if (!dt.isValid()) return QString("invalid datetime");
	    return dt.toString(dateTimeMask());
	}
	case xvtDate: 
	{
	    QDateTime dt = QDateTime::fromTime_t(uint(x_value));
	    if (!dt.isValid()) return QString("invalid date");
	    return dt.toString(dateMask());
	}
	case xvtTime: 
	{
	    QDateTime dt = QDateTime::fromTime_t(uint(x_value));
	    if (!dt.isValid()) return QString("invalid time");
	    return dt.toString(timeMask());
	}
	default: break;
    }
    return QString("err xtype");
}
/*
int LChartAxisParams::marksTextXOffset() const
{
    return symbolWidth()*2;
}
int LChartAxisParams::marksTextYOffset() const
{
    int left_symbols = QString(QString::number(qCeil(max.y()))).trimmed().length(); //максимальное количество знаков до запятой
    int n = symbolWidth()*(left_symbols + precisionY + 1);
    return (n + 5);
}
*/
void LChartAxisParams::setOffsets(int xo, int yo, int rt)
{
    if (rt > 0) rt_offset = rt;
    if (xo > 0) x_offset = xo;
    if (yo > 0) y_offset = yo;
    zero_point.setX(xOffset()); 
}
void LChartAxisParams::setTextOffsets(int xo, int yo)
{
    if (xo > 0) text_offset_x = xo;
    if (yo > 0) text_offset_y = yo;
}
QString LChartAxisParams::strMinMax() const
{
    QString s1 = QString("(%1; %2)").arg(QString::number(min.x(), 'f', precisionX)).arg(QString::number(min.y(), 'f', precisionY));
    QString s2 = QString("(%1; %2)").arg(QString::number(max.x(), 'f', precisionX)).arg(QString::number(max.y(), 'f', precisionY));
    QString s3 = QString("(%1; %2)").arg(QString::number(cur_min.x(), 'f', precisionX)).arg(QString::number(cur_min.y(), 'f', precisionY));
    QString s4 = QString("(%1; %2)").arg(QString::number(cur_max.x(), 'f', precisionX)).arg(QString::number(cur_max.y(), 'f', precisionY));
    return QString("MinMax info: min%1-max%2  cur_min%3-cur_max%4").arg(s1).arg(s2).arg(s3).arg(s4);
}
QString LChartAxisParams::strPoint(const QPointF &p) const
{
    return QString("x=%1  y=%2").arg(QString::number(p.x(), 'f', precisionX)).arg(QString::number(p.y(), 'f', precisionY));
}
void LChartAxisParams::setMarksInterval(int mxi, int myi)
{
    if (mxi > 10) marksXInterval = mxi;
    if (myi > 10) marksYInterval = myi;
}


/////////////////LChartMousePos////////////////////////////
double LChartMousePos::scaleFactor() const
{
    if (scale_step == 0) return 1;
    return qPow(scaleFactorStep(), scale_step);
}








