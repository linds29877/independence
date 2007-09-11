#ifdef WIN32
#include <CoreFoundation.h>
#include <stdio.h>
#include <windows.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "CFCompatibility.h"


int main (int argc, const char **argv)
{
	
    if (argc < 3) {
		printf("Usage: %s <plist file> <new file>\n", argv[0]);
		return -1;
    }
	
    CFDictionaryRef plistDict1 = PICreateDictionaryFromPlistFile(argv[1]);

    if (plistDict1 == NULL) {
		printf("Error in PICreateDictionaryFromPlistFile\n");
		return -1;
    }

#ifdef __APPLE__
    CFStringRef fileString = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], kCFStringEncodingUTF8);
    CFStringRef errString;
    CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, fileString, kCFURLPOSIXPathStyle, false);
    CFReadStreamRef stream= CFReadStreamCreateWithFile(kCFAllocatorDefault, url);
    Boolean opened = CFReadStreamOpen(stream);
	
    if (opened == FALSE) {
		CFRelease(fileString);
		CFRelease(url);
		CFRelease(stream);
		return -1;
    }
	
    CFPropertyListFormat format;
    CFDictionaryRef plistDict2 = (CFDictionaryRef)CFPropertyListCreateFromStream(kCFAllocatorDefault, stream, 0, kCFPropertyListMutableContainersAndLeaves, &format, &errString);

    CFReadStreamClose(stream);
    CFRelease(stream);
#endif

	printf("Dictionary created using PICreateDictionaryFromPlistFile:\n");
    CFShow(plistDict1);

#ifdef __APPLE__
	printf("\n\nDictionary created using CFPropertyListCreateFromStream:\n");
	CFShow(plistDict2);
	CFRelease(plistDict2);
#endif

	if (!PICreatePlistFileFromDictionary(plistDict1, argv[2])) {
		printf("Error creating plist file from dictionary\n");
		CFRelease(plistDict1);
		return -1;
	}

 	CFRelease(plistDict1);
    return 0;
}
