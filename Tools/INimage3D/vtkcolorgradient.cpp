#include "vtkcolorgradient.h"
#include "ui_vtkcolorgradient.h"

#include<QEvent>
#include<QMouseEvent>

GradientShape::GradientShape(QWidget *widget)
{
	if (widget)
	{
		m_parent = widget;
		m_parent->installEventFilter(this);
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
		case QEvent::Show:
		case QEvent::Paint:
		{
			QMouseEvent *mousePoint = (QMouseEvent *)event;
			QPointF point = mousePoint->pos();
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

	m_gradientShape = new GradientShape(ui->m_gwidget);
}

VtkColorGradient::~VtkColorGradient()
{
    delete ui;
}
