#ifndef VOLUMEPARAMS_H
#define VOLUMEPARAMS_H

#include <QWidget>

using Bounds3D = std::array<int, 6>;
class ShowBounds3D;
namespace Ui {
    class VolumeParams;
}

class VolumeParams : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeParams(QWidget *parent = nullptr);
    ~VolumeParams();
public:
    void UpdateUI(ShowBounds3D *bounds3D, const std::vector<Bounds3D>&result);

public:
    ShowBounds3D *m_Bound3D;

public slots:
    void SlotSelectRow();

private:
    Ui::VolumeParams *ui;
};

#endif // VOLUMEPARAMS_H
