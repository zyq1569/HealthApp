#include "QvtkGradient.h"

#include <QLabel>
#include <QVBoxLayout>
#include "shadewidget.h"
#include "hoverpoints.h"

QvtkGradient::QvtkGradient(QWidget *parent)
 : QWidget(parent)
{
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

	m_red_shade = new ShadeWidget(this);// ShadeWidget::RedShade, this);
    m_green_shade = new ShadeWidget(ShadeWidget::GreenShade, this);
    m_blue_shade = new ShadeWidget(ShadeWidget::BlueShade, this);
    m_alpha_shade = new ShadeWidget(ShadeWidget::ARGBShade, this);

    vbox->addLayout(hbox);
    vbox->addWidget(m_red_shade);
	vbox->addWidget(m_green_shade);
	vbox->addWidget(m_blue_shade);
    vbox->addWidget(m_alpha_shade);

    connect(m_nameLineEdit, SIGNAL(textChanged(const QString&)), SLOT(setTransferFunctionName(const QString&)));
    connect(m_red_shade, SIGNAL(colorsChanged()), SLOT(pointsUpdated()));
    connect(m_green_shade, SIGNAL(colorsChanged()), SLOT(pointsUpdated()));
    connect(m_blue_shade, SIGNAL(colorsChanged()), SLOT(pointsUpdated()));
    connect(m_alpha_shade, SIGNAL(colorsChanged()), SLOT(pointsUpdated()));

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
    double w = m_alpha_shade->width();

    QGradientStops stops;

    QPolygonF points;

    points += m_red_shade->points();
    points += m_green_shade->points();
    points += m_blue_shade->points();
    points += m_alpha_shade->points();

    qSort(points.begin(), points.end(), x_less_than);

    for (int i = 0; i < points.size(); ++i)
    {
        double x = int(points.at(i).x());
        if (i < points.size() - 1 && x == points.at(i + 1).x())
        {
            continue;
        }
        QColor color((0x00ff0000 & m_red_shade->colorAt(int(x))) >> 16,
                     (0x0000ff00 & m_green_shade->colorAt(int(x))) >> 8,
                     (0x000000ff & m_blue_shade->colorAt(int(x))),
                     (0xff000000 & m_alpha_shade->colorAt(int(x))) >> 24);

        if (x / w > 1)
        {
            return;
        }

        stops << QGradientStop(x / w, color);
    }

    m_alpha_shade->setGradientStops(stops);

    emit gradientStopsChanged(stops);
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
    QPolygonF pts_red, pts_green, pts_blue, pts_alpha;

    double h_red = m_red_shade->height();
    double h_green = m_green_shade->height();
    double h_blue = m_blue_shade->height();
    double h_alpha = m_alpha_shade->height();

    for (int i = 0; i < stops.size(); ++i)
    {
        double pos = stops.at(i).first;
        QRgb color = stops.at(i).second.rgba();
        pts_red << QPointF(pos * m_red_shade->width(), h_red - qRed(color) * h_red / 255);
        pts_green << QPointF(pos * m_green_shade->width(), h_green - qGreen(color) * h_green / 255);
        pts_blue << QPointF(pos * m_blue_shade->width(), h_blue - qBlue(color) * h_blue / 255);
        pts_alpha << QPointF(pos * m_alpha_shade->width(), h_alpha - qAlpha(color) * h_alpha / 255);
    }

    set_shade_points(pts_red, m_red_shade);
    set_shade_points(pts_green, m_green_shade);
    set_shade_points(pts_blue, m_blue_shade);
    set_shade_points(pts_alpha, m_alpha_shade);
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
