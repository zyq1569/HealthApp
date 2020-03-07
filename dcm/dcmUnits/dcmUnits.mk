##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=dcmUnits
ConfigurationName      :=Release
WorkspaceConfiguration := $(ConfigurationName)
WorkspacePath          :=D:/code/C++/HealthApp
ProjectPath            :=D:/code/C++/HealthApp/dcm/dcmUnits
IntermediateDirectory  :=../../build-$(ConfigurationName)/dcm/dcmUnits
OutDir                 :=../../build-$(ConfigurationName)/dcm/dcmUnits
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=admin
Date                   :=2020/3/7
CodeLitePath           :="C:/Program Files/CodeLite"
LinkerName             :=C:/MinGW/bin/g++.exe
SharedObjectLinkerName :=C:/MinGW/bin/g++.exe -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g -fexec-charset=UTF-8 -finput-charset=UTF-8
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c -fexec-charset=UTF-8 -finput-charset=UTF-8
OutputFile             :=..\..\build-$(ConfigurationName)\lib\$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
RcCmpOptions           := 
RcCompilerName         :=C:/MinGW/bin/windres.exe
LinkOptions            :=  
IncludePath            := $(IncludeSwitch)..\..\include\dcm\win32\config\include $(IncludeSwitch)..\..\include\dcm\win32\ofstd\include $(IncludeSwitch)..\..\include\dcm\win32\oflog\include $(IncludeSwitch)..\..\include\dcm\win32\\dcmdata\include $(IncludeSwitch)..\..\include\dcm\win32\dcmimgle\include $(IncludeSwitch)..\..\include\dcm\win32\dcmimage\include $(IncludeSwitch)..\..\include\dcm\win32\dcmjpeg\include $(IncludeSwitch)..\..\include\dcm\win32\dcmjpls\include $(IncludeSwitch)..\..\include\dcm\win32\dcmtls\include $(IncludeSwitch)..\..\include\dcm\win32\dcmnet\include $(IncludeSwitch)..\..\include\dcm\win32\dcmsr\include $(IncludeSwitch)..\..\include\dcm\win32\dcmsign\include $(IncludeSwitch)..\..\include\dcm\win32\dcmwlm\include $(IncludeSwitch)..\..\include\dcm\win32\dcmqrdb\include $(IncludeSwitch)..\..\include\dcm\win32\dcmpstat\include $(IncludeSwitch)..\..\include\dcm\win32\dcmrt\include $(IncludeSwitch)..\..\include\dcm\win32\dcmiod\include $(IncludeSwitch)..\..\include\dcm\win32\dcmfg\include $(IncludeSwitch)..\..\include\dcm\win32\dcmseg\include $(IncludeSwitch)..\..\include\dcm\win32\dcmtract\include $(IncludeSwitch)..\..\include\dcm\win32\dcmpmap\include $(IncludeSwitch)..\..\include\dcm\win32\dcmUnits $(IncludeSwitch)..\..\include\mysql\win32  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := C:/MinGW/bin/ar.exe rcu
CXX      := C:/MinGW/bin/g++.exe
CC       := C:/MinGW/bin/gcc.exe
CXXFLAGS :=   $(Preprocessors)
CFLAGS   :=   $(Preprocessors)
ASFLAGS  := 
AS       := C:/MinGW/bin/as.exe


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
Objects0=../../build-$(ConfigurationName)/dcm/dcmUnits/CJsonObject.cpp$(ObjectSuffix) ../../build-$(ConfigurationName)/dcm/dcmUnits/HMariaDb.cpp$(ObjectSuffix) ../../build-$(ConfigurationName)/dcm/dcmUnits/cJSON.c$(ObjectSuffix) ../../build-$(ConfigurationName)/dcm/dcmUnits/DcmConfig.cpp$(ObjectSuffix) ../../build-$(ConfigurationName)/dcm/dcmUnits/Units.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs ../../build-$(ConfigurationName)/dcm/dcmUnits/$(OutputFile)

../../build-$(ConfigurationName)/dcm/dcmUnits/$(OutputFile): $(Objects)
	@if not exist "..\..\build-$(ConfigurationName)\dcm\dcmUnits" mkdir "..\..\build-$(ConfigurationName)\dcm\dcmUnits"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(AR) $(ArchiveOutputSwitch)$(OutputFile) @$(ObjectsFileList)
	@echo rebuilt > $(IntermediateDirectory)/dcmUnits.relink

MakeIntermediateDirs:
	@if not exist "..\..\build-$(ConfigurationName)\dcm\dcmUnits" mkdir "..\..\build-$(ConfigurationName)\dcm\dcmUnits"
	@if not exist ""..\..\build-$(ConfigurationName)\lib"" mkdir ""..\..\build-$(ConfigurationName)\lib""

:
	@if not exist "..\..\build-$(ConfigurationName)\dcm\dcmUnits" mkdir "..\..\build-$(ConfigurationName)\dcm\dcmUnits"

PreBuild:


##
## Objects
##
../../build-$(ConfigurationName)/dcm/dcmUnits/CJsonObject.cpp$(ObjectSuffix): CJsonObject.cpp ../../build-$(ConfigurationName)/dcm/dcmUnits/CJsonObject.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/code/C++/HealthApp/dcm/dcmUnits/CJsonObject.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/CJsonObject.cpp$(ObjectSuffix) $(IncludePath)
../../build-$(ConfigurationName)/dcm/dcmUnits/CJsonObject.cpp$(DependSuffix): CJsonObject.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../build-$(ConfigurationName)/dcm/dcmUnits/CJsonObject.cpp$(ObjectSuffix) -MF../../build-$(ConfigurationName)/dcm/dcmUnits/CJsonObject.cpp$(DependSuffix) -MM CJsonObject.cpp

../../build-$(ConfigurationName)/dcm/dcmUnits/CJsonObject.cpp$(PreprocessSuffix): CJsonObject.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../build-$(ConfigurationName)/dcm/dcmUnits/CJsonObject.cpp$(PreprocessSuffix) CJsonObject.cpp

../../build-$(ConfigurationName)/dcm/dcmUnits/HMariaDb.cpp$(ObjectSuffix): HMariaDb.cpp ../../build-$(ConfigurationName)/dcm/dcmUnits/HMariaDb.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/code/C++/HealthApp/dcm/dcmUnits/HMariaDb.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/HMariaDb.cpp$(ObjectSuffix) $(IncludePath)
../../build-$(ConfigurationName)/dcm/dcmUnits/HMariaDb.cpp$(DependSuffix): HMariaDb.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../build-$(ConfigurationName)/dcm/dcmUnits/HMariaDb.cpp$(ObjectSuffix) -MF../../build-$(ConfigurationName)/dcm/dcmUnits/HMariaDb.cpp$(DependSuffix) -MM HMariaDb.cpp

../../build-$(ConfigurationName)/dcm/dcmUnits/HMariaDb.cpp$(PreprocessSuffix): HMariaDb.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../build-$(ConfigurationName)/dcm/dcmUnits/HMariaDb.cpp$(PreprocessSuffix) HMariaDb.cpp

../../build-$(ConfigurationName)/dcm/dcmUnits/cJSON.c$(ObjectSuffix): cJSON.c ../../build-$(ConfigurationName)/dcm/dcmUnits/cJSON.c$(DependSuffix)
	$(CC) $(SourceSwitch) "D:/code/C++/HealthApp/dcm/dcmUnits/cJSON.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/cJSON.c$(ObjectSuffix) $(IncludePath)
../../build-$(ConfigurationName)/dcm/dcmUnits/cJSON.c$(DependSuffix): cJSON.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT../../build-$(ConfigurationName)/dcm/dcmUnits/cJSON.c$(ObjectSuffix) -MF../../build-$(ConfigurationName)/dcm/dcmUnits/cJSON.c$(DependSuffix) -MM cJSON.c

../../build-$(ConfigurationName)/dcm/dcmUnits/cJSON.c$(PreprocessSuffix): cJSON.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../build-$(ConfigurationName)/dcm/dcmUnits/cJSON.c$(PreprocessSuffix) cJSON.c

../../build-$(ConfigurationName)/dcm/dcmUnits/DcmConfig.cpp$(ObjectSuffix): DcmConfig.cpp ../../build-$(ConfigurationName)/dcm/dcmUnits/DcmConfig.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/code/C++/HealthApp/dcm/dcmUnits/DcmConfig.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/DcmConfig.cpp$(ObjectSuffix) $(IncludePath)
../../build-$(ConfigurationName)/dcm/dcmUnits/DcmConfig.cpp$(DependSuffix): DcmConfig.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../build-$(ConfigurationName)/dcm/dcmUnits/DcmConfig.cpp$(ObjectSuffix) -MF../../build-$(ConfigurationName)/dcm/dcmUnits/DcmConfig.cpp$(DependSuffix) -MM DcmConfig.cpp

../../build-$(ConfigurationName)/dcm/dcmUnits/DcmConfig.cpp$(PreprocessSuffix): DcmConfig.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../build-$(ConfigurationName)/dcm/dcmUnits/DcmConfig.cpp$(PreprocessSuffix) DcmConfig.cpp

../../build-$(ConfigurationName)/dcm/dcmUnits/Units.cpp$(ObjectSuffix): Units.cpp ../../build-$(ConfigurationName)/dcm/dcmUnits/Units.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/code/C++/HealthApp/dcm/dcmUnits/Units.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Units.cpp$(ObjectSuffix) $(IncludePath)
../../build-$(ConfigurationName)/dcm/dcmUnits/Units.cpp$(DependSuffix): Units.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT../../build-$(ConfigurationName)/dcm/dcmUnits/Units.cpp$(ObjectSuffix) -MF../../build-$(ConfigurationName)/dcm/dcmUnits/Units.cpp$(DependSuffix) -MM Units.cpp

../../build-$(ConfigurationName)/dcm/dcmUnits/Units.cpp$(PreprocessSuffix): Units.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) ../../build-$(ConfigurationName)/dcm/dcmUnits/Units.cpp$(PreprocessSuffix) Units.cpp


-include ../../build-$(ConfigurationName)/dcm/dcmUnits//*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


