

#ifndef QEDITORBYVALUES_H
#define QEDITORBYVALUES_H

#include "ui_addvaluesbase.h"

#include "QIntervalEditor.h"

struct VtkColorPoint
{
	double m_X, m_Opacity;
	QColor m_Color;
};

class VtkColorStyle
{
public:
    VtkColorStyle()
    {
        m_bpointValue = true;
    };
	QList<VtkColorPoint> m_colorPoint;
	QString m_styleName;
	bool m_colorOpacity, m_colorAdd, m_lightShade;
	double m_Ambient, m_Diffuse, m_Specular, m_SpecularPower;

    //++++++
    QList<QPointF>m_gradientPoinst;
    bool m_bpointValue;
    int m_slope;
    //++++++
	void clearAll()
	{
		m_colorPoint.clear();
        m_gradientPoinst.clear();
	}
};

class QEditorByValues : public QWidget, private ::Ui::QEditorByValuesBase {
Q_OBJECT

public:

	QEditorByValues(QWidget *parent = 0);
    virtual ~QEditorByValues();

    void setMinimum(int minimum) ;

    void setMaximum(int maximum) ;

     void setValues(VtkColorStyle *vtkColors);

	 void getValues() const;

	 const VtkColorStyle *getVtkColorStyle();

public slots:

    void addInterval();

    void removeInterval();

	void setValues3D();

	void saveValues3D();

	void loadFileValues();

signals:
	void signalsColorValue(VtkColorStyle colorValue);

private:

	QIntervalEditor* addIntervalAndReturnIt();

private slots:

    void markAsChanged();

private:

    QWidget *m_intervalEditorsWidget;

    QVBoxLayout *m_intervalEditorsLayout;


    unsigned int m_numberOfIntervals;

    mutable bool m_changed;

	int m_minimum;

	int m_maximum;

	mutable VtkColorStyle m_vtkColorStyle;

};

#endif
