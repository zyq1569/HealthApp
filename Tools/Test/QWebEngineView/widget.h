#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_buttonBrowse_clicked();

    void on_buttonPre_clicked();

    void on_buttonNext_clicked();

    void on_buttonInspect_clicked();

    void slotLoadFinished();

    void on_buttonZoom_clicked();

    void on_buttonDefault_clicked();

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H
