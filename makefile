# Makefile para el proyecto SOchain
# Miguel Angel Lopez Sanchez fc65675
# Alejandro Dominguez fc64447
# Bruno Felisberto fc32447

CC = gcc
CFLAGS = -Wall -Wextra -g

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))
EXEC = $(BINDIR)/SOchain

all: $(BINDIR) $(EXEC)

$(BINDIR):
	mkdir -p $(BINDIR)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(EXEC)

clean:
	rm -f $(OBJ)
	rm -f $(EXEC)

