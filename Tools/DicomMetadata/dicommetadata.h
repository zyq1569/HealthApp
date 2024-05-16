#ifndef DICOMMETADATA_H
#define DICOMMETADATA_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class DicomMetadata; }
QT_END_NAMESPACE

class DicomMetadata : public QMainWindow
{
    Q_OBJECT

public:
    DicomMetadata(QWidget *parent = nullptr);
    ~DicomMetadata();

private:
    Ui::DicomMetadata *ui;
};
#endif // DICOMMETADATA_H
