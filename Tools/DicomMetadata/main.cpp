#include "dicommetadata.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DicomMetadata w;
    w.show();
    return a.exec();
}
