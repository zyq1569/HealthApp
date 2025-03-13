

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
	QList<VtkColorPoint> m_colorPoint;
	QString m_styleName;
	bool m_colorOpacity,m_colorAdd;
	void clearAll()
	{
		m_colorPoint.clear();
	}
};

class QEditorByValues : public QWidget, private ::Ui::QEditorByValuesBase {
Q_OBJECT

public:
    /// Crea l'editor amb un sol interval de 0 a 255 amb RGBA = (0,0,0,0).
	QEditorByValues(QWidget *parent = 0);
    virtual ~QEditorByValues();

    /// Sets the minimum input value mapped by this editor.
    void setMinimum(int minimum) ;
    /// Sets the minimum input value mapped by this editor.
    void setMaximum(int maximum) ;

    /// Assigna una funció de transferència a l'editor.
     void setValues(VtkColorStyle *vtkColors);
    /// Retorna la funció de transferència de l'editor.
	 void getValues() const;

	 const VtkColorStyle *getVtkColorStyle();

public slots:
    /// Afegeix un interval a l'editor (al final). Si ja n'hi ha el màxim no fa res.
    void addInterval();
    /// Esborra un interval de l'editor (l'últim). Si n'hi ha només 1 no fa res.
    void removeInterval();

	void setValues3D();

	void saveValues3D();

	void loadFileValues();

signals:
	void signalsColorValue(VtkColorStyle colorValue);

private:
    /// Afegeix un interval al final i el retorna. Si ja n'hi ha el màxim no fa res.
	QIntervalEditor* addIntervalAndReturnIt();

private slots:
    /// Posa m_changed a cert.
    void markAsChanged();

private:
    /// Widget on es situaran els editors d'interval, dins d'una àrea d'scroll.
    QWidget *m_intervalEditorsWidget;
    /// El layout del widget anterior.
    QVBoxLayout *m_intervalEditorsLayout;

    /// Nombre d'intervals de l'editor.
    unsigned int m_numberOfIntervals;

    /// Memoria cau de la funció de transferència representada actualment per l'editor (si m_changed
    /// és fals).
    //mutable TransferFunction m_transferFunction;

    /// Indica si hi ha hagut canvis des de l'últim cop que s'ha actualitzat la memòria cau de la
    /// funció de transferència.
    mutable bool m_changed;

	/// Minimum mapped input value.
	int m_minimum;
	/// Maximum mapped input value.
	int m_maximum;

	mutable VtkColorStyle m_vtkColorStyle;

};

#endif
