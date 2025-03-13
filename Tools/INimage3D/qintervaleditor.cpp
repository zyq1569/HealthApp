#include "qintervaleditor.h"

#include <QColorDialog>

QIntervalEditor::QIntervalEditor(QWidget *parent)
 : QWidget(parent), m_minimum(-2000), m_maximum(2000)
{
    setupUi(this);

    m_isFirst = m_isLast = false;

    m_intervalEndSpinBox->setVisible(m_isIntervalCheckBox->isChecked());

    connect(m_isIntervalCheckBox, SIGNAL(toggled(bool)), SLOT(isIntervalToggled(bool)));
    connect(m_intervalStartSpinBox, SIGNAL(valueChanged(int)), SLOT(adjustWithNewStart(int)));
    connect(m_intervalEndSpinBox, SIGNAL(valueChanged(int)), SLOT(adjustWithNewEnd(int)));
    connect(m_intervalStartSpinBox, SIGNAL(valueChanged(int)), SIGNAL(startChanged(int)));
    connect(m_intervalEndSpinBox, SIGNAL(valueChanged(int)), SIGNAL(endChanged(int)));
    connect(m_selectColorPushButton, SIGNAL(clicked()), SLOT(selectColor()));
    connect(m_colorSpinBox, SIGNAL(colorChanged(const QColor&)), SIGNAL(colorChanged(const QColor&)));
}

QIntervalEditor::QIntervalEditor(int maximum, QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    m_maximum = maximum;
    m_isFirst = m_isLast = false;

    m_intervalStartSpinBox->setMaximum(m_maximum);
    m_intervalEndSpinBox->setMaximum(m_maximum);

    m_intervalEndSpinBox->setVisible(m_isIntervalCheckBox->isChecked());

    connect(m_isIntervalCheckBox, SIGNAL(toggled(bool)), SLOT(isIntervalToggled(bool)));
    connect(m_intervalStartSpinBox, SIGNAL(valueChanged(int)), SLOT(adjustWithNewStart(int)));
    connect(m_intervalEndSpinBox, SIGNAL(valueChanged(int)), SLOT(adjustWithNewEnd(int)));
    connect(m_intervalStartSpinBox, SIGNAL(valueChanged(int)), SIGNAL(startChanged(int)));
    connect(m_intervalEndSpinBox, SIGNAL(valueChanged(int)), SIGNAL(endChanged(int)));
    connect(m_selectColorPushButton, SIGNAL(clicked()), SLOT(selectColor()));
    connect(m_colorSpinBox, SIGNAL(colorChanged(const QColor&)), SIGNAL(colorChanged(const QColor&)));
}

QIntervalEditor::~QIntervalEditor()
{
}

int QIntervalEditor::minimum() const
{
    return m_minimum;
}

void QIntervalEditor::setMinimum(int minimum)
{
    m_minimum = minimum;
    m_intervalStartSpinBox->setMinimum(m_minimum);
    m_intervalEndSpinBox->setMinimum(m_minimum);

    if (m_isFirst)
    {
        this->setStart(m_minimum);
    }
}

int QIntervalEditor::maximum() const
{
    return m_maximum;
}

void QIntervalEditor::setMaximum(int maximum)
{
    m_maximum = maximum;
    m_intervalStartSpinBox->setMaximum(m_maximum);
    m_intervalEndSpinBox->setMaximum(m_maximum);

    if (m_isLast)
    {
        if (this->isInterval())
        {
            this->setEnd(m_maximum);
        }
        else
        {
            this->setStart(m_maximum);
        }
    }
}

void QIntervalEditor::setIsFirst(bool isFirst)
{
    m_isFirst = isFirst;
    if (m_isFirst && m_isLast)
    {
        firstAndLast();
    }
    else
    {
        m_isIntervalCheckBox->setEnabled(true);
        if (m_isFirst)
        {
            this->setStart(m_minimum);
        }
        m_intervalStartSpinBox->setReadOnly(m_isFirst);
    }
}

bool QIntervalEditor::isFirst() const
{
    return m_isFirst;
}

void QIntervalEditor::setIsLast(bool isLast)
{
    m_isLast = isLast;
    if (m_isFirst && m_isLast)
    {
        firstAndLast();
    }
    else
    {
        m_isIntervalCheckBox->setEnabled(true);
        if (m_isLast)
        {
            this->setEnd(m_maximum);
        }
        m_intervalEndSpinBox->setReadOnly(m_isLast);
        if (!m_isIntervalCheckBox->isChecked())
        {
            if (m_isLast)
            {
                this->setStart(m_maximum);
            }
            m_intervalStartSpinBox->setReadOnly(m_isLast);
        }
    }
}

bool QIntervalEditor::isLast() const
{
    return m_isLast;
}

bool QIntervalEditor::isInterval() const
{
    return m_isIntervalCheckBox->isChecked();
}

void QIntervalEditor::setStart(int start)
{
    m_intervalStartSpinBox->setValue(start);
    emit startChanged(this->start());
}

int QIntervalEditor::start() const
{
    return m_intervalStartSpinBox->value();
}

void QIntervalEditor::setEnd(int end)
{
    m_intervalEndSpinBox->setValue(end);
    emit endChanged(this->end());
}

int QIntervalEditor::end() const
{
    return m_intervalEndSpinBox->value();
}

void QIntervalEditor::setColor(QColor color)
{
    m_colorSpinBox->setColor(color);
}

const QColor& QIntervalEditor::color() const
{
    return m_colorSpinBox->getColor();
}

void QIntervalEditor::setIsInterval(bool isInterval)
{
    m_isIntervalCheckBox->setChecked(isInterval);
}

void QIntervalEditor::setPreviousEnd(int previousEnd)
{
    if (previousEnd >= this->start())
    {
        this->setStart(previousEnd + 1);
    }
}

void QIntervalEditor::setNextStart(int nextStart)
{
    if (nextStart <= this->end())
    {
        this->setEnd(nextStart - 1);
    }
}

void QIntervalEditor::firstAndLast()
{
    m_isIntervalCheckBox->setChecked(true);
    m_isIntervalCheckBox->setDisabled(true);
    this->setStart(m_minimum);
    m_intervalStartSpinBox->setReadOnly(true);
    this->setEnd(m_maximum);
    m_intervalEndSpinBox->setReadOnly(true);
}

void QIntervalEditor::isIntervalToggled(bool checked)
{
    if (!checked)
    {
        this->setEnd(this->start());
    }
    if (m_isLast)
    {
        if (!checked)
        {
            this->setStart(m_maximum);
        }
        m_intervalStartSpinBox->setReadOnly(!checked);
    }
}

void QIntervalEditor::adjustWithNewStart(int start)
{
    if (!m_isIntervalCheckBox->isChecked() || start > this->end())
    {
        this->setEnd(start);
    }
}

void QIntervalEditor::adjustWithNewEnd(int end)
{
    if (end < this->start())
    {
        this->setStart(end);
    }
}

void QIntervalEditor::selectColor()
{
    QColor color = this->color();
    bool ok;
    color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &ok, this));
    if (ok)
    {
        this->setColor(color);
    }
}