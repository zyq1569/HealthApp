#ifndef UDGQCOLORSPINBOX_H
#define UDGQCOLORSPINBOX_H

#include <QSpinBox>

class QColorSpinBox : public QSpinBox {
Q_OBJECT

public:
    /// Crea un QColorSpinBox blanc i amb alfa 0.
    QColorSpinBox(QWidget *parent = 0);
    virtual ~QColorSpinBox();

    /// Retorna el color RGBA associat a l'spinbox.
    const QColor& getColor() const;

public slots:
    /// Assigna un color RGBA a l'spinbox.
    void setColor(const QColor &color);

signals:
    /// S'emet quan canvia el color RGBA associat a l'spinbox.
    void colorChanged(const QColor &color);

private slots:
    /// Assigna una opacitat a l'spinbox.
    void setOpacity(int opacity);

private:
    /// Color RGBA associat a l'spinbox.
    QColor m_color;
    /// Booleà de control per evitar d'emetre dues vegades el signal colorChanged.
    bool m_settingColor;

};

#endif
