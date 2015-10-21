
#CXX = g++44
CXX = g++
CC = gcc
AR = ar
CP = cp
RM = rm -f

INC_DIR = ./include
SRC_DIR = ./src
LIB_DIR = ./lib
BIN_DIR = ./bin

OPENCV_INCS = `pkg-config --cflags 'opencv'`
OPENCV_LIBS = `pkg-config --libs 'opencv'`

FACE_DETECTOR_PATH = ../face_detector
FACE_DETECTOR_LIB = $(FACE_DETECTOR_PATH)/lib/libobject.a
FACE_DETECTOR_INC = $(FACE_DETECTOR_PATH)/include

FACIAL_FEATURE_DETECTOR_PATH = ../facial_feature_detector
FACIAL_FEATURE_DETECTOR_LIB = $(FACIAL_FEATURE_DETECTOR_PATH)/lib/libffd.a 
FACIAL_FEATURE_DETECTOR_INC = $(FACIAL_FEATURE_DETECTOR_PATH)/include

INCS = -I$(INC_DIR) $(OPENCV_INCS) -I$(FACIAL_FEATURE_DETECTOR_INC) -I$(FACE_DETECTOR_INC)
LDFLAGS = -lm $(OPENCV_LIBS) $(FACE_DETECTOR_FLAG) $(FACIAL_FEATURE_DETECTOR_FLAG)
LIBS = $(FACIAL_FEATURE_DETECTOR_LIB) $(FACIAL_FEATURE_TRAINER_LIB) $(FACE_DETECTOR_LIB)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)

CXXFLAGS = -O3 -g -Wall -fopenmp $(INCS) -std=c++0x

TARGET = $(BIN_DIR)/face_demo.exe

all: build

build: $(TARGET)

$(TARGET): $(OBJS) $(LIBS)
	mkdir -p $(BIN_DIR)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(BIN_DIR)
