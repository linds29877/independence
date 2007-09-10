#include <CoreFoundation/CoreFoundation.h>
#include "CFCompatibility.h"


int main (int argc, const char *argv[])
{
	
    if (argc < 2) {
		printf("Usage: %s <plist file>\n", argv[0]);
		exit(-1);
    }
	
    CFDictionaryRef plistDict1 = PICreateDictionaryFromPlistFile(argv[1]);

    if (plistDict1 == NULL) {
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
    CFDictionaryRef plistDict2 = (CFDictionaryRef)CFPropertyListCreateFromStream(kCFAllocatorDefault, stream, 0, kCFPropertyListMutableContainersAndLeaves, &format, &errString);

    CFReadStreamClose(stream);
    CFRelease(stream);

	printf("Dictionary created using PICreateDictionaryFromPlistFile:\n");
    CFShow(plistDict1);
	printf("\n\nDictionary created using CFPropertyListCreateFromStream:\n");
	CFShow(plistDict2);

	CFRelease(plistDict1);
	CFRelease(plistDict2);
    exit(0);
}
