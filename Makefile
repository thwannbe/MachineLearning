CC=g++
INCLUDE=-I include
OBJS = test.o List.o

.PHONY: all clean

all: test

test: $(OBJS) ; $(LD) $(LDFLAGS) -o $@ $^

$(OBJS): $($@:.o=.c)
	$(CC) $(CCFLAGS) -o $@ $^

clean:
	rm test
	rm *.o

