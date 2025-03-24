#ifndef VTKCOLORGRADIENT_H
#define VTKCOLORGRADIENT_H

#include <QWidget>

namespace Ui {
class VtkColorGradient;
}

//++++++++++++++++++++++++++
class GradientShape :    public QObject
{
	Q_OBJECT
public:
	GradientShape(QWidget *widget);
	bool eventFilter(QObject *obj, QEvent *event);

public:
	QWidget *m_parent;

private:
	QPolygonF m_points;
	int m_delta, m_currentIndex;
	QList<QColor> m_colors;
	
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
