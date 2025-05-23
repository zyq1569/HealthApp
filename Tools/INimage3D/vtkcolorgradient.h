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
class VtkColorGradient;

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
    inline void setGrayMaxMin(int max, int min, int numberPixels = 0)
    {
        m_grayMax      = max;
        m_grayMin      = min;
        m_numberPixels = numberPixels;
    }
    void setVtkColor(VtkColorGradient *vtkcolor);

    inline void setgrayZoom(double zoom)
    {
        m_grayZoom = zoom;
    }
private:
	QPolygonF m_points, m_gradientPoints;
    int m_delta, m_currentIndex, m_maxH, m_maxW;
    int m_slope;//1--->100
	QPen m_linePen;
	QList<QColor> m_colors;
    ShapeStyle m_shapeStyle;
    int m_grayMax, m_grayMin, m_numberPixels;
    VtkColorGradient *m_vtkcolor;
    double m_grayZoom = 1;
	
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
    void updateImage3D();
    void saveParametersOfImage3D();
    void loadFileValues();

private:
    Ui::VtkColorGradient *ui;

public:
	GradientShape *m_gradientShape, *m_colorBar;
    QWidget *m_parentViewer;
    VtkColorStyle m_vtkColorStyle;

public:
    int m_grayMax, m_grayMin;
    int m_imageGrayHis[4096] = { 0 };
    int m_lValue = -1, m_hValue = -1;
    int m_numberPixels;
};

#endif // VTKCOLORGRADIENT_H
