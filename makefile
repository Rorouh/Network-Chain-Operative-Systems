# Miguel Angel Lopez Sanchez fc65675
# Alejandro Dominguez fc64447
# Bruno Felisberto fc32447

CC = gcc
CFLAGS = -Wall -g

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = main.c memory.c process.c server.c wallet.c synchronization.c csettings.c ctime.c clog.c csignals.c cstats.c
OBJECTS = main.o memory.o process.o server.o wallet.o synchronization.o csettings.o ctime.o clog.o csignals.o cstats.o

EXEC = SOchain

all: $(BINDIR)/$(EXEC)

$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR)/$(EXEC): $(addprefix $(OBJDIR)/, $(OBJECTS)) | $(BINDIR)
	$(CC) $(CFLAGS) -o $(BINDIR)/$(EXEC) $(addprefix $(OBJDIR)/, $(OBJECTS))

$(OBJDIR)/main.o: $(SRCDIR)/main.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $(OBJDIR)/main.o

$(OBJDIR)/memory.o: $(SRCDIR)/memory.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/memory.c -o $(OBJDIR)/memory.o

$(OBJDIR)/process.o: $(SRCDIR)/process.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/process.c -o $(OBJDIR)/process.o

$(OBJDIR)/server.o: $(SRCDIR)/server.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/server.c -o $(OBJDIR)/server.o

$(OBJDIR)/wallet.o: $(SRCDIR)/wallet.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/wallet.c -o $(OBJDIR)/wallet.o

$(OBJDIR)/synchronization.o: $(SRCDIR)/synchronization.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/synchronization.c -o $(OBJDIR)/synchronization.o

$(OBJDIR)/csettings.o: $(SRCDIR)/csettings.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/csettings.c -o $(OBJDIR)/csettings.o

$(OBJDIR)/ctime.o: $(SRCDIR)/ctime.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/ctime.c -o $(OBJDIR)/ctime.o

$(OBJDIR)/clog.o: $(SRCDIR)/clog.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/clog.c -o $(OBJDIR)/clog.o

$(OBJDIR)/csignals.o: $(SRCDIR)/csignals.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/csignals.c -o $(OBJDIR)/csignals.o

 $(OBJDIR)/cstats.o: $(SRCDIR)/cstats.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/csignals.c -o $(OBJDIR)/csignals.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/cstats.c -o $(OBJDIR)/cstats.o

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/$(EXEC)

