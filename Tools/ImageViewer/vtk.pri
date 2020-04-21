include(defaultdirectories.pri)

INCLUDEPATH += $${VTKINCLUDEDIR} \
             $${VTKINCLUDEDIR}/../ \
             $${VTKINCLUDEDIR}/Common/ExecutionModel \
             $${VTKINCLUDEDIR}/Common/Core \
             $${VTKINCLUDEDIR}/Common/DataModel \
             $${VTKINCLUDEDIR}/Common/Math \
             $${VTKINCLUDEDIR}/Common/Transforms \
             $${VTKINCLUDEDIR}/Rendering/Volume \
             $${VTKINCLUDEDIR}/Rendering/Annotation \
             $${VTKINCLUDEDIR}/Rendering/Core \
             $${VTKINCLUDEDIR}/Rendering/FreeType \
             $${VTKINCLUDEDIR}/Interaction/Widgets \
             $${VTKINCLUDEDIR}/Interaction/Style \
             $${VTKINCLUDEDIR}/Interaction/Image \
             $${VTKINCLUDEDIR}/Filters/Sources \
             $${VTKINCLUDEDIR}/Filters/Core \
             $${VTKINCLUDEDIR}/Filters/Hybrid \
             $${VTKINCLUDEDIR}/IO/Image \
             $${VTKINCLUDEDIR}/GUISupport/Qt \
             $${VTKINCLUDEDIR}/Interaction/Style \
             $${VTKINCLUDEDIR}/Rendering/OpenGL2 \
             $${VTKINCLUDEDIR}/Rendering/Image \
             $${VTKINCLUDEDIR}/Rendering/VolumeOpenGL \
             $${VTKINCLUDEDIR}/Common/Misc \
             $${VTKINCLUDEDIR}/Imaging/General \
             $${VTKINCLUDEDIR}/Imaging/Core \
             $${VTKINCLUDEDIR}/Imaging/Hybrid \
             $${VTKINCLUDEDIR}/Imaging/Math

#VTKLIBSUFFIX = -8.2
#LIBS += -L$${VTKLIBDIR} \
#    -lvtkChartsCore-8.2 \
#    -lvtkCommonDataModel-8.2 \
#    -lvtkCommonExecutionModel-8.2 \
#    -lvtkCommonMath-8.2 \
#    -lvtkCommonMisc-8.2 \
#    -lvtkCommonTransforms-8.2 \
#    -lvtkFiltersCore-8.2 \
#    -lvtkFiltersSources-8.2 \
#    -lvtkGUISupportQt-8.2 \
#    -lvtkImagingCore-8.2 \
#    -lvtkImagingGeneral-8.2 \
#    -lvtkImagingHybrid-8.2 \
#    -lvtkImagingMath-8.2 \
#    -lvtkInteractionStyle-8.2 \
#    -lvtkInteractionWidgets-8.2 \
#    -lvtkIOImage-8.2 \
#    -lvtkRenderingAnnotation-8.2 \
#    -lvtkRenderingCore-8.2 \
#    -lvtkRenderingFreeType-8.2 \
#    -lvtkRenderingImage-8.2 \
#    -lvtkRenderingOpenGL-8.2 \
#    -lvtkRenderingVolume-8.2 \
#    -lvtkRenderingVolumeOpenGL-8.2
VTKLIBSUFFIX = -6.1
LIBS += -L$${VTKLIBDIR} \
    -lvtkChartsCore-6.1 \
    -lvtkCommonDataModel-6.1 \
    -lvtkCommonExecutionModel-6.1 \
    -lvtkCommonMath-6.1 \
    -lvtkCommonMisc-6.1 \
    -lvtkCommonTransforms-6.1 \
    -lvtkFiltersCore-6.1 \
    -lvtkFiltersSources-6.1 \
    -lvtkGUISupportQt-6.1 \
    -lvtkImagingCore-6.1 \
    -lvtkImagingGeneral-6.1 \
    -lvtkImagingHybrid-6.1 \
    -lvtkImagingMath-6.1 \
    -lvtkInteractionStyle-6.1 \
    -lvtkInteractionWidgets-6.1 \
    -lvtkIOImage-6.1 \
    -lvtkRenderingAnnotation-6.1 \
    -lvtkRenderingCore-6.1 \
    -lvtkRenderingFreeType-6.1 \
    -lvtkRenderingImage-6.1 \
    -lvtkRenderingOpenGL-6.1 \
    -lvtkRenderingVolume-6.1 \
    -lvtkRenderingVolumeOpenGL-6.1

#LIBS += -L$${VTKLIBDIR}
#LIBS += -L$${LOG4CXXLIBDIR} -llog4cxx
#for(lib, VTKLIBS) {
#    LIBS += -l$${lib}
#}
