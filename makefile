# Archivos fuente: main.c, memory.c, process.c, server.c, wallet.c
# Ejecutable: SOchain

# Miguel Angel Lopez Sanchez fc65675
# Alejandro Dominguez fc64447
# Bruno Felisberto fc32435

CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ = main.o memory.o process.o server.o wallet.o
EXEC = SOchain

#Comprobar el ficheor .bin añadirlo al proyecto cuando se haga el main para que funcione

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXEC)
	./$(EXEC)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(EXEC)

re: fclean all

.PHONY: all run clean fclean re
