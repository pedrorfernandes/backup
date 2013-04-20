CFLAGS = -Wall
BINDIR = ./bin

DEPS= backup.h restore.h utilities.h
OBJ_SHARED = utilities.o
OBJ_RESTORE = restore.o
OBJ_BACKUP = backup.o

all: backup restore

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

restore: $(OBJ_RESTORE) $(OBJ_SHARED)
	$(CC) $(CFLAGS) $(OBJ_RESTORE) $(OBJ_SHARED) -o $(BINDIR)/rstr

backup: $(OBJ_BACKUP) $(OBJ_SHARED)
	$(CC) $(CFLAGS) $(OBJ_BACKUP) $(OBJ_SHARED) -o $(BINDIR)/bckp

.PHONY: clean dist all

clean:
	rm -f *~ *.o *.a $(BINDIR)/*

dist: 
	cd .. && tar -zcvf T1G05.tar.gz T1G05/