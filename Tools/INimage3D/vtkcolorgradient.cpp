#include "vtkcolorgradient.h"
#include "ui_vtkcolorgradient.h"

#include<QEvent>
#include<QMouseEvent>
#include<QPainter>
#include<QPainterPath>
#include<QColorDialog>

GradientShape::GradientShape(QWidget *widget, ShapeStyle style)
{
    if (widget)
    {
        m_parent = widget;
        m_parent->installEventFilter(this);
        widget->setStyleSheet("background-color:rgb(255,255,255)}");
        m_maxH = m_parent->height();
        m_maxW = m_parent->width();
    }
    m_shapeStyle = style;
    m_linePen = QPen(QColor(5, 5, 5, 255), 2 /*SolidLine*/);
    if (style == ShapeStyle::Default)
    {
        m_points.clear();
        int deltaX = 30, deltaY = 0;
        int delta = (m_maxH - 2 * deltaY) / 12;
        m_points << QPointF(35, m_maxH - 7) << QPointF(m_maxW - 35, m_maxH - deltaY - 10 * delta);
        //左上点 右下点
        m_xyRect = QRectF(QPointF(35, m_maxH - deltaY - 10 * delta), QPointF(m_maxW - 35, m_maxH));
    }
    else
    {
        m_points.clear();
        int deltaX = 35, deltaY = 0, deltaH = 20;
        int delta = (m_maxW - 2 * deltaX) / 10;
        int Y = (m_maxH - deltaY - deltaH) / 2;
        m_points << QPointF(deltaX, Y) << QPointF(deltaX + 10 * delta, Y);
        QColor color(255, 255, 255);
        m_colors.push_back(color);
        m_colors.push_back(color);
        m_xyRect = QRectF(QPointF(deltaX, Y), QPointF(deltaX + 10 * delta, Y));
    }
    m_slope = 80;
}

inline QRectF GradientShape::PointInRectX(int i)const
{
    QPointF point = m_points.at(i);
    double w = 10;
    double x = point.x() - w / 2;
    return QRectF(x, 0, w, m_maxH);
}
inline QRectF GradientShape::getPointRect(int i)const
{
    QPointF point = m_points.at(i);
    double w = 10, h = 10;
    double x = point.x() - w / 2;
    double y = point.y() - h / 2;
    return QRectF(x, y, w, h);
}

void GradientShape::movePoints(int index, const QPointF &point, bool update)
{
    int len = m_points.size();
    if (index >= 0 && index < len)
    {
        QPointF pt    = point;
        double left   = m_xyRect.left();
        double right  = m_xyRect.right();
        double top    = m_xyRect.top();
        double bottom = m_xyRect.bottom();

        if (pt.x() < left)
        {
            pt.setX(left);
        }
        else if (pt.x() > right)
        {
            pt.setX(right);
        }
        if (index > 0 && index < len - 1)
        {
            if (pt.x() >= m_points[index + 1].x())
            {
                pt.setX(m_points[index + 1].x() - 5);
            }
            else if (pt.x() <= m_points[index - 1].x())
            {
                pt.setX(m_points[index -1].x() + 5);
            }
        }
        if (index == 0 || index == len - 1)
        {
            pt.setX(m_points[index].x());
        }
        if (pt.y() < top)
        {
            pt.setY(top);
        }
        else if (pt.y() > bottom)
        {
            pt.setY(bottom);
        }

        m_points[index] = pt;
        m_parent->update();
    }
    
}

void GradientShape::paintPointsLines()
{
    QPainter painter(m_parent);
    painter.setRenderHint(QPainter::Antialiasing);
    if (m_shapeStyle == ShapeStyle::Default)
    {
        // draw curve
        //QPainterPath path;
        //path.moveTo(m_points.at(0));
        //int size = m_points.size();
        //for (int i = 0; i < size; i++)
        //{
        //    QPointF pt1 = m_points.at(i - 1);
        //    QPointF pt2 = m_points.at(i);
        //
        //    double distance = pt2.x() - pt1.x();
        //    path.cubicTo(pt1.x() + distance / 2, pt1.y(), pt1.x() + distance / 2, pt2.y(), pt2.x(), pt2.y());
        //}
        //painter.drawPath(path);
        painter.drawPolyline(m_points);

        int len = m_points.size();
        if (m_slope < 0)
        {
            m_slope = 1;
        }
        else if (m_slope > 100)
        {
            m_slope = 100;
        }
        QPolygonF slopePoints;
        for (int i = 0; i < len; i++)
        {
            QRectF bounds = getPointRect(i);
            painter.drawEllipse(bounds);

            QPointF ptH = m_points.at(i);
            int deltH = m_xyRect.height() - ptH.y();
            if (deltH > 0)
            {
                ptH.setY(m_xyRect.height() - deltH*m_slope/100);
            }
            slopePoints.append(ptH);
        }
        painter.drawPolyline(slopePoints);
    }
    else
    {
        int len = m_points.size();
        for (int i = 0; i < len; i++)
        {
            QRectF bounds = getPointRect(i);
            painter.drawRect(bounds);
            painter.fillRect(bounds, QBrush(m_colors.at(i)));
        }
    }
    // test 和绘制XY坐标系的范围
    //QColor red(255, 0, 0);
    //QPen pen(red);
    //painter.setPen(red);
    //painter.drawRect(m_xyRect);

}

void GradientShape::paintRuler()
{
    QPainter painter(m_parent);
    painter.setRenderHint(QPainter::Antialiasing);

    //XY刻度
    //Y
    if (m_shapeStyle == ShapeStyle::Default)
    {
        //XY轴线
        QPolygonF points;
        int deltaX = 30, deltaY = 0;
        int delta = (m_maxH - 2 * deltaY) / 12;
        QPointF pointO(deltaX, m_maxH - deltaY), pointX(m_maxW - deltaX, m_maxH - deltaY), pointY(deltaX, deltaY + delta);
        points << pointY << pointO << pointX;
        painter.setPen(m_linePen);
        painter.drawPolyline(points);
        for (int i = 0; i < 11; i++)
        {
            painter.drawLine(QPointF(deltaX, m_maxH - deltaY - i * delta), QPointF(deltaX / 2, m_maxH - deltaY - i * delta));
            painter.drawText(QPointF(deltaX / 2 - 10, m_maxH - deltaY - i * delta - delta / 2 + 7), QString::number(i / 10.00, 'f', 2));
        }
    }
    else
    {
        //color bar
        QPolygonF points;
        int deltaX = 35, deltaY = 0, deltaH = 20;
        QPointF point0(deltaX, deltaY), point1(deltaX, m_maxH - deltaY - deltaH), point2(m_maxW - deltaX, m_maxH - deltaY - deltaH), point3(m_maxW - deltaX, deltaY);
        points << point0 << point1 << point2 << point3 << point0;
        painter.setPen(m_linePen);
        painter.drawPolyline(points);

        int delta = (m_maxW - 2 * deltaX) / 10;
        int j = -2000, dis = 2000 / 5;
        for (int i = 0; i < 11; i++)
        {
            painter.drawLine(QPointF(deltaX + i * delta, m_maxH - deltaY - deltaH), QPointF(deltaX + i * delta, m_maxH - deltaY - deltaH + 7));
            painter.drawText(QPointF(deltaX + i * delta - 3, m_maxH - deltaY - deltaH + 18), QString::number(j));
            j += dis;
        }
    }

}

bool GradientShape::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_parent)
    {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        int index = -1, removeindex = -1, len = m_points.size();
        QPointF clickPoint = mouseEvent->pos();
        switch (event->type())
        {
            case QEvent::MouseButtonDblClick:
            {
                //if (m_shapeStyle == ShapeStyle::ColorStyle){//}
                for (int i = 0; i < len; i++)
                {
                    QPainterPath path, pathPointX;
                    path.addRect(getPointRect(i));
                    pathPointX.addRect(PointInRectX(i));
                    if (path.contains(clickPoint))
                    {
                        index = i;
                        removeindex = i;
                    }
                    else if (pathPointX.contains(clickPoint))
                    {
                        index = i;
                    }
                    if (index > -1)
                    {                  
                        break;
                    }
                }
                if (mouseEvent->button() == Qt::LeftButton)
                {
                    if (index == -1)
                    {
                        int pos = 0;
                        for (int i = 0; i < len; i++)
                        {
                            if (m_points.at(i).x() > clickPoint.x())
                            {
                                pos = i;
                                break;
                            }
                        }
                        if (m_shapeStyle == ShapeStyle::ColorStyle)
                        {
                            bool bok;
                            QColor color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &bok));
                            if (bok)
                            {
                                color.setAlpha(255);
                                m_colors.insert(pos, color);
                            }
                            m_points.insert(pos, QPointF(clickPoint.x(), m_points.at(0).y()));
                        }
                        else
                        {
                            m_points.insert(pos, clickPoint);
                        }
                        m_parent->update();
                        m_currentIndex = pos;
                    }
                    else
                    {
                        if (m_shapeStyle == ShapeStyle::ColorStyle)
                        {
                            bool bok;
                            QColor color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &bok));
                            if (bok)
                            {
                                color.setAlpha(255);
                                m_colors.replace(index, color);
                                m_parent->update();
                            }
                        }
                        m_currentIndex = index;
                    }
                }
                else if(mouseEvent->button() == Qt::RightButton)
                {
                    if (removeindex > 0 && removeindex < len - 1)
                    {
                        m_points.remove(removeindex);
                        m_colors.removeAt(removeindex);
                        m_parent->update();
                    }
                }
                break;
            }
            case QEvent::MouseButtonPress:
            {
                if (mouseEvent->button() == Qt::LeftButton)
                {
                    for (int i = 0; i < len; i++)
                    {
                        QPainterPath path, pathPointX;
                        path.addRect(getPointRect(i));
                        pathPointX.addRect(PointInRectX(i));
                        if (path.contains(clickPoint))
                        {
                            index = i;
                            removeindex = i;
                        }
                        else if (pathPointX.contains(clickPoint))
                        {
                            index = i;
                        }
                        if (index > -1)
                        {                  
                            break;
                        }
                    }
                    m_currentIndex = index;
                }
                break;
            }
            case QEvent::MouseMove:
                if (m_currentIndex > -1)
                {
                    QMouseEvent *mouseEvent = (QMouseEvent *)event;
                    movePoints(m_currentIndex, clickPoint, true);
                }
                break;
            case QEvent::MouseButtonRelease:
            {
                m_currentIndex = -1;
                break;
            }

            case QEvent::Resize:
            {
                m_maxH = m_parent->height();
                m_maxW = m_parent->width();
                break;
            }
            case QEvent::Show:
            {
                m_maxH = m_parent->height();
                m_maxW = m_parent->width();
                break;
            }
            case QEvent::Paint:
            {
                //QMouseEvent *mousePoint = (QMouseEvent *)event;
                //QPointF point = mousePoint->pos();
                paintRuler();
                paintPointsLines();
                return true;
            }
            default:
                break;
        }
    }
    return false;
}


//++++++++++++++++++++++++++++++++++++++++++++++++

VtkColorGradient::VtkColorGradient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VtkColorGradient)
{
    ui->setupUi(this);

    //setStyleSheet("background-color:rgb(255,255,255)}");
    m_gradientShape = new GradientShape(ui->m_gwidget);
    m_colorBar = new GradientShape(ui->m_colorWidget, ShapeStyle::ColorStyle);
}

VtkColorGradient::~VtkColorGradient()
{
    delete ui;
}
