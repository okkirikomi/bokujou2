CC=gcc
CXX=g++
CXXFLAGS=-std=c++11 -Wall -Wextra -Wpedantic -Wunreachable-code -Wshadow -Wstrict-aliasing -pedantic-errors -fno-exceptions
RM=rm -f
INCLUDE_DIR=/usr/local/include/
CPPFLAGS=-I$(INCLUDE_DIR) -g -O2
LDFLAGS=
LDLIBS=
SRC_DIR=.
OBJ_DIR=obj

SRCS=$(wildcard $(SRC_DIR)/*.cpp)
OBJS=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: text_dump

text_dump: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS)
