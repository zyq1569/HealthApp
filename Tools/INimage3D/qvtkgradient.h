#ifndef QvtkGradient_H
#define QvtkGradient_H

#include <QLineEdit>

class ShadeWidget;

class QvtkGradient : public QWidget {
Q_OBJECT

public:
    QvtkGradient(QWidget *parent = 0);
    virtual ~QvtkGradient();


    virtual void setTransferFunction(/*const TransferFunction &transferFunction*/);
    /// Retorna la funció de transferència de l'editor.

    void setGradientStops(const QGradientStops &stops);

public slots:
    void pointsUpdated();

signals:
    void gradientStopsChanged(const QGradientStops &stops);

private slots:
    void setTransferFunction(const QGradientStops &stops);
    void setTransferFunctionName(const QString &name);

private:
    QLineEdit *m_nameLineEdit;
	ShadeWidget *m_gradientShade, *m_colorShade;

    /// Funció de transferència actual.
    //TransferFunction m_transferFunction;

};

#endif
