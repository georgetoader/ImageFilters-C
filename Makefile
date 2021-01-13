CC = gcc
CFLAG = -Wall -Wextra
EXE = tema
OBJ = tema.o
DEPS = util/bmp_header.h

build: $(EXE)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAG)

$(EXE): $(OBJ)
	$(CC) $^ $(CFLAG) -o $(EXE)

run:
	./tema

.PHONY: clean

clean:
	rm -f *.o $(EXE)

