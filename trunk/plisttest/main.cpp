#include <CoreFoundation/CoreFoundation.h>
#include "CFCompatibility.h"


int main (int argc, const char *argv[])
{
	
    if (argc < 2) {
		printf("Usage: %s <plist file>\n", argv[0]);
		exit(-1);
    }
	
    CFDictionaryRef activationDict1 = PICreateDictionaryFromPlistFile(argv[1]);
	
    if (activationDict1 == NULL) {
		printf("Error in PICreateDictionaryFromPlistFile\n");
		exit(-1);
    }
	
    CFStringRef fileString = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], kCFStringEncodingUTF8);
    CFStringRef errString;
    CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, fileString, kCFURLPOSIXPathStyle, false);
    CFReadStreamRef stream= CFReadStreamCreateWithFile(kCFAllocatorDefault, url);
    Boolean opened = CFReadStreamOpen(stream);
	
    if (opened == FALSE) {
		CFRelease(fileString);
		CFRelease(url);
		CFRelease(stream);
		exit(-1);
    }
	
    CFPropertyListFormat format;
    CFDictionaryRef activationDict2 = (CFDictionaryRef)CFPropertyListCreateFromStream(kCFAllocatorDefault, stream, 0, kCFPropertyListMutableContainersAndLeaves, &format, &errString);
	
    CFReadStreamClose(stream);
    CFRelease(stream);

	printf("Activation dict from PICreateDictionaryFromPlistFile:\n");
    CFShow(activationDict1);
	printf("\n\nActivation dict from CFPropertyListCreateFromStream:\n");
	CFShow(activationDict2);

    exit(0);
}
