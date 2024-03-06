##
## This file is part of Ugh project. For license details, see the file
## 'LICENSE' in this package.
##

CXXFLAGS=-std=c++17 -O3 -mwindows -municode -D_WIN32 -Wall -Wshadow -Werror
LDFLAGS=-s -mwindows -municode -lcomctl32 -static

.PHONY: all
all: ugh.exe

ugh.exe: main.o app.o ugh.o
	g++ -o $@ $^ $(LDFLAGS)

ugh.o: ugh.rc
	windres -o $@ -i $<

.PHONY: clean
clean:
	rm -f *.o *.exe
