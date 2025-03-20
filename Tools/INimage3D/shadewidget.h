
#ifndef UDGSHADEWIDGET_H
#define UDGSHADEWIDGET_H

#include <QWidget>

class HoverPoints;

class ShadeWidget : public QWidget
{
Q_OBJECT
public:
    enum ShadeType
	{
        RedShade,
        GreenShade,
        BlueShade,
        BlackShade,
        ARGBShade,
		GradientShade,
		ColorShade
    };

    ShadeWidget(QWidget *parent);
    ShadeWidget(ShadeType type, QWidget *parent);

    void setGradientStops(const QGradientStops &stops);

    void paintEvent(QPaintEvent *e);

    QSize sizeHint() const
    {
        return QSize(150, 40);
    }
    QPolygonF points() const;
    void setPoints(QPolygonF p);

    HoverPoints* hoverPoints() const
    {
        return m_hoverPoints;
    }

    uint colorAt(int x);

signals:
    void colorsChanged();

private:
    void generateShade();

    ShadeType m_shade_type;
    QImage m_shade;
    HoverPoints *m_hoverPoints;
    QLinearGradient m_alpha_gradient;
};

#endif
