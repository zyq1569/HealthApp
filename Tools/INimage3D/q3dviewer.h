#ifndef Q3DVIEWER_H
#define Q3DVIEWER_H

#include <QWidget>

namespace Ui {
class Q3dviewer;
}

class Q3dviewer : public QWidget
{
    Q_OBJECT

public:
    explicit Q3dviewer(QWidget *parent = nullptr);
    ~Q3dviewer();

private:
    Ui::Q3dviewer *ui;
};

#endif // Q3DVIEWER_H
