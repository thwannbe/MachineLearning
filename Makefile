CC=g++

INCLUDE=-I include

SOURCES=$(SRCDIR)/*.cpp

SRCDIR=./src

.PHONY: all clean

all: namiML

namiML: $(SOURCES)
	$(CC) -o $@ $^ $(INCLUDE)

clean:
	rm namiML

