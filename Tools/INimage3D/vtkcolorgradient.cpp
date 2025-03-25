#include "vtkcolorgradient.h"
#include "ui_vtkcolorgradient.h"

#include<QEvent>
#include<QMouseEvent>
#include<QPainter>

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
        m_points << QPointF(m_maxW - 35, m_maxH - delta / 2 + 7);
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
			QMouseEvent *mousePoint = (QMouseEvent *)event;
			QPointF point = mousePoint->pos();
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
            return true;
		}
		default:
			break;
		}
	}
	return false;
}

void GradientShape::paintRuler()
{
	QPainter painter(m_parent);
	painter.setRenderHint(QPainter::Antialiasing);
	
    //XYøÃ∂»
    //Y
    if (m_shapeStyle == ShapeStyle::Default)
    {
        //XY÷·œﬂ
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
