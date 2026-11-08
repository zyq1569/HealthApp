#ifndef VOLUMEPARAMS_H
#define VOLUMEPARAMS_H

#include <QWidget>

namespace Ui {
    class VolumeParams;
}

class VolumeParams : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeParams(QWidget *parent = nullptr);
    ~VolumeParams();

public slots:
    void SlotSelectRow();

private:
    Ui::VolumeParams *ui;
};

#endif // VOLUMEPARAMS_H
