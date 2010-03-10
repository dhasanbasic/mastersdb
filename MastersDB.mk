##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=MastersDB
ConfigurationName      :=Debug
IntermediateDirectory  :=./bin/Debug
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "D:\DEV\workspaces\mastersdb"
ProjectPath            := "D:\DEV\workspaces\mastersdb"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Dinko
Date                   :=03/10/10
CodeLitePath           :="D:\DEV\CodeLite"
LinkerName             :=gcc
ArchiveTool            :=ar rcus
SharedObjectLinkerName :=gcc -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
CompilerName           :=gcc
C_CompilerName         :=gcc
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=
MakeDirCommand         :=makedir
CmpOptions             := -g $(Preprocessors)
LinkOptions            :=  
IncludePath            :=  "$(IncludeSwitch)./include" "$(IncludeSwitch)." 
RcIncludePath          :=
Libs                   :=
LibPath                := "$(LibraryPathSwitch)." 


##
## User defined environment variables
##
UNIT_TEST_PP_SRC_DIR:=D:\DEV\UnitTest++-1.3
WXCFG:=gcc_dll\mswu
WXWIN:=D:\DEV\wxWidgets-2.8.10
Objects=src/$(IntermediateDirectory)/btree$(ObjectSuffix) src/$(IntermediateDirectory)/libloader$(ObjectSuffix) src/$(IntermediateDirectory)/mastersdb$(ObjectSuffix) 

##
## Main Build Targets 
##
all: $(OutputFile)

$(OutputFile): makeDirStep $(Objects)
	@$(MakeDirCommand) $(@D)
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)

makeDirStep:
	@$(MakeDirCommand) "./bin/Debug"

PreBuild:


##
## Objects
##
src/$(IntermediateDirectory)/btree$(ObjectSuffix): src/btree.c src/$(IntermediateDirectory)/btree$(DependSuffix)
	@$(MakeDirCommand) "src/bin/Debug"
	$(C_CompilerName) $(SourceSwitch) "D:/DEV/workspaces/mastersdb/src/btree.c" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/btree$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/btree$(DependSuffix): src/btree.c
	@$(MakeDirCommand) "src/bin/Debug"
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/btree$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/btree$(DependSuffix) -MM "D:/DEV/workspaces/mastersdb/src/btree.c"

src/$(IntermediateDirectory)/libloader$(ObjectSuffix): src/libloader.c src/$(IntermediateDirectory)/libloader$(DependSuffix)
	@$(MakeDirCommand) "src/bin/Debug"
	$(C_CompilerName) $(SourceSwitch) "D:/DEV/workspaces/mastersdb/src/libloader.c" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/libloader$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/libloader$(DependSuffix): src/libloader.c
	@$(MakeDirCommand) "src/bin/Debug"
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/libloader$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/libloader$(DependSuffix) -MM "D:/DEV/workspaces/mastersdb/src/libloader.c"

src/$(IntermediateDirectory)/mastersdb$(ObjectSuffix): src/mastersdb.c src/$(IntermediateDirectory)/mastersdb$(DependSuffix)
	@$(MakeDirCommand) "src/bin/Debug"
	$(C_CompilerName) $(SourceSwitch) "D:/DEV/workspaces/mastersdb/src/mastersdb.c" $(CmpOptions) $(ObjectSwitch)src/$(IntermediateDirectory)/mastersdb$(ObjectSuffix) $(IncludePath)
src/$(IntermediateDirectory)/mastersdb$(DependSuffix): src/mastersdb.c
	@$(MakeDirCommand) "src/bin/Debug"
	@$(C_CompilerName) $(CmpOptions) $(IncludePath) -MTsrc/$(IntermediateDirectory)/mastersdb$(ObjectSuffix) -MFsrc/$(IntermediateDirectory)/mastersdb$(DependSuffix) -MM "D:/DEV/workspaces/mastersdb/src/mastersdb.c"


-include src/$(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) src/$(IntermediateDirectory)/btree$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/btree$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/btree$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/libloader$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/libloader$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/libloader$(PreprocessSuffix)
	$(RM) src/$(IntermediateDirectory)/mastersdb$(ObjectSuffix)
	$(RM) src/$(IntermediateDirectory)/mastersdb$(DependSuffix)
	$(RM) src/$(IntermediateDirectory)/mastersdb$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe


