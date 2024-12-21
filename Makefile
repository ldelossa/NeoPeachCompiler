OBJS=$(subst .c,.o,$(wildcard src/*.c))
OBJS+= $(subst .c,.o,$(wildcard helpers/*.c))
CFLAGS+=-g

main: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf main
	rm -rf src/*.o
	rm -rf helpers/*.o
