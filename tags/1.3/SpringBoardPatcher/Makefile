CC=g++
CFLAGS=-I../Public\ Headers
LDFLAGS=-L. -lPhoneInteraction -framework CoreFoundation

SpringBoardPatcher: SpringBoardPatcher.cpp
	cp ../Public\ Libraries/Release/libPhoneInteraction.dylib .
	$(CC) -arch i386 $(CFLAGS) $(LDFLAGS) -o SpringBoardPatcher.intel SpringBoardPatcher.cpp
	$(CC) -arch ppc $(CFLAGS) $(LDFLAGS) -o SpringBoardPatcher.ppc SpringBoardPatcher.cpp
	lipo -create SpringBoardPatcher.intel SpringBoardPatcher.ppc -output SpringBoardPatcher

clean:
	rm -f SpringBoardPatcher SpringBoardPatcher.ppc SpringBoardPatcher.intel libPhoneInteraction
