CC=/usr/local/bin/arm-apple-darwin-gcc
CFLAGS=-fsigned-char
LDFLAGS=-Wl,-syslibroot,/usr/local/share/iphone-filesystem
LD=$(CC)

all: optivate

optivate: optivate.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o optivate
