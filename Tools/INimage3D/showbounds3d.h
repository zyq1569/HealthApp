#ifndef SHOWBOUNDS3D_H
#define SHOWBOUNDS3D_H

#include <QWidget>

namespace Ui {
class ShowBounds3D;
}

using Bounds3D = std::array<int, 6>;
class vtkImageData;
class vtkRenderer;
class ShowBounds3D : public QWidget
{
    Q_OBJECT

public:
    explicit ShowBounds3D(QWidget *parent = nullptr);
    ~ShowBounds3D();


public:
    void ReadVolumes(vtkImageData *imageData);
    void ShowSegmentedVolumes(vtkImageData* imageData, const std::vector<Bounds3D>& partitions, vtkRenderer* renderer);

private:
    Ui::ShowBounds3D *ui;
};

#endif // SHOWBOUNDS3D_H
