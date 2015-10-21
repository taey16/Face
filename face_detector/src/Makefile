# Makefile for Lemongrass library

.SUFFIXES: .o .c .cpp .a .so
CXX = g++
CC = gcc
AR = ar
CP = cp
RM = rm -f
MV = mv
ECHO = echo
DEBUG = -O2 -g -Wall -fPIC -shared

UNAME := $(shell uname)
ARCH = $($(shell uname -sm | tr \  _)_ARCH)

ifeq ($(UNAME), Linux)
NO_COLOR=
OK_COLOR=
WARN_COLOR=
ERROR_COLOR=
SUCCESS_STRING = $(OK_COLOR)[BUILD SUCCESS]$(NO_COLOR)
endif
ifeq ($(UNAME), Darwin)
NO_COLOR=\x1b[0m
OK_COLOR=\x1b[32;01m
WARN_COLOR=\x1b[33;01m
ERROR_COLOR=\x1b[31;01m
SUCCESS_STRING = $(OK_COLOR)[BUILD SUCCESS]$(NO_COLOR)
endif

HOME = .
FD_ROOT_PATH = $(HOME)/face
OD_ROOT_PATH = $(HOME)/object
UTIL_ROOT_PATH = $(HOME)/util

MAIN_SRCS:= $(wildcard $(HOME)/*.cpp)
MAIN_OBJS:= $(MAIN_SRCS:.cpp=.o)

UTIL_SRCS:= $(wildcard $(UTIL_ROOT_PATH)/*.cpp)
UTIL_OBJS:= $(UTIL_SRCS:.cpp=.o)

FD_SRCS:= $(wildcard $(FD_ROOT_PATH)/*.cpp)
FD_OBJS:= $(FD_SRCS:.cpp=.o)

OD_SRCS:= $(wildcard $(OD_ROOT_PATH)/*.cpp)
OD_OBJS:= $(OD_SRCS:.cpp=.o)

INCS = -I$(HOME) -I$(FD_ROOT_PATH) -I$(OD_ROOT_PATH) -I$(UTIL_ROOT_PATH)
OBJS = $(FD_OBJS) $(OD_OBJS) $(UTIL_OBJS) $(MAIN_OBJS) 

CXXFLAGS = $(DEBUG) $(INCS) 
ARFLAGS = crs

TARGET = libobject.a
ifeq ($(UNAME), Linux)
TARGET_SO=$(TARGET:.a=.so)
endif
ifeq ($(UNAME), Darwin)
TARGET_SO=$(TARGET:.a=.dylib)
endif

all: build
build: $(TARGET)
build_dynamic: $(TARGET_SO)

$(TARGET):$(OBJS)
	@${ECHO} "${WARN_COLOR}        $^ ${NO_COLOR}"
	$(AR) $(ARFLAGS) $@ $^ 
	@${ECHO} "${SUCCESS_STRING} ${WARN_COLOR}Static Libray${NO_COLOR}"

$(TARGET_SO): $(OBJS)
	@${ECHO} "${WARN_COLOR}        $^ ${NO_COLOR}"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS_PATH) $(LIBS) $(PKG_CONFIG_LDFLAGS)
	@${ECHO} "${SUCCESS_STRING} ${WARN_COLOR}Shared Libray${NO_COLOR}"

.cpp.o : $(OBJS)
	@${ECHO} "${ERROR_COLOR}Compiling${NO_COLOR} ${WARN_COLOR}        $< ${NO_COLOR}"
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(MAIN_OBJS)
	$(RM) $(FD_OBJS)
	$(RM) $(OD_OBJS)
	$(RM) $(UTIL_OBJS)
	$(RM) $(TARGET) $(TARGET_SO)
	$(RM) ../include/*.hpp
	$(RM) ../include/*.h
	$(RM) -rf ../lib

install: build
	mkdir -p ../lib
	$(CP) libobject.* ../lib
	$(CP) ./*.hpp ../include
	$(CP) ./*.h ../include
	$(CP) ./util/*.h ../include

revision:
	LANG=en_US.UTF-8
	LC_ALL=en_US.UTF-8
	svn info | grep Revision | awk '{print $$2}' > .revision
