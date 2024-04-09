QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SDK_INSTALL_PREFIX = D:\SDK\starviewer-sdk-win64-0.15
isEmpty(SDK_INSTALL_PREFIX){
    unix:SDK_INSTALL_PREFIX = $$(HOME)/starviewer-sdk-0.15/usr/local
    win32:SDK_INSTALL_PREFIX = $$(USERPROFILE)/starviewer-sdk-0.15/32
    win32:contains(QMAKE_TARGET.arch, x86_64):SDK_INSTALL_PREFIX = $$(USERPROFILE)/starviewer-sdk-0.15/64
}

# DCMTK Libraries

DCMTKLIBDIR = $$(DCMTKLIBDIR)
isEmpty(DCMTKLIBDIR){
    unix:DCMTKLIBDIR = $$SDK_INSTALL_PREFIX/lib
     win32:DCMTKLIBDIR = $$SDK_INSTALL_PREFIX/dcmtk/3.6.5/lib
}
DCMTKINCLUDEDIR = $$(DCMTKINCLUDEDIR)
isEmpty(DCMTKINCLUDEDIR){
    unix:DCMTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/include/dcmtk
     win32:DCMTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/dcmtk/3.6.5/include/dcmtk
}


# VTK Libraries

VTKLIBDIR = $$(VTKLIBDIR)
isEmpty(VTKLIBDIR){
    unix:VTKLIBDIR = $$SDK_INSTALL_PREFIX/lib
    win32:VTKLIBDIR = $$SDK_INSTALL_PREFIX/VTK/8.2.0/libshare
}
VTKINCLUDEDIR = $$(VTKINCLUDEDIR)
isEmpty(VTKINCLUDEDIR){
    unix:VTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/include/vtk-8.1
    win32:VTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/VTK/8.2.0/include
}


# ITK Libraries

ITKLIBDIR = $$(ITKLIBDIR)
isEmpty(ITKLIBDIR){
    unix:ITKLIBDIR = $$SDK_INSTALL_PREFIX/lib
#    win32:ITKLIBDIR = $$SDK_INSTALL_PREFIX/InsightToolkit/4.13.0/lib
    win32:ITKLIBDIR = $$SDK_INSTALL_PREFIX/InsightToolkit/5.0.1/lib820
}
ITKINCLUDEDIR = $$(ITKINCLUDEDIR)
isEmpty(ITKINCLUDEDIR){
    unix:ITKINCLUDEDIR = $$SDK_INSTALL_PREFIX/include/ITK-4.13
#    win32:ITKINCLUDEDIR = $$SDK_INSTALL_PREFIX/InsightToolkit/4.13.0/include/ITK-4.13
    win32:ITKINCLUDEDIR = $$SDK_INSTALL_PREFIX/InsightToolkit/5.0.1/include
}

# GDCM Libraries
#2.8.6
#GDCMLIBDIR = $$(GDCMLIBDIR)
#isEmpty(GDCMLIBDIR){
#    unix:GDCMLIBDIR = $$SDK_INSTALL_PREFIX/lib
#    win32:GDCMLIBDIR = $$SDK_INSTALL_PREFIX/gdcm/2.8.6/lib
#}
#GDCMINCLUDEDIR = $$(GDCMINCLUDEDIR)
#isEmpty(GDCMINCLUDEDIR){
#    unix:GDCMINCLUDEDIR = $$SDK_INSTALL_PREFIX/include/gdcm-2.8
#    win32:GDCMINCLUDEDIR = $$SDK_INSTALL_PREFIX/gdcm/2.8.6/include/gdcm-2.8
#}
#3.0.0
GDCMLIBDIR = $$(GDCMLIBDIR)
isEmpty(GDCMLIBDIR){
    unix:GDCMLIBDIR = $$SDK_INSTALL_PREFIX/lib
    win32:GDCMLIBDIR = $$SDK_INSTALL_PREFIX/gdcm/3.0.0/lib
}
GDCMINCLUDEDIR = $$(GDCMINCLUDEDIR)
isEmpty(GDCMINCLUDEDIR){
    unix:GDCMINCLUDEDIR = $$SDK_INSTALL_PREFIX/include/gdcm-3.0
    win32:GDCMINCLUDEDIR = $$SDK_INSTALL_PREFIX/gdcm/3.0.0/include/gdcm-3.0
}
win32:LIBS += -lShell32
unix:LIBS += -ldl

#//----------------------------------------------------lib include
#ITK 5.0.0
INCLUDEPATH += $${ITKINCLUDEDIR}

#ITKLIBSUFFIX = -4.13
ITKLIBSUFFIX = -5.0
ITKLIBS = \
        ITKCommon \
        ITKMetaIO \
        ITKNrrdIO \
        ITKSpatialObjects \
        ITKStatistics \
        itkjpeg \
        itksys \
        itkvnl_algo \
        itkvnl \
        itkvcl \
        itkv3p_netlib \
        ITKVNLInstantiation \
        ITKniftiio \
        ITKznz \
        ITKOptimizers \
        ITKVTK \
        ITKIOGDCM \
        ITKIOJPEG \
        ITKIOImageBase \
        itksys \
        itkdouble-conversion \
        ITKTransform

macx:official_release:ITKLIBS += itkzlib

LIBS += -L$${ITKLIBDIR}

for(lib, ITKLIBS) {
    LIBS += -l$${lib}$${ITKLIBSUFFIX}
}

win32:LIBS += -lShell32
unix:LIBS += -ldl


#------------------------------------------------------------VTK8.2.0-------------------------------------
INCLUDEPATH +=  $${VTKINCLUDEDIR} \
                $${VTKINCLUDEDIR}/more/ \
                $${VTKINCLUDEDIR}/Common/Core/ \
                $${VTKINCLUDEDIR}/Common/DataModel/ \
                $${VTKINCLUDEDIR}/Common/Math/ \
                $${VTKINCLUDEDIR}/Common/ComputationalGeometry/ \
                $${VTKINCLUDEDIR}/Common/ExecutionModel/ \
                $${VTKINCLUDEDIR}/Common/Misc/ \
                $${VTKINCLUDEDIR}/Common/System/ \
                $${VTKINCLUDEDIR}/Common/Transforms/ \
                $${VTKINCLUDEDIR}/Common/Color/ \
                $${VTKINCLUDEDIR}/Imaging/Core/ \
                $${VTKINCLUDEDIR}/Imaging/Color/ \
                $${VTKINCLUDEDIR}/Imaging/Fourier/ \
                $${VTKINCLUDEDIR}/Imaging/General/ \
                $${VTKINCLUDEDIR}/Imaging/Hybrid/ \
                $${VTKINCLUDEDIR}/Imaging/Math/ \
                $${VTKINCLUDEDIR}/Imaging/Morphological/ \
                $${VTKINCLUDEDIR}/Imaging/OpenGL2/ \
                $${VTKINCLUDEDIR}/Imaging/Sources/ \
                $${VTKINCLUDEDIR}/Imaging/Statistics/ \
                $${VTKINCLUDEDIR}/Imaging/Stencil/ \
                $${VTKINCLUDEDIR}/IO/ADIOS/ \
                $${VTKINCLUDEDIR}/IO/Core/ \
                $${VTKINCLUDEDIR}/IO/EnSight/ \
                $${VTKINCLUDEDIR}/IO/Exodus/ \
                $${VTKINCLUDEDIR}/IO/Export/ \
                $${VTKINCLUDEDIR}/IO/ExportOpenGL/ \
                $${VTKINCLUDEDIR}/IO/ExportOpenGL2/ \
                $${VTKINCLUDEDIR}/IO/FFMPEG/ \
                $${VTKINCLUDEDIR}/IO/GDAL/ \
                $${VTKINCLUDEDIR}/IO/GeoJSON/ \
                $${VTKINCLUDEDIR}/IO/Geometry/ \
                $${VTKINCLUDEDIR}/IO/Image/ \
                $${VTKINCLUDEDIR}/IO/Import/ \
                $${VTKINCLUDEDIR}/IO/Infovis/ \
                $${VTKINCLUDEDIR}/IO/LAS/ \
                $${VTKINCLUDEDIR}/IO/Legacy/ \
                $${VTKINCLUDEDIR}/IO/LSDyna/ \
                $${VTKINCLUDEDIR}/IO/MINC/ \
                $${VTKINCLUDEDIR}/IO/Movie/ \
                $${VTKINCLUDEDIR}/IO/MPIImage/ \
                $${VTKINCLUDEDIR}/IO/MPIParallel/ \
                $${VTKINCLUDEDIR}/IO/MySQL/ \
                $${VTKINCLUDEDIR}/IO/NetCDF/ \
                $${VTKINCLUDEDIR}/IO/ODBC/ \
                $${VTKINCLUDEDIR}/IO/Parallel/ \
                $${VTKINCLUDEDIR}/IO/ParallelExodus/ \
                $${VTKINCLUDEDIR}/IO/ParallelLSDyna/ \
                $${VTKINCLUDEDIR}/IO/ParallelNetCDF/ \
                $${VTKINCLUDEDIR}/IO/ParallelXdmf3/ \
                $${VTKINCLUDEDIR}/IO/ParallelXML/ \
                $${VTKINCLUDEDIR}/IO/PDAL/ \
                $${VTKINCLUDEDIR}/IO/PLY/ \
                $${VTKINCLUDEDIR}/IO/PostgreSQL/ \
                $${VTKINCLUDEDIR}/IO/SegY/ \
                $${VTKINCLUDEDIR}/IO/TecplotTable/ \
                $${VTKINCLUDEDIR}/IO/Video/ \
                $${VTKINCLUDEDIR}/IO/VPIC/ \
                $${VTKINCLUDEDIR}/IO/Xdmf2/ \
                $${VTKINCLUDEDIR}/IO/Xdmf3/ \
                $${VTKINCLUDEDIR}/IO/XML/ \
                $${VTKINCLUDEDIR}/IO/XMLParser/ \
                $${VTKINCLUDEDIR}/GUISupport/MFC/ \
                $${VTKINCLUDEDIR}/GUISupport/Qt/ \
                $${VTKINCLUDEDIR}/GUISupport/QtOpenGL/ \
                $${VTKINCLUDEDIR}/GUISupport/QtSQL/ \
                $${VTKINCLUDEDIR}/GUISupport/QtWebkit/ \
                $${VTKINCLUDEDIR}/Rendering/Annotation/ \
                $${VTKINCLUDEDIR}/Rendering/Context2D/ \
                $${VTKINCLUDEDIR}/Rendering/ContextOpenGL/ \
                $${VTKINCLUDEDIR}/Rendering/ContextOpenGL2/ \
                $${VTKINCLUDEDIR}/Rendering/Core/ \
                $${VTKINCLUDEDIR}/Rendering/External/ \
                $${VTKINCLUDEDIR}/Rendering/FreeType/ \
                $${VTKINCLUDEDIR}/Rendering/FreeTypeFontConfig/ \
                $${VTKINCLUDEDIR}/Rendering/GL2PS/ \
                $${VTKINCLUDEDIR}/Rendering/GL2PSOpenGL2/ \
                $${VTKINCLUDEDIR}/Rendering/Label/ \
                $${VTKINCLUDEDIR}/Rendering/Image/ \
                $${VTKINCLUDEDIR}/Rendering/Core/ \
                $${VTKINCLUDEDIR}/Rendering/LIC/ \
                $${VTKINCLUDEDIR}/Rendering/LICOpenGL2/ \
                $${VTKINCLUDEDIR}/Rendering/LOD/ \
                $${VTKINCLUDEDIR}/Rendering/Matplotlib/ \
                $${VTKINCLUDEDIR}/Rendering/Oculus/ \
                $${VTKINCLUDEDIR}/Rendering/OpenGL/ \
                $${VTKINCLUDEDIR}/Rendering/OpenGL2/ \
                $${VTKINCLUDEDIR}/Rendering/OpenVR/ \
                $${VTKINCLUDEDIR}/Rendering/External/ \
                $${VTKINCLUDEDIR}/Rendering/OptiX/ \
                $${VTKINCLUDEDIR}/Rendering/OSPRay/ \
                $${VTKINCLUDEDIR}/Rendering/GL2PS/ \
                $${VTKINCLUDEDIR}/Rendering/Parallel/ \
                $${VTKINCLUDEDIR}/Rendering/ParallelLIC/ \
                $${VTKINCLUDEDIR}/Rendering/Qt/ \
                $${VTKINCLUDEDIR}/Rendering/SceneGraph/ \
                $${VTKINCLUDEDIR}/Rendering/Tk/ \
                $${VTKINCLUDEDIR}/Rendering/Volume/ \
                $${VTKINCLUDEDIR}/Rendering/VolumeAMR/ \
                $${VTKINCLUDEDIR}/Rendering/VolumeOpenGL/ \
                $${VTKINCLUDEDIR}/Rendering/VolumeOpenGL2/ \
                $${VTKINCLUDEDIR}/Filters/Core/ \
                $${VTKINCLUDEDIR}/Filters/Extraction/ \
                $${VTKINCLUDEDIR}/Filters/FlowPaths/ \
                $${VTKINCLUDEDIR}/Filters/General/ \
                $${VTKINCLUDEDIR}/Filters/Generic/ \
                $${VTKINCLUDEDIR}/Filters/Geometry/ \
                $${VTKINCLUDEDIR}/Filters/Hybrid/ \
                $${VTKINCLUDEDIR}/Filters/HyperTree/ \
                $${VTKINCLUDEDIR}/Filters/Imaging/ \
                $${VTKINCLUDEDIR}/Filters/Modeling/ \
                $${VTKINCLUDEDIR}/Filters/OpenTurns/ \
                $${VTKINCLUDEDIR}/Filters/Parallel/ \
                $${VTKINCLUDEDIR}/Filters/ParallelDIY2/ \
                $${VTKINCLUDEDIR}/Filters/ParallelFlowPaths/ \
                $${VTKINCLUDEDIR}/Filters/ParallelGeometry/ \
                $${VTKINCLUDEDIR}/Filters/ParallelImaging/ \
                $${VTKINCLUDEDIR}/Filters/ParallelMPI/ \
                $${VTKINCLUDEDIR}/Filters/ParallelStatistics/ \
                $${VTKINCLUDEDIR}/Filters/ParallelVerdict/ \
                $${VTKINCLUDEDIR}/Filters/Points/ \
                $${VTKINCLUDEDIR}/Filters/Programmable/ \
                $${VTKINCLUDEDIR}/Filters/Python/ \
                $${VTKINCLUDEDIR}/Filters/ReebGraph/ \
                $${VTKINCLUDEDIR}/Filters/Selection/ \
                $${VTKINCLUDEDIR}/Filters/SMP/ \
                $${VTKINCLUDEDIR}/Filters/Sources/ \
                $${VTKINCLUDEDIR}/Filters/Statistics/ \
                $${VTKINCLUDEDIR}/Filters/Qt/ \
                $${VTKINCLUDEDIR}/Filters/SceneGraph/ \
                $${VTKINCLUDEDIR}/Filters/Texture/ \
                $${VTKINCLUDEDIR}/Utilities/KWIML/ \
                $${VTKINCLUDEDIR}/Utilities/KWIML/vtkkwiml/ \
                $${VTKINCLUDEDIR}/Interaction/Widgets/ \
                $${VTKINCLUDEDIR}/Interaction/Style/ \
                $${VTKINCLUDEDIR}/Interaction/Image/ \
                $${VTKINCLUDEDIR}/Filters/Verdict

VTKLIBSUFFIX = -8.2
VTKLIBS = \
    vtkCommonCore \
    vtkCommonDataModel \
    vtkCommonExecutionModel \
    vtkCommonMath \
    vtkCommonMisc \
    vtkCommonSystem \
    vtkCommonTransforms \
    vtkFiltersCore \
    vtkFiltersGeneral \
    vtkFiltersSources \
    vtkGUISupportQt \
    vtkImagingCore \
    vtkImagingGeneral \
    vtkImagingHybrid \
    vtkImagingMath \
    vtkInteractionImage \
    vtkInteractionStyle \
    vtkInteractionWidgets \
    vtkIOImage \
    vtkRenderingAnnotation \
    vtkRenderingCore \
    vtkRenderingFreeType \
    vtkRenderingImage \
    vtkRenderingOpenGL2 \
    vtkRenderingVolume \
#vtkCommonColor \
#vtkCommonComputationalGeometry \
#vtkCommonCore \
#vtkCommonDataModel \
#vtkCommonExecutionModel \
#vtkCommonMath \
#vtkCommonMisc \
#vtkCommonSystem \
#vtkCommonTransforms \
#vtkDICOMParser \
#vtkFiltersCore \
#vtkFiltersExtraction \
#vtkFiltersGeneral \
#vtkFiltersGeometry \
#vtkFiltersHybrid \
#vtkFiltersModeling \
#vtkFiltersSources \
#vtkFiltersStatistics \
#vtkfreetype \
#vtkglew \
#vtkImagingColor \
#vtkImagingCore \
#vtkImagingFourier \
#vtkImagingGeneral \
#vtkImagingHybrid \
#vtkImagingMath \
#vtkImagingSources \
#vtkInteractionImage \
#vtkInteractionStyle \
#vtkInteractionWidgets \
#vtkIOImage \
#vtkjpeg \
#vtkmetaio \
#vtkpng \
#vtkRenderingAnnotation \
#vtkRenderingCore \
#vtkRenderingFreeType \
#vtkRenderingOpenGL2 \
#vtkRenderingVolume \
#vtkRenderingVolumeOpenGL2 \
#vtksys \
#vtktiff \
#vtkzlib \
#vtkGUISupportQt \
#vtkRenderingImage \
    vtkRenderingVolumeOpenGL2

LIBS += -L$${VTKLIBDIR}

for(lib, VTKLIBS) {
    LIBS += -l$${lib}$${VTKLIBSUFFIX}
}


#3.0.0
INCLUDEPATH +=  $${GDCMINCLUDEDIR} \
                $${GDCMINCLUDEDIR}/Source/Common/ \
                $${GDCMINCLUDEDIR}/Source/MediaStorageAndFileFormat/ \
                $${GDCMINCLUDEDIR}/Source/DataStructureAndEncodingDefinition/ \
                $${GDCMINCLUDEDIR}/Source/DataDictionary/ \
                $${GDCMINCLUDEDIR}/Source/InformationObjectDefinition/ \
                $${GDCMINCLUDEDIR}/Source/Attribute/ \
                $${GDCMINCLUDEDIR}/Source/Common/ \
                $${GDCMINCLUDEDIR}/Source/MessageExchangeDefinition/ \
                $${GDCMINCLUDEDIR}/more/ \
                $${GDCMINCLUDEDIR}/Utilities/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmcharls/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmexpat/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmjpeg/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmmd5/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmopenjpeg/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmrle/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmutfcpp/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmuuid/ \
                $${GDCMINCLUDEDIR}/Utilities/gdcmzlib/ \
                $${GDCMINCLUDEDIR}/Utilities/getopt/ \
                $${GDCMINCLUDEDIR}/Utilities/KWStyle/ \
                $${GDCMINCLUDEDIR}/Utilities/pvrg/ \
                $${GDCMINCLUDEDIR}/Utilities/VTK/ \
                $${GDCMINCLUDEDIR}/Utilities/socketxx

LIBS += -L$${GDCMLIBDIR} \
        -lgdcmcharls \
        -lgdcmCommon \
        -lgdcmDICT \
        -lgdcmDSED \
        -lgdcmexpat \
        -lgdcmIOD \
        -lgdcmjpeg8 \
        -lgdcmjpeg12 \
        -lgdcmjpeg16 \
        -lgdcmMSFF \
-lgdcmopenjp2 \
        -lvtkgdcm

win32 {
    LIBS += -lgdcmgetopt -lgdcmzlib
}
#//---------------------------------------------------------------------


SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui


LIBS += -lws2_32 #dcmtk >= 3.6.3 need
LIBS += -lNetAPI32 -lWSock32 -lSnmpAPI
LIBS += -ladvapi32 \
         -lRpcrt4 \
         -lwbemuuid \
         -lIphlpapi


win32:LIBS += -lShell32 -lgdi32 -luser32

#QMAKE_LFLAGS_DEBUG      = /DEBUG /NODEFAULTLIB:ITKIOGDCM-4.13.lib
#QMAKE_LFLAGS_RELEASE    = /DEBUG /NODEFAULTLIB:ITKIOGDCM-4.13.lib
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
