#ifndef QPROGRESSDATA_H
#define QPROGRESSDATA_H

#include <QWidget>

namespace Ui {
class QProgressData;
}

class QProgressData : public QWidget
{
    Q_OBJECT

public:
    explicit QProgressData(QWidget *parent = nullptr);
    ~QProgressData();

private:
    Ui::QProgressData *ui;
};

#endif // QPROGRESSDATA_H
