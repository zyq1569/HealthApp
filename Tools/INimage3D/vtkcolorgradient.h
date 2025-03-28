#ifndef VTKCOLORGRADIENT_H
#define VTKCOLORGRADIENT_H

#include"qeditorbyvalues.h"

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


    inline QRectF getPointRect(int i, double w = 10, double h = 10)const;
    inline QRectF PointInRectX(int i)const;

public slots:
    void updateslope(int vl);

signals:
    void update3D();

public:
	QWidget *m_parent;
    bool m_synData;
    QRectF m_xyRect;
public:
    inline QPolygonF getPoints()
    {
        return m_points;
    }
    inline QList<QColor> getColors()
    {
        return m_colors;
    }
    inline void setGrayMaxMin(int max, int min)
    {
        m_grayMax = max;
        m_grayMin = min;
    }
private:
	QPolygonF m_points, m_gradientPoints;
    int m_delta, m_currentIndex, m_maxH, m_maxW;
    int m_slope;//1--->100
	QPen m_linePen;
	QList<QColor> m_colors;
    ShapeStyle m_shapeStyle;
    int m_grayMax, m_grayMin;
	
};
//++++++++++++++++++++++++++


class VtkColorGradient : public QWidget
{
    Q_OBJECT

public:
    explicit VtkColorGradient(QWidget *parent = nullptr);
    ~VtkColorGradient();
protected:
    //virtual void closeEvent(QCloseEvent *event);
    void updateDataVtkColorStyle();

protected slots:
    void update3D();

private:
    Ui::VtkColorGradient *ui;

public:
	GradientShape *m_gradientShape, *m_colorBar;
    QWidget *m_parentViewer;
    VtkColorStyle m_vtkColorStyle;

private:
    int m_grayMax, m_grayMin;
};

#endif // VTKCOLORGRADIENT_H
