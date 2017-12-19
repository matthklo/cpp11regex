.PHONY: out

out: cpp11regex.out

cpp11regex.out: cpp11regex/xgetopt.c cpp11regex/xgetopt.h cpp11regex/main.cpp
	g++ --std=c++11 -o cpp11regex.out cpp11regex/xgetopt.c cpp11regex/main.cpp

