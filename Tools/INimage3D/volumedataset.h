#ifndef VOLUMEDATASET_H
#define VOLUMEDATASET_H

#include <QWidget>

namespace Ui {
class VolumeDataSet;
}

class VolumeDataSet : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeDataSet(QWidget *parent = nullptr);
    ~VolumeDataSet();

signals:
    void SplitImageData(int *dims, int start, int end);

    void RectData(int orientation, int dx, int dy, int w, int h);

public slots:
    void SaveSplitParm();
    void SaveRectParm();

public:
    void SetSlicesNumber(int *dim);

public:
    int *m_dims, m_slicesNumber, m_topStart, m_topEnd, m_centerStart, m_centerEnd, m_bottomStart, m_bottomEnd;
    int m_showIndex;

private:
    Ui::VolumeDataSet *ui;
};

#endif // VOLUMEDATASET_H
