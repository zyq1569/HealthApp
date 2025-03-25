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
	m_linePen = QPen(QColor(5, 5, 5, 255),2 /*SolidLine*/);
    if (style == ShapeStyle::Default)
    {
        m_points.clear();
        int deltaX = 30, deltaY = 0;
        int delta = (m_maxH - 2 * deltaY) / 12;
        m_points << QPointF(35, m_maxH - 7) << QPointF(m_maxW - 35, m_maxH - deltaY - 10 * delta);
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
    }
}

inline QRectF GradientShape::PointInRect(int i)const
{
    QPointF point = m_points.at(i);
    double w = 10;
    double x = point.x() - w / 2;
    return QRectF(x, 0, w, m_maxH);
}
inline QRectF GradientShape::getPointRect(int i)const
{
    QPointF point = m_points.at(i);
    double w = 10,h = 10;
    double x = point.x() - w / 2;
    double y = point.y() - h / 2;
    return QRectF(x, y, w, h);
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
        for (int i = 0; i < len; i++)
        {
            QRectF bounds = getPointRect(i);
            painter.drawEllipse(bounds);
        }
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

}

void GradientShape::paintRuler()
{
	QPainter painter(m_parent);
	painter.setRenderHint(QPainter::Antialiasing);
	
    //XY¿Ì¶È
    //Y
    if (m_shapeStyle == ShapeStyle::Default)
    {
        //XYÖáÏß
        QPolygonF points;
        int deltaX = 30, deltaY = 0;
        int delta = (m_maxH - 2 * deltaY) / 12;
        QPointF pointO(deltaX,m_maxH- deltaY), pointX(m_maxW- deltaX,m_maxH- deltaY), pointY(deltaX, deltaY+ delta);
        points << pointY << pointO << pointX;
        painter.setPen(m_linePen);
        painter.drawPolyline(points);
        for (int i = 0; i < 11; i++)
        {
            painter.drawLine(QPointF(deltaX, m_maxH - deltaY - i * delta), QPointF(deltaX / 2, m_maxH - deltaY - i * delta));
            painter.drawText(QPointF(deltaX /2-10, m_maxH - deltaY - i * delta - delta / 2+7), QString::number(i/10.00, 'f', 2));
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
            painter.drawLine(QPointF(deltaX+i* delta, m_maxH - deltaY - deltaH), QPointF(deltaX + i * delta, m_maxH - deltaY - deltaH+7));
            painter.drawText(QPointF(deltaX + i * delta -3, m_maxH - deltaY - deltaH + 18), QString::number(j));
            j += dis;
        }
    }

}

bool GradientShape::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == m_parent)
	{
		switch (event->type())
		{
		    case QEvent::MouseButtonDblClick:
		    {
                //if (m_shapeStyle == ShapeStyle::ColorStyle)
                {
			        QMouseEvent *mouseEvent = (QMouseEvent *)event;
			        QPointF clickPoint = mouseEvent->pos();
                    int index = -1, len = m_points.size();
                    for (int i = 0; i < len; i++)
                    {
                        QPainterPath path,pathPoint;
                        path.addRect(PointInRect(i));
                        pathPoint.addRect(getPointRect(i));
                        if (pathPoint.contains(clickPoint))
                        {
                            index = i;
                            if (m_shapeStyle == ShapeStyle::ColorStyle)
                            {
                                bool bok;
                                QColor color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &bok));
                                if (bok)
                                {
                                    color.setAlpha(255);
                                    m_colors.replace(i, color);
                                }
                            }
                            break;
                        }
                        else if (path.contains(clickPoint))
                        {
                            index = i;
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
                            m_currentIndex = index;
                        }
                    }              
                    return true;
                }
			    break;
		    }
		    case QEvent::MouseButtonPress:
		    case QEvent::MouseButtonRelease:
		    case QEvent::MouseMove:
		    case QEvent::Resize:
		    {
			    m_maxH = m_parent->height();
			    m_maxW = m_parent->width();
		    }
		    case QEvent::Show:
		    {
			    m_maxH = m_parent->height();
			    m_maxW = m_parent->width();
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
