################################################################
# Makefile for Vector Boost
# Written by J.Y.
# 2010.10.29

CXX = g++

DEBUG = -g -O2

INCS_DIR = ../include
SRCS_DIR = .
LIBS_DIR = ../bin

PKG_CONFIG_SRCS = `pkg-config --cflags 'opencv'`
PKG_CONFIG_LIBS = `pkg-config --libs 'opencv'`

LIBS = -lm -lobject 

CXXFLAGS = $(DEBUG) $(PKG_CONFIG_SRCS) -I$(INCS_DIR)
LINKFLAGS = $(DEBUG) -L$(LIBS_DIR)

SRCS = $(SRCS_DIR)/main.cpp $(SRCS_DIR)/ImageProcessing.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = object_detector_sample.e

build: all
all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) $(LINKFLAGS) -o $(TARGET) $(OBJS) $(LIBS) $(PKG_CONFIG_LIBS) 

.cpp .o: $(OBJS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(OBJECT_OBJS) $(TARGET)
