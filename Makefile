CC = gcc 
CFLAGS = -g 
CFILES = main.c
OFILES = main.o
OBJNAME = main

%.o: %.c #$(HFILES)
	$(CC) -c $< -o $@

main: $(OFILES)
	@ $(CC) $(OFILES) -o $(OBJNAME)
clean:
	rm -f *.o
	rm -f main 