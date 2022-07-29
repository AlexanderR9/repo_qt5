 #ifndef LCHART_H
 #define LCHART_H

 #include <QDialog>
 #include "ui_lsimpledialog.h"

class QSettings;
class QMouseEvent;
class QResizeEvent;
class QPainter;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;


// LChartParams
struct LChartParams
{
    LChartParams() :pointsColor(Qt::black), lineColor(Qt::black) {}
    LChartParams(QColor c1, QColor c2) :pointsColor(c1), lineColor(c2) {}

    QList<QPointF> points; //список точек одного графика 
    QColor pointsColor;
    QColor lineColor;
};


// LChartMousePos
struct LChartMousePos
{
    LChartMousePos() :x(-1), y(-1), cross_color(QColor(25, 25, 250)), mode_axis_x_value(0) {reset();}

    // координады указателя мыши на виджете графика
    int x;
    int y;

    //точки захвата области для увеличения части график
    QPoint drag_point; //в момент нажатия кнопки мыши
    QPoint drop_point; //в момент отпускания кнопки мыши
    
    //коэффициенты положения мыши относительно начала осей и их длины , насколько удалена точка от начала оси по отношению к длине оси, принимают значения [0 .. 1]
    double dx_factor;
    double dy_factor;


    QPointF cross_point; //значение точки на графике, куда наведен указатель мыши в текущий момент
    QColor cross_color; // цвет перекрестия на графике при движении указателя мыши
    int scale_step; // количество шагов уменьшения/увеличения картинки (0 - исходный размер)
    bool visible; //видимость перекрестия на графике при движении указателя мыши
    int mode_axis_x_value; //режим отображения текстового значения кординаты X cross_point на оси (0-на том же уровне, что и остальные метки, 1 - выше, 2 - ниже)

    void drag() {drag_point.setX(x); drag_point.setY(y);}
    void drop() {drop_point.setX(x); drop_point.setY(y);}
    bool needRepaintDragRect() const  {return (drag_point.x() > 0);}
    


    void resetDragDrop() {drag_point.setX(-1); drag_point.setY(-1); drop_point.setX(-1); drop_point.setY(-1);}
    void resetCrossPoint() {cross_point.setX(-1); cross_point.setY(-1);}
    bool invalidPos() const {return (x <= 0 || y <= 0);}
    void reset() {visible = true; scale_step = 0; resetCrossPoint(); resetDragDrop();}
    void upScale() {scale_step++;}
    void downScale() {scale_step--; if (scale_step < 0) scale_step = 0;}
    double scaleFactor() const;
    void outRange() {x = y = dx_factor = dy_factor = -1;}
    void setCrossColor(QColor c) {cross_color = c;}
    void setXAxisTextViewMode(int t) {mode_axis_x_value = t;}

    static double scaleFactorStep() {return 1.2;} // коэфиициент уменьшения/увеличения после каждого шага

};


// LChartAxisParams
struct LChartAxisParams
{
    enum XValuesType {xvtSimple = 0, xvtDateTime, xvtTime, xvtDate};

    LChartAxisParams() {reset();}

    QPointF min; //минимальные значения точек среди всех графиков (x и y), соответствует началу осей
    QPointF max; //максимальные значения точек среди всех графиков (x и y)
    QPointF cur_min; //минимальные значения точек при изменении масштаба
    QPointF cur_max; //максимальные значения точек при изменении масштаба
    
    bool min_max_fixed; //признак того, что значения min и max фиксированные, на переменные cur_min и cur_max это не распространяется

    
    int precisionX; //точность значений на оси X
    int precisionY; //точность значений на оси Y
    int marksXInterval; //расстояние между поперечными метками на оси X в пикселях
    int marksYInterval; //расстояние между поперечными метками на оси Y в пикселях

    //текущие длины осей на виджете в пикселях, 
    //требуется пересчет при изменении размеров виджета
    int xLen; 
    int yLen; 

    int xType; //тип значений по оси Х

    //отступ осей графика от краев виджета слева и снизу
    int x_offset;
    int y_offset;

    int rt_offset; //отступ осей графика от краев виджета сверху и справа

    //отступ текстовых значений от осей графика
    int text_offset_x;
    int text_offset_y;


    inline int xOffset() const {return x_offset;} //отступ картинки графика от краев виджета по ширине
    inline int yOffset() const {return y_offset;} //отступ картинки графика от краев виджета по вертикали
    inline int rtOffset() const {return rt_offset;} //отступ картинки графика от краев виджета сверху и справа

    //текущая точка начала координатных осей (реальные координаты на виджете), по сути это отступ на виджете от левого нижнего угла
    //требуется пересчет точки при изменении размеров виджета
    QPoint zero_point; 

    QColor color; //цвет осей
    int width; //толщина линии осей
    bool visible; //видимость осей

    static int arrowLen() {return 7;} //длина проекции стрелок на концах осей в пикселях (если 0, то оси без стрелок)
    static int arrowDeviation() {return 2;} //отклонение концов линии стрелок от осей в пикселях
    static int marksLen() {return 4;} //длина поперечных меток на осях в пикселях
    static int symbolWidth() {return 8;} // ширина одного символа в текстовой надписи в пикселях

    static QString dateTimeMask() {return QString("dd.MM.yyyy hh:mm");} //формат для вывода значений по оси X при xType == xvtDateTime
    static QString timeMask() {return QString("hh:mm:ss");} //формат для вывода значений по оси X при xType == xvtTime
    static QString dateMask() {return QString("dd.MM.yyyy");} //формат для вывода значений по оси X при xType == xvtDate

    inline void recalcZeroPoint(int h) {if (h > yOffset()) zero_point.setY(h - yOffset());} //пересчет точки начала координатных осей
    inline void setPrecision(int px, int py) {precisionX = px; precisionY = py;}

    void recalcAxisLen(int, int); //пересчет длин осей
    void reset(); //сброс всех параметров
    void recalc(int, int); //пересчет длин осей и zero_point
    void resetMinMax(); //сброс значений переменных min и max
    void syncCurMinMax(); //присвоить переменым cur_min и cur_max те же координаты, что и у min и max
    void updateMinMax(const QPointF&, bool); //обновление значений переменных min и max
    QString valueByXType(const double&) const; //конвертирует в текстовое значение на оси Х, взависимости от параметра xType
    QString strMinMax() const; //вывод инфы о мин. макс. оригинальном и текущем
    QString strPoint(const QPointF&) const; //вывод инфы о координатах точки
    bool isOutPoint(const QPointF&) const; // признак того, что точка выходит за рамки области графика (координаты точки реального графика)
    void setFixedMinMax(const double&, const double&, const double&, const double&); //установка фиксированных значений min и max
    void setMarksInterval(int, int); //задание интервалов между метками на осях, отрицательное значение значит что интервал не изменится
    void setOffsets(int, int, int); //задание отступов осей графика от краев виджета слева и снизу и от правого верхнего угла, отрицательное значение значит что отступ не изменится
    void setTextOffsets(int, int); //задание отступов текстовых значений от осей графика, отрицательное значение значит что отступ не изменится

    //marks
    QList<int> marksXList() const; // список координат X на виджете для расставления меток по оси Х
    QList<int> marksYList() const; // список координат X на виджете для расставления меток по оси Y
    QList<double> marksXValues() const; // список значений под метками на оси X
    QList<double> marksYValues() const; // список значений под метками на оси Y
    inline int xMarksCount() const {return marksXList().count();} // количество меток на оси X в текущий момент
    inline int yMarksCount() const {return marksYList().count();} // количество меток на оси Y в текущий момент
    int marksTextXOffset() const {return text_offset_x;} //отступ текстовых значений меток от оси X в пикселях (вних)
    int marksTextYOffset() const {return text_offset_y;} //отступ текстовых значений меток от оси Y в пикселях (влево)

};


//  LChartWidget - chart class
class LChartWidget : public QWidget
{
    Q_OBJECT
public:
    LChartWidget(QWidget *parent = NULL);
    virtual ~LChartWidget() {}

    void setBackgroundColor(const QColor&);
    void clearPoints(); //удаляет все точки всех графиков, графики при это остаются, далее перерисовка
    void fullClear();//полная очистка графика, далее перерисовка

    //axis params
    inline void setAxisColor(const QColor &color) {m_axis.color = color;}
    inline void setAxisWidth(int w) {m_axis.width = w;}
    inline void setAxisVisible(bool b) {m_axis.visible = b;}
    inline void setAxisXType(int t) {m_axis.xType = t;}
    inline void setAxisPrecision(int px, int py) {m_axis.setPrecision(px, py);}
    inline void setAxisFixedMinMax(const double &x1, const double &x2, const double &y1, const double &y2) {m_axis.setFixedMinMax(x1, x2, y1, y2);}
    inline void setAxisMarksInterval(int a, int b) {m_axis.setMarksInterval(a, b);}
    inline void setAxisOffsets(int a, int b, int c) {m_axis.setOffsets(a, b, c);}
    inline void setAxisTextOffsets(int a, int b) {m_axis.setTextOffsets(a, b);}
    inline QString strMinMax() const {return m_axis.strMinMax();} //вывод инфы о мин. макс. оригинальном и текущем


    //other params
    inline void setVisibleMouseCross(bool b) {m_mousePos.visible = b;}    
//    inline void setColorMouseCross(const QColor &color) {m_mousePos.cross_color = color;}    
    inline void setLineWidth(int w) {m_lineWidth = w;}
    inline void setPointSize(int w) {m_pointSize = w;}
    inline void setOnlyPoints(bool b) {m_onlyPoints = b;}
    inline void addChart(const LChartParams &chart) {/*qDebug("add chart"); */m_charts.append(chart);} 
    inline int chartsCount() const {return m_charts.count();} //количество линий (графиков)
    inline void setCrossColor(QColor c) {m_mousePos.setCrossColor(c);}
    inline void setCrossXAxisTextViewMode(int t) {m_mousePos.setXAxisTextViewMode(t);}

    void setChartPointsColor(const QColor &color, int i = -1); //задать цвет точек заданного графика (при i<0 для всех графиков)
    void setChartLineColor(const QColor &color, int i = -1);//задать цвет линии заданного графика (при i<0 для всех графиков)
    void clearChartPoints(int i = -1); //удаление точек заданного графика, но график при этом остается (при i<0 удаляются точки со всех графиков)
    void addChartPoints(const QList<QPointF>&, int i); // добавление точек к заданному графику
    void removeChart(int i = -1); //удаление заданного график с индексом i (при i<0 удаляются все графики из контейнера m_charts)

    //пересчет мин. макс. с учетов масштабирования и cross точки, далее перерисовка картинки
    void updateAxis();
    

protected:
    QList<LChartParams> m_charts;
    LChartMousePos m_mousePos;
    LChartAxisParams m_axis;

    int m_pointSize;
    int m_lineWidth;
    bool m_onlyPoints; //рисовать только точки

    void init();

    //recalc and repaint funcs
    virtual void recalcMinMax();
    virtual void recalcMinMaxScale();
    virtual void recalcCrossPoint();
    virtual void recalcPosFactors();
    virtual void repaintAxis(QPainter&);
    virtual void repaintAxisText(QPainter&);
    virtual void repaintMouseCross(QPainter&);
    virtual void repaintDragRect(QPainter&);
    virtual void repaintCharts(QPainter&);
    virtual void repaintChart(int, QPainter&);
    virtual void rescaleByDrag();
    

    // qt events funcs
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
//    void dragEnterEvent(QDragEnterEvent*);
//    void dragMoveEvent(QDragMoveEvent*);
//    void dropEvent(QDropEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);


private:
    void convertPoint(const QPointF&, QPointF&); //пересчет координат точки графика в координаты виджета для корректного отображения этой точки
    void convertPoints(const QList<QPointF>&, QList<QPointF>&); 

};




///////////////LChartDialog//////////////////////////////
class LChartDialog : public QDialog, public Ui::LSimpleDialog
{
    Q_OBJECT
public:
    LChartDialog(QWidget *parent = 0);
    virtual ~LChartDialog() {}

    void updateChart() {if (m_chart) m_chart->updateAxis();} //пересчет и обновление всей картинки

//    void clearPoints() {if (m_chart) m_chart->clearPoints();}
    void addChart(const LChartParams &chart) {if (m_chart) m_chart->addChart(chart);}
    void clearChartPoints(int i) {if (m_chart) m_chart->clearChartPoints(i);}
    void addChartPoints(const QList<QPointF> &points, int i) {if (m_chart) m_chart->addChartPoints(points, i);}
//    void repaintChart(int i) {if (m_chart) m_chart->repaintChart(i);}
//    void addChartPoint(const QPointF &point, int i) {QList<QPointF> points; points << point; addChartPoints(points, i);}
    //void setPrecisionX(int p) {if (m_chart) m_chart->setPrecisionX(p);}
    //void setPrecisionY(int p) {if (m_chart) m_chart->setPrecisionY(p);}


    void setAxisMarksInterval(int a, int b) {if (m_chart) m_chart->setAxisMarksInterval(a, b);}
    void setAxisPrecision(int px, int py) {if (m_chart) m_chart->setAxisPrecision(px, py);} //задать точности значений на осях
    void setAxisFixedMinMax(const double &x1, const double &x2, const double &y1, const double &y2) {if (m_chart) m_chart->setAxisFixedMinMax(x1, x2, y1, y2);}
    void setVisibleMouseCross(bool b) {if (m_chart) m_chart->setVisibleMouseCross(b);}    
    void setAxisColor(const QColor &color) {if (m_chart) m_chart->setAxisColor(color);}
    void setCrossColor(const QColor &color) {if (m_chart) m_chart->setCrossColor(color);}
    void setAxisVisible(bool b) {if (m_chart) m_chart->setAxisVisible(b);}


    void setLineWidth(int w) {if (m_chart) m_chart->setLineWidth(w);}
    void setPointSize(int w) {if (m_chart) m_chart->setPointSize(w);}
    void setOnlyPoints(bool b) {if (m_chart) m_chart->setOnlyPoints(b);}
    
    void save(QSettings&);
    void load(QSettings&);


protected slots:
    virtual void slotApply() {close();}

protected:
    LChartWidget *m_chart;

};





 #endif



