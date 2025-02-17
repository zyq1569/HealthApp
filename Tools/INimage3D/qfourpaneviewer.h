#ifndef QFOURPANEVIEWER_H
#define QFOURPANEVIEWER_H

#include <QWidget>

namespace Ui {
class QFourpaneviewer;
}

class QFourpaneviewer : public QWidget
{
    Q_OBJECT

public:
    explicit QFourpaneviewer(QWidget *parent = nullptr);
    ~QFourpaneviewer();

private:
    Ui::QFourpaneviewer *ui;
};

#endif // QFOURPANEVIEWER_H
