APP = battnotif
CC = gcc

CFLAGS = -O3 -g3  -W -Wall -Wextra -Wuninitialized -Wstrict-aliasing -std=c11
LIBS = -D_REENTRANT -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -lm -lpulse -pthread -ldbus-1

battnotif: battnotif.c
	$(CC) $(CFLAGS) -o $(APP) $(LIBS) battnotif.c

clean:
	rm $(APP)

default: $(APP)
