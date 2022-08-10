#include "mainapp.h"
#include "logging.h"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP


#include <QApplication>
#include <QFile>


#define LOADQSS(qssFile)                      \
{                                             \
    QString strQss;                           \
    QFile file(qssFile);                      \
    file.open(QFile::ReadOnly);               \
    if(file.isOpen())                         \
{                                         \
    strQss=QLatin1String(file.readAll()); \
    qApp->setStyleSheet(strQss);          \
    file.close();                         \
    }                                         \
    }




//static void *pull_one_url(void *url)
//{
////    CURL *curl;

////    curl = curl_easy_init();
////    curl_easy_setopt(curl, CURLOPT_URL, url);
////    curl_easy_perform(curl); /* ignores error */
////    curl_easy_cleanup(curl);

//    return NULL;
//}

//提取运行包文件
//F:\temp\HealthApp\Tools\MHealthReport\ms_bin\release>C:\Qt\Qt5.14.1\5.14.1\msvc2017_64\bin\windeployqt.exe MHealthReport.exe
///
///need  office:
///
/// to do: https://curl.se/windows/
///        https://curl.se/download.html
/// 使用浏览器页面功能.暂时需要QWebEngineView. to do.... curl???----- 20220805
///
/// 当前使用的线程 共享内存方式 to do....共享内存直接 vc(rel版本) 和 mingw 编译 失败
/// 线程方式也有替换 to do..   moveToThread
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainApp MainWindow;

    udg::beginLogging();
    MainWindow.setWindowFlags(MainWindow.windowFlags()&  ~Qt::WindowMinimizeButtonHint);
    //MainWindow.show();///~Qt::WindowMaximizeButtonHint&
    MainWindow.showMaximized();// Max size show
    //MainWindow.showFullScreen();

    LOADQSS(":/qss/qss/style.qss");
    //a.setFont(QFont("ZYSong", 10));

    return a.exec();
}


//Organizationally Derived UID
//A UID may be formed using a registered root (see Annex C) and an organization specific suffix.
//The manner in which the suffix of such an organizationally derived UID is defined is not constrained by the DICOM Standard.
//Only the guarantee of its uniqueness by the defining organization is required by DICOM.

//The following example presents a particular choice made by a specific organization in defining its suffix to guarantee uniqueness of a SOP Instance UID.

//  "1.2.840.xxxxx.3.152.235.2.12.187636473"
//   \___________/ \______________________/
//       root     .         suffix
//In this example, the root is:

//1 Identifies ISO

//2 Identifies ANSI Member Body

//840 Country code of a specific Member Body (U.S. for ANSI)

//xxxxx Identifies a specific Organization.(assigned by ANSI)

//In this example the first two components of the suffix relate to the identification of the device:

//3 Manufacturer defined device type

//152 Manufacturer defined serial number

//The remaining four components of the suffix relate to the identification of the image:

//235 Study number

//2 Series number

//12 Image number

//187636473 Encoded date and time stamp of image acquisition

//In this example, the organization has chosen these components to guarantee uniqueness. Other organizations may choose an entirely different
//                 series of components to uniquely identify its images. For example it may have been perfectly valid to omit the Study Number,
//                 Series Number and Image Number if the time stamp had a sufficient precision to ensure that no two images might have the same date and time stamp.

//Because of the flexibility allowed by the DICOM Standard in creating organizationally derived UIDs, implementations should not
//                 depend on any assumed structure of UIDs and should not attempt to parse UIDs to extract the semantics of some of its components.

