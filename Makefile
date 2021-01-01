##
CC = g++
CFLAGS = -O3 -Wall -std=c++11 -I./include

LFLAGS = -pthread

##
SRC = $(wildcard src/*.cpp)
SEARCH = $(wildcard src/searchmain/*.cpp)
INDEX= $(wildcard src/indexmain/*.cpp)

OBJS = $(SRC:.cpp=.o)
OBJSEARCH = $(SEARCH:.cpp=.o)
OBJINDEX = $(INDEX:.cpp=.o)

TARGET1 = nass
TARGET2 = nass-index

##
.suffixes:
	.cpp

%.o:%.cpp
	$(CC) -c $< $(CFLAGS) -o $@

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJS) $(OBJSEARCH)
	$(CC) -o $@ $(OBJS) $(OBJSEARCH) $(LFLAGS) 

$(TARGET2): $(OBJS) $(OBJINDEX)
	$(CC) -o $@ $(OBJS) $(OBJINDEX) $(LFLAGS) 

clean:
	rm -f *~ */*~ $(OBJS) $(OBJSEARCH) $(OBJINDEX) $(TARGET1) $(TARGET2)
