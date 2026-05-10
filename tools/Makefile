CC = cc
CFLAGS = -Wall -Wextra -O2

TARGETS = dsfplay dsfinfo

all: $(TARGETS)

dsfplay: dsfplay.c
	$(CC) $(CFLAGS) -o dsfplay dsfplay.c

dsfinfo: dsfinfo.c
	$(CC) $(CFLAGS) -o dsfinfo dsfinfo.c

clean:
	rm -f $(TARGETS)
