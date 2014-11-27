OBJS := _divsi3.o _modsi3.o _udivsi3.o _umodsi3.o


%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o:%.S
	$(CC) $(CFLAGS) -c -o $@ $<

all:$(OBJS)
	$(AR) -rv -o $(SUBTGT) $(OBJS)

clean:
	$(RM) $(OBJS) $(SUBTGT)

.PHONY: clean
