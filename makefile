PROGRAM = code

CXX = g++
CXXFLAGS = -g -std=c++17

CPP_FILES = $(wildcard *.cpp)
H_FILES = $(wildcard *.h)
HPP_FILES = $(wildcard *.hpp)

LDOPTIONS = -L.

$(PROGRAM): $(CPP_FILES) $(H_FILES) 
	$(CXX) -O2 -o $(PROGRAM) $(CXXFLAGS) $(LDOPTIONS) $(CPP_FILES)
