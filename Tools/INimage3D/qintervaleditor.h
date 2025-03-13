#ifndef UINTERVALEDITOR_H
#define UINTERVALEDITOR_H

#include "ui_editorbase.h"

class QIntervalEditor : public QWidget, public Ui::QEditorBase {
Q_OBJECT

public:

	QIntervalEditor(QWidget *parent = 0);

	QIntervalEditor(int maximum, QWidget *parent = 0);
    virtual ~QIntervalEditor();


    int minimum() const;

    void setMinimum(int minimum);


    int maximum() const;

    void setMaximum(int maximum);


    void setIsFirst(bool isFirst);

    bool isFirst() const;

    void setIsLast(bool isLast);

    bool isLast() const;

    bool isInterval() const;

    void setStart(int start);

    int start() const;

    void setEnd(int end);

    int end() const;

    void setColor(QColor color);

    const QColor& color() const;

public slots:

    void setIsInterval(bool isInterval);

    void setPreviousEnd(int previousEnd);

    void setNextStart(int nextStart);

signals:

    void startChanged(int start);

    void endChanged(int end);

    void colorChanged(const QColor &color);

private:

    void firstAndLast();

private slots:

    void isIntervalToggled(bool checked);

    void adjustWithNewStart(int start);

    void adjustWithNewEnd(int end);

    void selectColor();

private:
    /// Minimum of the interval.
    int m_minimum;
    /// Maximum of the interval.
    int m_maximum;
    /// Propietats isFirst i isLast.
    bool m_isFirst, m_isLast;

};

#endif
