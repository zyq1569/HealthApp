#ifndef UDGQCOLORSPINBOX_H
#define UDGQCOLORSPINBOX_H

#include <QSpinBox>

class QColorSpinBox : public QSpinBox
{
Q_OBJECT

public:
    QColorSpinBox(QWidget *parent = 0);
    virtual ~QColorSpinBox();

    const QColor& getColor() const;

public slots:

    void setColor(const QColor &color);

signals:

    void colorChanged(const QColor &color);

private slots:

    void setOpacity(int opacity);

private:

    QColor m_color;

    bool m_settingColor;

};

#endif
