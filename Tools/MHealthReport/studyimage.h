#ifndef STUDYIMAGE_H
#define STUDYIMAGE_H

#include <QMainWindow>

namespace Ui {
class StudyImage;
}

class StudyImage : public QMainWindow
{
    Q_OBJECT

public:
    explicit StudyImage(QWidget *parent = nullptr);
    ~StudyImage();

private:
    Ui::StudyImage *ui;
};

#endif // STUDYIMAGE_H
