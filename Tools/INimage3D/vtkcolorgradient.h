#ifndef VTKCOLORGRADIENT_H
#define VTKCOLORGRADIENT_H

#include<QWidget>
#include<Qpen>
namespace Ui 
{
	class VtkColorGradient;
}

//++++++++++++++++++++++++++
enum ShapeStyle
{
    Default,
    ColorStyle
};

class GradientShape :public QObject
{
	Q_OBJECT
public:

public:
	GradientShape(QWidget *widget, ShapeStyle style = ShapeStyle::Default);
	bool eventFilter(QObject *obj, QEvent *event);
	void paintRuler();
    void paintPointsLines();
    void movePoints(int index, const QPointF &point, bool update);


    inline QRectF getPointRect(int i)const;
    inline QRectF PointInRectX(int i)const;

public:
	QWidget *m_parent;

private:
	QPolygonF m_points, m_gradientPoints;
    int m_delta, m_currentIndex, m_maxH, m_maxW;
    int m_slope;//1--->100
	QPen m_linePen;
	QList<QColor> m_colors;
    ShapeStyle m_shapeStyle;
    QRectF m_xyRect;
	
};
//++++++++++++++++++++++++++


class VtkColorGradient : public QWidget
{
    Q_OBJECT

public:
    explicit VtkColorGradient(QWidget *parent = nullptr);
    ~VtkColorGradient();

private:
    Ui::VtkColorGradient *ui;

public:
	GradientShape *m_gradientShape, *m_colorBar;
};

#endif // VTKCOLORGRADIENT_H
