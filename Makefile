all: barbie_extract

barbie_extract: barbie_extract.cpp
	g++ -g -o barbie_extract barbie_extract.cpp

