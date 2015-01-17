CC=g++

INCLUDE=-I include

SOURCES=$(SRCDIR)/*.cpp

SRCDIR=./src

.PHONY: all doc clean

all: namiML

namiML: $(SOURCES)
	$(CC) -o $@ $^ $(INCLUDE)

doc:
	doxygen

clean:
	rm namiML

