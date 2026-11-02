CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
MAINS := src/main.cpp src/sales_main.cpp
SRC := $(filter-out $(MAINS),$(wildcard src/*.cpp))
OBJ := $(SRC:.cpp=.o)

all: jc8_catalog jc8_sales

jc8_catalog: $(OBJ) src/main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

jc8_sales: $(OBJ) src/sales_main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o jc8_catalog jc8_sales
