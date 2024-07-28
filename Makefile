all: barbie_extract

barbie_extract: barbie_extract.cpp
	g++ -g -o barbie_extract barbie_extract.cpp

iterate: barbie_extract
	./barbie_extract

.PHONY: clean
clean:
	rm -f barbie_extract output/*.wav

