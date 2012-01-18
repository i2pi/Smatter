SRC=main.c data.c read_csv.c transform.c
LIBS=libmba-0.9.1/libmba.a -lm
CFLAGS=-Wall -g -Ilibmba-0.9.1/src
PROJ=smatter

$(PROJ): Makefile $(SRC)
	gcc $(CFLAGS) -o $(PROJ) $(SRC) $(LIBS)

clean: 
	rm -f $(PROJ)
	
