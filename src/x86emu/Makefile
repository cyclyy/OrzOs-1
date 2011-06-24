OBJS = decode.o fpu.o ops.o ops2.o prim_ops.o sys.o
CFLAGS += -DNO_SYS_HEADERS

libx86emu.a: $(OBJS)
	ar rv $@ $+

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

CFLAGS += -I. -I../../include -I../../include/x86emu

clean:
	rm -f *.a *.o

