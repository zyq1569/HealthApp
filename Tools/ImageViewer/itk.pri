include(defaultdirectories.pri)

#INCLUDEPATH += $${ITKINCLUDEDIR}

INCLUDEPATH += $${ITKINCLUDEDIR} \
        $${ITKINCLUDEDIR}/../ \
        $${ITKINCLUDEDIR}/Core/Common/include/ \
        $${ITKINCLUDEDIR}/Core/ImageAdaptors/include/ \
        $${ITKINCLUDEDIR}/Core/FiniteDifference/include/ \
        $${ITKINCLUDEDIR}/Core/Transform/include/ \
        $${ITKINCLUDEDIR}/Core/ImageFunction/include/ \
        $${ITKINCLUDEDIR}/IO/ImageBase/include/ \
        $${ITKINCLUDEDIR}/IO/GDCM/include/ \
        $${ITKINCLUDEDIR}/ThirdParty/ZLIB/src/ \
        $${ITKINCLUDEDIR}/ThirdParty/KWSys/src/ \
        $${ITKINCLUDEDIR}/ThirdParty/VNL/src/vxl/core/ \
        $${ITKINCLUDEDIR}/ThirdParty/VNL/src/vxl/core/vnl/ \
        $${ITKINCLUDEDIR}/ThirdParty/VNL/src/vxl/vcl/ \
        $${ITKINCLUDEDIR}/ThirdParty/VNL/src/vxl/vcl/generic/ \
        $${ITKINCLUDEDIR}/ThirdParty/VNL/src/vxl/vcl/emulation/ \
        $${ITKINCLUDEDIR}/ThirdParty/MetaIO/src/MetaIO/ \
        $${ITKINCLUDEDIR}/Segmentation/RegionGrowing/include \
        $${ITKINCLUDEDIR}/Segmentation/LevelSets/include/ \
        $${ITKINCLUDEDIR}/Segmentation/LabelVoting/include/ \
        $${ITKINCLUDEDIR}/Bridge/VTK/include/ \
        $${ITKINCLUDEDIR}/Filtering/ImageFilterBase/include/ \
        $${ITKINCLUDEDIR}/Filtering/AnisotropicSmoothing/include/ \
        $${ITKINCLUDEDIR}/Filtering/Thresholding/include/ \
        $${ITKINCLUDEDIR}/Filtering/CurvatureFlow/include/ \
        $${ITKINCLUDEDIR}/Filtering/Smoothing/include/ \
        $${ITKINCLUDEDIR}/Filtering/ImageGradient/include/ \
        $${ITKINCLUDEDIR}/Filtering/FastMarching/include/ \
        $${ITKINCLUDEDIR}/Filtering/BinaryMathematicalMorphology/include/ \
        $${ITKINCLUDEDIR}/Filtering/MathematicalMorphology/include/ \
        $${ITKINCLUDEDIR}/Filtering/ImageIntensity/include/ \
        $${ITKINCLUDEDIR}/Filtering/ImageFeature/include/ \
        $${ITKINCLUDEDIR}/Filtering/ImageGrid/include/ \
        $${ITKINCLUDEDIR}/Filtering/FFT/include/ \
        $${ITKINCLUDEDIR}/Filtering/ImageStatistics/include/ \
        $${ITKINCLUDEDIR}/Filtering/DistanceMap/include/ \
        $${ITKINCLUDEDIR}/Numerics/Statistics/include/ \
        $${ITKINCLUDEDIR}/Numerics/Optimizers/include/ \
        $${ITKINCLUDEDIR}/Registration/Common/include/ \
        $${ITKINCLUDEDIR}/Core/SpatialObjects/include/



ITKLIBSUFFIX = -4.7
#ITKLIBS = \
#        ITKCommon \
#        ITKDICOMParser \
#        ITKEXPAT \
#        ITKFEM \
#        ITKMetaIO \
#        ITKNrrdIO \
#        ITKSpatialObjects \
#        ITKStatistics \
#        itkjpeg \
#        itksys \
#        itkvcl \
#        itkvnl \
#        itkvnl_algo \
#        ITKVNLInstantiation \
#        ITKniftiio \
#        ITKznz \
#        ITKOptimizers \
#        ITKIOImageBase \
#        ITKVTK \
#        ITKIOGDCM \
#        itksys \
#        itkdouble-conversion

win32 {
 ITKLIBS += itktiff \
         itkpng \
         itkv3p_netlib \
         itkzlib

 LIBS += -lShell32
}

macx {
 ITKLIBS += itkv3p_netlib
}

LIBS += -L$${ITKLIBDIR} \
        -lITKCommon-4.7 \
        -lITKDICOMParser-4.7 \
        -lITKEXPAT-4.7 \
        -lITKFEM-4.7 \
        -lITKMetaIO-4.7 \
        -lITKNrrdIO-4.7 \
        -lITKSpatialObjects-4.7 \
        -lITKStatistics-4.7 \
        -litkjpeg-4.7 \
        -litksys-4.7 \
        -litkvcl-4.7 \
        -litkvnl-4.7 \
        -litkvnl_algo-4.7 \
        -lITKVNLInstantiation-4.7 \
        -lITKniftiio-4.7 \
        -lITKznz-4.7 \
        -lITKOptimizers-4.7 \
        -lITKIOImageBase-4.7 \
        -lITKIOGDCM-4.7 \
        -lITKVTK-4.7 \
        -litksys-4.7 \
        -litkdouble-conversion-4.7

#        -lITKIOGDCM-4.7 \
#for(lib, ITKLIBS) {
#    LIBS += -l$${lib}$${ITKLIBSUFFIX}
#}


