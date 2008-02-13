CC=/usr/local/bin/arm-apple-darwin-gcc
CFLAGS=-fsigned-char
LDFLAGS=-Wl,-syslibroot,/usr/local/share/iphone-filesystem -framework IOKit
LD=$(CC)

all: gunlock_new

gunlock_new: gunlock_new.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o gunlock_new
