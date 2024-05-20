QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG  += c++11
DEFINES += CHARLS_STATIC MDCM_WIN32_UNC

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
                main.cpp \
                dicommetadata.cpp \
                sqtree.cpp

HEADERS += \
                dicommetadata.h \
                sqtree.h

FORMS += \
                dicommetadata.ui \
                sqtree.ui
INCLUDEPATH += ./ \
                Source/mdcm/DataDictionary \
                Source/mdcm/Common \
                Source/mdcm/DataStructureAndEncodingDefinition \
                Source/mdcm/MediaStorageAndFileFormat \
                Source/Common \
                Source  \
                Source/dicom \
                Source/mdcm/Utilities/ \
                Source/mdcm/Utilities

INCLUDEPATH += \
                D:\SDK\InsightToolkit-5.0.1\Modules\Core\Common\include \
                D:\SDK\InsightToolkit-5.0.1_VS2017\Modules\Core\Common \
                D:\SDK\InsightToolkit-5.0.1\Modules\ThirdParty\VNL\src\vxl\core \
                D:\SDK\InsightToolkit-5.0.1_VS2017\Modules\ThirdParty\VNL\src\vxl\core \
                D:\SDK\InsightToolkit-5.0.1_VS2017\Modules\ThirdParty\VNL\src\vxl\vcl \
                D:\SDK\InsightToolkit-5.0.1\Modules\ThirdParty\VNL\src\vxl\vcl \
                D:\SDK\InsightToolkit-5.0.1\Modules\Core\ImageFunction\include \
                D:\SDK\InsightToolkit-5.0.1\Modules\Filtering\ImageIntensity\include \
                D:\SDK\InsightToolkit-5.0.1\Modules\Numerics\Statistics\include \
                D:\SDK\InsightToolkit-5.0.1\Modules\Filtering\ImageGrid\include \
                D:\SDK\InsightToolkit-5.0.1\Modules\Core\Transform\include \
                D:\SDK\InsightToolkit-5.0.1_VS2017\Modules\ThirdParty\Eigen3\src \
                D:\SDK\InsightToolkit-5.0.1\Modules\ThirdParty\Eigen3\src
LIBS   += \
            Source\lib\mdcmjpeg8.lib \
            Source\lib\mdcmjpeg12.lib \
            Source\lib\mdcmjpeg16.lib \
            Source\lib\mdcmopenjp2.lib \
            Source\lib\mdcmcharls.lib \
            Source\lib\mdcmzlib.lib \
            Source\lib\alizalcms2.lib \
            Source\lib\alizams.lib

LIBS   += \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKCommon-5.0.lib                       \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKMetaIO-5.0.lib                       \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKNrrdIO-5.0.lib                       \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKSpatialObjects-5.0.lib               \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKStatistics-5.0.lib                   \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\itkjpeg-5.0.lib                         \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\itkvnl_algo-5.0.lib                     \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\itkvnl-5.0.lib                          \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\itkvcl-5.0.lib                          \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\itkv3p_netlib-5.0.lib                   \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKVNLInstantiation-5.0.lib             \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKniftiio-5.0.lib                      \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKznz-5.0.lib                          \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKOptimizers-5.0.lib                   \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKVTK-5.0.lib                          \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKIOGDCM-5.0.lib                       \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKIOJPEG-5.0.lib                       \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKIOImageBase-5.0.lib                  \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\itksys-5.0.lib                          \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\itkdouble-conversion-5.0.lib            \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKTransform-5.0.lib                    \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\itkzlib-5.0.lib                         \
            D:\SDK\starviewer-win64-5.0.1-8.2.0-3.0.0\InsightToolkit\5.0.1\lib\ITKEXPAT-5.0.lib

LIBS   += \
            uuid.lib             \
            crypt32.lib          \
            rpcrt4.lib           \
            kernel32.lib         \
            user32.lib           \
            gdi32.lib            \
            winspool.lib         \
            shell32.lib          \
            ole32.lib            \
            oleaut32.lib         \
            uuid.lib             \
            comdlg32.lib         \
            advapi32.lib



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    DicomMetadata.qrc
