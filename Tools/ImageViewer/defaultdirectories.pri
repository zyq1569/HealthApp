# Directoris per defecte de les diferents llibreries. Si vols que siguin uns altres, simplement has de declarar
# com a variables de sistema les que vulguis substituir. Ex.: export ITKLIBDIR=/usr/lib64/InsightToolkit

# DCMTK Libraries

DCMTKLIBDIR = $$(DCMTKLIBDIR)
isEmpty(DCMTKLIBDIR){
    unix:DCMTKLIBDIR = $$(SDK_INSTALL_PREFIX)/lib
    macx:DCMTKLIBDIR = /usr/local/lib
    win32:DCMTKLIBDIR = D:/code/C++/starviewer-sdk/dcmtk-3.6.3/lib
    win32:contains(QMAKE_TARGET.arch, x86_64):DCMTKLIBDIR = $$(SystemDrive)/dcmtk/3.6.1_20120515-64/lib
}
DCMTKINCLUDEDIR = $$(DCMTKINCLUDEDIR)
isEmpty(DCMTKINCLUDEDIR){
    unix:DCMTKINCLUDEDIR = $$(SDK_INSTALL_PREFIX)/include/dcmtk
    macx:DCMTKINCLUDEDIR = /usr/local/include/dcmtk
    win32:DCMTKINCLUDEDIR = $$(SystemDrive)/dcmtk/3.6.1_20120515-64/include/dcmtk
    win32:contains(QMAKE_TARGET.arch, x86_64):DCMTKINCLUDEDIR = $$(SystemDrive)/dcmtk/3.6.1_20120515-64/include/dcmtk
}


# VTK Libraries

VTKLIBDIR = $$(VTKLIBDIR)
isEmpty(VTKLIBDIR){
    unix:VTKLIBDIR = $$(SDK_INSTALL_PREFIX)/lib
    macx:VTKLIBDIR = /usr/local/lib
    win32:VTKLIBDIR = D:/code/C++/starviewer-sdk/VTK-8.2.0/lib
    win32:contains(QMAKE_TARGET.arch, x86_64):VTKLIBDIR = $$(SystemDrive)/VTK/6.1.0-64/lib
}
VTKINCLUDEDIR = $$(VTKINCLUDEDIR)
isEmpty(VTKINCLUDEDIR){
    unix:VTKINCLUDEDIR = $$(SDK_INSTALL_PREFIX)/include/vtk-6.1
    macx:VTKINCLUDEDIR = /usr/local/include/vtk-6.1
    win32:VTKINCLUDEDIR = C:/DEV_SDK/VTK/include/VTK-8.2.0
    win32:contains(QMAKE_TARGET.arch, x86_64):VTKINCLUDEDIR =C:/VTK/6.1.0-64/include/vtk-6.1
}


# ITK Libraries

ITKLIBDIR = $$(ITKLIBDIR)
isEmpty(ITKLIBDIR){
    unix:ITKLIBDIR = $$(SDK_INSTALL_PREFIX)/lib
    macx:ITKLIBDIR = /usr/local/lib
    win32:ITKLIBDIR =  D:/code/C++/starviewer-sdk/InsightToolkit-5.0.1/lib
    win32:contains(QMAKE_TARGET.arch, x86_64):ITKLIBDIR = $$(SystemDrive)/InsightToolkit/4.7.1-64/lib
}
ITKINCLUDEDIR = $$(ITKINCLUDEDIR)
isEmpty(ITKINCLUDEDIR){
    unix:ITKINCLUDEDIR = $$(SDK_INSTALL_PREFIX)/include/ITK-4.7
    macx:ITKINCLUDEDIR = /usr/local/include/ITK-4.7
    win32:ITKINCLUDEDIR = C:/InsightToolkit/4.7.1-64/include/ITK-4.7
    win32:contains(QMAKE_TARGET.arch, x86_64):ITKINCLUDEDIR = C:/InsightToolkit/4.7.1-64/include/ITK-4.7
}

# GDCM Libraries

GDCMLIBDIR = $$(GDCMLIBDIR)
isEmpty(GDCMLIBDIR){
    unix:GDCMLIBDIR = $$(SDK_INSTALL_PREFIX)/lib
    macx:GDCMLIBDIR = /usr/local/lib
    win32:GDCMLIBDIR = D:/code/C++/starviewer-sdk/gdcm-3.0.5/lib
    win32:contains(QMAKE_TARGET.arch, x86_64):GDCMLIBDIR = $$(SystemDrive)/gdcm/2.4.4-64/lib
}
GDCMINCLUDEDIR = $$(GDCMINCLUDEDIR)
isEmpty(GDCMINCLUDEDIR){
    unix:GDCMINCLUDEDIR = $$(SDK_INSTALL_PREFIX)/include/gdcm-2.4
    macx:GDCMINCLUDEDIR = /usr/local/include/gdcm-2.4
    win32:GDCMINCLUDEDIR = C:/gdcm/include
    win32:contains(QMAKE_TARGET.arch, x86_64):GDCMINCLUDEDIR = $$(SystemDrive)/gdcm/2.4.4-64/include/gdcm-2.4
}

# Log4cxx Libraries

LOG4CXXLIBDIR = $$(LOG4CXXLIBDIR)
isEmpty(LOG4CXXLIBDIR){
    unix:LOG4CXXLIBDIR = /usr/lib
    macx:LOG4CXXLIBDIR = /usr/local/lib/
    win32:LOG4CXXLIBDIR = C:/log4cxx-64/lib
    win32:contains(QMAKE_TARGET.arch, x86_64):LOG4CXXLIBDIR = C:/log4cxx-64/lib
}
LOG4CXXINCLUDEDIR = $$(LOG4CXXINCLUDEDIR)
isEmpty(LOG4CXXINCLUDEDIR){
    unix:LOG4CXXINCLUDEDIR = /usr/include/log4cxx
    macx:LOG4CXXINCLUDEDIR = /usr/local/include
    win32:LOG4CXXINCLUDEDIR =C:/DEV_SDK/log4cxx-64/include
    win32:contains(QMAKE_TARGET.arch, x86_64):LOG4CXXINCLUDEDIR =C:/log4cxx-64/include
    #win32:contains(QMAKE_TARGET.arch, x86_64):LOG4CXXINCLUDEDIR = C:/log4cxx-64/include
}

# Threadweaver libraries

THREADWEAVERLIBDIR = $$(THREADWEAVERLIBDIR)
isEmpty(THREADWEAVERLIBDIR){
    # This unix default is for 64-bit Debian-based systems. It will be different for others.
    unix:THREADWEAVERLIBDIR = $$(SDK_INSTALL_PREFIX)/lib/x86_64-linux-gnu
    macx:THREADWEAVERLIBDIR = /usr/local/lib/
    win32:THREADWEAVERLIBDIR =  D:/code/C++/starviewer-sdk/threadweaver/lib
    win32:contains(QMAKE_TARGET.arch, x86_64):THREADWEAVERLIBDIR = $$(SystemDrive)/ThreadWeaver/5.3.0-64/lib
}
THREADWEAVERINCLUDEDIR = $$(THREADWEAVERINCLUDEDIR)
isEmpty(THREADWEAVERINCLUDEDIR){
    unix:THREADWEAVERINCLUDEDIR = $$(SDK_INSTALL_PREFIX)/include/KF5
    macx:THREADWEAVERINCLUDEDIR = /usr/local/include/KF5
    win32:THREADWEAVERINCLUDEDIR = $$(SystemDrive)/ThreadWeaver/5.3.0-64/include
    win32:contains(QMAKE_TARGET.arch, x86_64):THREADWEAVERINCLUDEDIR = $$(SystemDrive)/ThreadWeaver/5.3.0-64/include
}


# CUDA Libraries

CUDALIBDIR = $$(CUDALIBDIR)
isEmpty(CUDALIBDIR){
    unix:CUDALIBDIR = /usr/local/cuda/lib
}
CUDAINCLUDEDIR = $$(CUDAINCLUDEDIR)
isEmpty(CUDAINCLUDEDIR){
    unix:CUDAINCLUDEDIR = /usr/local/cuda/include
}
# De moment cal el CUDA SDK, però s'hauria de poder treballar sense ell
CUDASDKINCLUDEDIR = $$(CUDASDKINCLUDEDIR)
isEmpty(CUDASDKINCLUDEDIR){
    unix:CUDASDKINCLUDEDIR = /usr/local/cuda-sdk/common/inc
}
