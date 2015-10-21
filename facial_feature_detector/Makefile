
CXX = g++
CC = gcc
AR = ar
CP = cp
RM = rm -f
MV = mv

INC_DIR = ./include
SRC_DIR = ./src
LIB_DIR = ./lib

OPENCV_INCS = `pkg-config --cflags 'opencv'`
OPENCV_LIBS = `pkg-config --libs 'opencv'`

INCS = -I$(INC_DIR) $(OPENCV_INCS)
LIBS = -lm $(OPENCV_LIBS)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)

CXXFLAGS = -O3 -g -Wall $(INCS)

TARGET = $(LIB_DIR)/libffd.a

all:	build

build: $(TARGET)

$(TARGET):	$(OBJS)
	mkdir -p $(LIB_DIR)
	ar -r $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(LIB_DIR)
