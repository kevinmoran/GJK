#Kevin's Cross-Platform Makefile
#for Single Translation Unit Builds
#i.e. assumes only source file is main.cpp

BIN = gjk
BUILD_DIR = 

CXX = g++
#General compiler flags
COMPILER_FLAGS = -Wall -pedantic

#Debug/Release build flags
DEBUG_FLAGS = -g -DDEBUG
RELEASE_FLAGS = -O3

#Platform-specific flags
FLAGS_WIN32 = 
FLAGS_MAC = -mmacosx-version-min=10.9 -arch x86_64 -fmessage-length=0 -stdlib=libc++

#Additional include directories (common/platform-specific)
INCLUDE_COMMON = -I include
INCLUDE_DIRS_WIN32 = 
INCLUDE_DIRS_MAC = -I/sw/include -I/usr/local/include

#External libs to link to
LIB_DIR_WIN32 = libs/win32/
LIBS_WIN32 = $(LIB_DIR_WIN32)libglfw3.a
LIB_DIR_MAC = libs/osx_64/
LIBS_MAC = $(LIB_DIR_MAC)libglfw3.a

#System libs/Frameworks to link
WIN_SYS_LIBS = -lOpenGL32 -lgdi32
FRAMEWORKS = -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

SRC = main.cpp

#---------Platform Wrangling---------

#--- WINDOWS ---
ifeq ($(OS),Windows_NT)
    BIN_EXT = .exe
    FLAGS = $(COMPILER_FLAGS) $(FLAGS_WIN32)
    INCLUDE_DIRS = $(INCLUDE_COMMON) $(INCLUDE_DIRS_WIN32)
    LIBS = $(LIBS_WIN32)
    SYS_LIBS = $(WIN_SYS_LIBS)
	ifneq ($(BUILD_DIR),) #Check if build dir was specified, don't try to create one if not
    	PREBUILD = @if not exist "$(BUILD_DIR)" mkdir $(BUILD_DIR)
	endif
else
#--- UNIX ---
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		#--- MAC OS ---
        FLAGS = $(COMPILER_FLAGS) $(FLAGS_MAC)
        INCLUDE_DIRS = $(INCLUDE_COMMON) $(INCLUDE_DIRS_MAC)
        LIBS = $(LIBS_MAC)
        SYS_LIBS = $(FRAMEWORKS)
		ifneq ($(BUILD_DIR),) #Check if build dir was specified, don't try to create one if not
        	PREBUILD = @mkdir -p $(BUILD_DIR)
		endif
	else ifeq ($(UNAME_S),Linux)
		#--- LINUX ---
		# Debian: apt-get install libglfw3-dev libglvnd-dev
		# Fedora: dnf/yum install glfw-devel libglvnd-devel
		# Gentoo: emerge --noreplace media-libs/glfw media-libs/libglvnd
		LIBS = $(shell pkg-config --libs glfw3 gl)
		FLAGS = $(COMPILER_FLAGS) $(shell pkg-config --cflags glfw3 gl)
		ifneq ($(BUILD_DIR),) #Check if build dir was specified, don't try to create one if not
			PREBUILD = @mkdir -p $(BUILD_DIR)
		endif
	endif
endif


#------------TARGETS------------
all: Debug

#Prebuild task: Just makes a build directory before compiling!
#All other tasks depend on this so it always runs
#The test thing checks if it already exists first
prebuild:
	$(PREBUILD)

Debug: prebuild
	${CXX} ${FLAGS} ${DEBUG_FLAGS} -o $(BUILD_DIR)${BIN}${BIN_EXT} ${SRC} ${INCLUDE_DIRS} ${LIBS} ${SYS_LIBS}

Release: prebuild
	${CXX} ${FLAGS} ${RELEASE_FLAGS} -o $(BUILD_DIR)${BIN}${BIN_EXT} ${SRC} ${INCLUDE_DIRS} ${LIBS} ${SYS_LIBS}

Debug_timed: prebuild
	${CXX} ${FLAGS} -ftime-report ${DEBUG_FLAGS} -o $(BUILD_DIR)${BIN}${BIN_EXT} ${SRC} ${INCLUDE_DIRS} ${LIBS} ${SYS_LIBS}

Release_timed: prebuild
	${CXX} ${FLAGS} -ftime-report ${RELEASE_FLAGS} -o $(BUILD_DIR)${BIN}${BIN_EXT} ${SRC} ${INCLUDE_DIRS} ${LIBS} ${SYS_LIBS}