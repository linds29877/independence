CC=g++
CFLAGS=-I../Public\ Headers
LDFLAGS=-L. -lPhoneInteraction -framework CoreFoundation

300MBKiller: 300MBKiller.cpp
	cp ../Public\ Libraries/Release/libPhoneInteraction.dylib .
	$(CC) -arch i386 $(CFLAGS) $(LDFLAGS) -o 300MBKiller.intel 300MBKiller.cpp
	$(CC) -arch ppc $(CFLAGS) $(LDFLAGS) -o 300MBKiller.ppc 300MBKiller.cpp
	lipo -create 300MBKiller.intel 300MBKiller.ppc -output 300MBKiller

clean:
	rm -f 300MBKiller 300MBKiller.ppc 300MBKiller.intel libPhoneInteraction.dylib
