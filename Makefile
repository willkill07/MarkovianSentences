CXX := clang++
CXXFLAGS := -O3 -march=native
CPPFLAGS := -std=c++14

ifdef DEBUG
CXXFLAGS := -O0 -g
endif

all : Markov

.PHONY : clean

clean :
	@rm -vf Markov
