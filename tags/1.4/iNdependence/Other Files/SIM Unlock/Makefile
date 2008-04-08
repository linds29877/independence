CC=/usr/local/bin/arm-apple-darwin-gcc
CFLAGS=-fsigned-char -DDEBUG
LDFLAGS=-Wl,-syslibroot,/usr/local/share/iphone-filesystem -framework IOKit -framework CoreFoundation
LD=$(CC)

all: gunlock_0p gunlock_0p_new getversion

gunlock_0p: gunlock_0p.o
	$(LD) $(LDFLAGS) -o $@ $^

gunlock_0p_new: gunlock_0p_new.o
	$(LD) $(LDFLAGS) -o $@ $^

getversion: getversion.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o gunlock_0p gunlock_0p_new getversion
