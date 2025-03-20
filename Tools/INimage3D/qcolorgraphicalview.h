
#ifndef QColorGraphicalView_H
#define QColorGraphicalView_H

#include "ui_QGraphicalViewbase.h"

class QColorGraphicalView : public QGraphicsView, private Ui::QGraphicalViewBase {
Q_OBJECT

public:

    QColorGraphicalView(QWidget *parent = 0);

protected:
    virtual void makeConnections();

protected slots:
    virtual void setTransferFunction(/*const TransferFunction &transferFunction*/);

    virtual void setName(const QString &name);

    virtual void setColorTransferFunction(/*const ColorTransferFunction &colorTransferFunction*/);

    virtual void setScalarOpacityTransferFunction(/*const OpacityTransferFunction &scalarOpacityTransferFunction*/);

    virtual void addColorPoint(double x, const QColor &color);

    virtual void removeColorPoint(double x);

    virtual void addScalarOpacityPoint(double x, double opacity);

    virtual void removeScalarOpacityPoint(double x);

private:
    Q_DISABLE_COPY(QColorGraphicalView)
    void enableEditingConnections();
    void disableEditingConnections();

private:
    bool m_editingConnectionsEnabled;

};



#endif // QColorGraphicalView_H
