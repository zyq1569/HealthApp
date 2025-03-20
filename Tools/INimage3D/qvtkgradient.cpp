#include "QvtkGradient.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QScrollBar>

#include "shadewidget.h"
#include "hoverpoints.h"

QvtkGradient::QvtkGradient(QWidget *parent)
 : QWidget(parent)
{
	setStyleSheet("background-color:rgb(240,240,240)}");

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(1);
    vbox->setMargin(1);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(6);
    hbox->setMargin(0);

    QLabel *nameLabel = new QLabel(tr("Name"), this);
    m_nameLineEdit = new QLineEdit(this);

    hbox->addWidget(nameLabel);
    hbox->addWidget(m_nameLineEdit);



	m_widget = new QWidget(this);
	int totalLength = 120;
	int visibleLength = 100;
	QScrollBar *VscrollArea = new QScrollBar(Qt::Vertical, m_widget);
	VscrollArea->setFixedWidth(20);
	VscrollArea->setValue(60);

	m_gradientShade = new ShadeWidget(ShadeWidget::GradientShade, m_widget);
	m_gradientShade->setFixedHeight(200);
	QHBoxLayout *vLayout = new QHBoxLayout(m_widget);
	vLayout->setMargin(0);
	vLayout->addWidget(m_gradientShade);
	vLayout->addWidget(VscrollArea);

	m_colorShade = new ShadeWidget(ShadeWidget::ColorShade, this);
	m_colorShade->setFixedHeight(20);

    vbox->addWidget(m_widget);
	vbox->addWidget(m_colorShade);
    vbox->addLayout(hbox);
	vbox->addStretch(0);	
	

    connect(m_nameLineEdit, SIGNAL(textChanged(const QString&)), SLOT(setTransferFunctionName(const QString&)));
    connect(m_gradientShade, SIGNAL(colorsChanged()), SLOT(pointsUpdated()));
    connect(m_colorShade, SIGNAL(colorsChanged()), SLOT(pointsUpdated()));

    connect(this, SIGNAL(gradientStopsChanged(const QvtkGradientStops&)), SLOT(setTransferFunction(const QvtkGradientStops&)));

	this->setStyleSheet("background-color:rgb(240,240,240)}");
}

QvtkGradient::~ QvtkGradient()
{
}

void QvtkGradient::setTransferFunction(/*const TransferFunction &transferFunction*/)
{
    //if (m_transferFunction == transferFunction)
    //{
    //    return;
    //}

    //m_nameLineEdit->setText(transferFunction.name());

   // QvtkGradientStops gradientStops;

    //QList<double> points = transferFunction.keys();
	//
    //foreach (double x, points)
    //{
    //    gradientStops << QvtkGradientStop((x - m_minimum) / (m_maximum - m_minimum), transferFunction.get(x));
    //}

    //setGradientStops(gradientStops);
    pointsUpdated();
}

//const TransferFunction &QvtkGradient::getTransferFunction() const
//{
//    return m_transferFunction;
//}

inline static bool x_less_than(const QPointF &p1, const QPointF &p2)
{
    return p1.x() < p2.x();
}

void QvtkGradient::pointsUpdated()
{
    //double w = m_gradientShade->width();
	//
    //QGradientStops stops;
	//
    //QPolygonF points;
	//
	//
    //points += m_gradientShade->points();
    //points += m_colorShade->points();
	//
    //qSort(points.begin(), points.end(), x_less_than);
	//
    //for (int i = 0; i < points.size(); ++i)
    //{
    //    double x = int(points.at(i).x());
    //    if (i < points.size() - 1 && x == points.at(i + 1).x())
    //    {
    //        continue;
    //    }
	//
    //    if (x / w > 1)
    //    {
    //        return;
    //    }
	//
    //    stops << QGradientStop(x / w, m_gradientShade->colorAt(int(x)));
    //}
	//
	//m_colorShade->setGradientStops(stops);
	//
    //emit gradientStopsChanged(stops);
}

static void set_shade_points(const QPolygonF &points, ShadeWidget *shade)
{
    shade->hoverPoints()->setPoints(points);
    shade->hoverPoints()->setPointLock(0, HoverPoints::LockToLeft);
    shade->hoverPoints()->setPointLock(points.size() - 1, HoverPoints::LockToRight);
    shade->update();
}

void QvtkGradient::setGradientStops(const QGradientStops &stops)
{
    QPolygonF pts_alpha;

    double h_alpha = m_colorShade->height();

    for (int i = 0; i < stops.size(); ++i)
    {
        double pos = stops.at(i).first;
        QRgb color = stops.at(i).second.rgba();

        pts_alpha << QPointF(pos * m_colorShade->width(), h_alpha - qAlpha(color) * h_alpha / 255);
    }

    set_shade_points(pts_alpha, m_colorShade);
}

void QvtkGradient::setTransferFunction(const QGradientStops &stops)
{
    //m_transferFunction.clear();
	//
    //for (int i = 0; i < stops.size(); i++)
    //{
    //    m_transferFunction.set(m_minimum + stops.at(i).first * (m_maximum - m_minimum), stops.at(i).second, stops.at(i).second.alphaF());
    //}
}

void QvtkGradient::setTransferFunctionName(const QString &name)
{
    //m_transferFunction.setName(name);
}
