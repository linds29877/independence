/*
 *  PIVersion.cpp
 *  libPhoneInteraction
 *
 *  Created by The Operator on 31/08/07.
 *  Copyright 2007 The Operator. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 2 for more details 
 */

#include "PIVersion.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>


bool ConvertCStringToPIVersion(const char *versionStr, PIVersion *version)
{
	version->major = 0;
	version->minor = 0;
	version->point = 0;

	int versionStrlen = strlen(versionStr);
	int left = versionStrlen;

	if (left == 0) {
		return false;
	}
	
	// get the major version
	char *ptr = strstr(versionStr, ".");
	
	if (ptr == NULL) {
		long major = strtol(versionStr, NULL, 10);

		if ( (errno == ERANGE) || (errno == EINVAL) ) {
			return false;
		}
		
		version->major = (int)major;
		return true;
	}
	else if (ptr > versionStr) {
		int len = ptr - versionStr;
		char substr[len+1];
		strncpy(substr, versionStr, len);
		substr[len] = 0;
		
		long major = strtol(substr, NULL, 10);
		
		if ( (errno == ERANGE) || (errno == EINVAL) ) {
			return false;
		}
		
		version->major = (int)major;
	}

	ptr++;
	left = versionStrlen - (ptr - versionStr);

	if (left <= 0) {
		return false;
	}

	// get the minor version
	char *ptr2 = strstr(ptr, ".");

	if (ptr2 == NULL) {
		long minor = strtol(ptr, NULL, 10);
		
		if ( (errno == ERANGE) || (errno == EINVAL) ) {
			return false;
		}
		
		version->minor = (int)minor;
		return true;
	}
	else if (ptr2 > ptr) {
		int len = ptr2 - ptr;
		char substr[len+1];
		strncpy(substr, ptr, len);
		substr[len] = 0;
		
		long minor = strtol(substr, NULL, 10);

		if ( (errno == ERANGE) || (errno == EINVAL) ) {
			return false;
		}
		
		version->minor = (int)minor;
	}

	ptr = ptr2 + 1;
	left = versionStrlen - (ptr - versionStr);

	if (left <= 0) {
		return false;
	}

	// get the point release
	ptr2 = strstr(ptr, ".");
	
	if (ptr2 == NULL) {
		long point = strtol(ptr, NULL, 10);
		
		if ( (errno == ERANGE) || (errno == EINVAL) ) {
			return false;
		}
		
		version->point = (int)point;
		return true;
	}
	else if (ptr2 > ptr) {
		int len = ptr2 - ptr;
		char substr[len+1];
		strncpy(substr, ptr, len);
		substr[len] = 0;
		
		long point = strtol(substr, NULL, 10);
		
		if ( (errno == ERANGE) || (errno == EINVAL) ) {
			return false;
		}
		
		version->point = (int)point;
	}
	
	return true;
}

bool ConvertCFStringToPIVersion(CFStringRef versionStr, PIVersion *version, FILE *fp)
{
	version->major = 0;
	version->minor = 0;
	version->point = 0;

	CFIndex len = CFStringGetLength(versionStr);
	
	if (len < 1) {

		if (fp != NULL) {
			fwrite("(DEBUG) len < 1\n", 1, 16, fp);
		}

		return false;
	}
	
	// get the major version
	CFRange dot = CFStringFind(versionStr, CFSTR("."), 0);
	CFRange sub;
	
	if (dot.location == kCFNotFound) {

		if (fp != NULL) {
			fwrite("(DEBUG) No dot found\n", 1, 21, fp);
		}

		SInt32 major = CFStringGetIntValue(versionStr);

		if ( (major == INT_MAX) || (major == INT_MIN) ) {

			if (fp != NULL) {
				fwrite("(DEBUG) major is invalid\n", 1, 25, fp);
			}

			return false;
		}

		if (fp != NULL) {
			char buf[20];
			sprintf(buf, "(DEBUG) major = %d\n", (int)major);
			fwrite(buf, 1, strlen(buf), fp);
		}

		version->major = (int)major;
		return true;
	}
	else if (dot.location > 0) {
		sub.location = 0;
		sub.length = dot.location;
		CFStringRef majorStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);

		if (fp != NULL) {
			char subinfo[50];
			sprintf(subinfo, "(DEBUG) sub.location = %d, sub.length = %d\n", sub.location, sub.length);
			fwrite(subinfo, 1, strlen(subinfo), fp);
		}

		if (majorStr == NULL) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) majorStr is NULL\n", 1, 25, fp);
			}
			
			return false;
		}

		// DEBUG
		if (fp != NULL) {
			CFIndex len = CFStringGetLength(majorStr);
			char *buf = (char*)malloc(len+1);
			CFStringGetCString(majorStr, buf, len+1, kCFStringEncodingASCII);
			fwrite("(DEBUG) majorString: ", 1,  21, fp);
			fwrite(buf, 1, len, fp);
			fwrite("\n", 1, 1, fp);
			free(buf);
		}
		// DEBUG

		SInt32 major = CFStringGetIntValue(majorStr);
		CFRelease(majorStr);
		
		if ( (major == INT_MAX) || (major == INT_MIN) ) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) major is invalid 2\n", 1, 27, fp);
			}
			
			return false;
		}
		
		if (fp != NULL) {
			char buf[20];
			sprintf(buf, "(DEBUG) major = %d\n", (int)major);
			fwrite(buf, 1, strlen(buf), fp);
		}

		version->major = (int)major;
	}
	
	sub.location = dot.location + dot.length;
	sub.length = len - (dot.location + dot.length);
	
	if (sub.length < 1) {
		return true;
	}
	
	// get the minor version
	if (!CFStringFindWithOptions(versionStr, CFSTR("."), sub, 0, &dot)) {
		CFStringRef minorStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);
		
		if (fp != NULL) {
			char subinfo[50];
			sprintf(subinfo, "(DEBUG) sub.location = %d, sub.length = %d\n", sub.location, sub.length);
			fwrite(subinfo, 1, strlen(subinfo), fp);
		}
		
		if (minorStr == NULL) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) minorStr is NULL\n", 1, 25, fp);
			}
			
			return false;
		}
		
		// DEBUG
		if (fp != NULL) {
			CFIndex len = CFStringGetLength(minorStr);
			char *buf = (char*)malloc(len+1);
			CFStringGetCString(minorStr, buf, len+1, kCFStringEncodingASCII);
			fwrite("(DEBUG) minorString: ", 1,  21, fp);
			fwrite(buf, 1, len, fp);
			fwrite("\n", 1, 1, fp);
			free(buf);
		}
		// DEBUG

		SInt32 minor = CFStringGetIntValue(minorStr);
		CFRelease(minorStr);
		
		if ( (minor == INT_MAX) || (minor == INT_MIN) ) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) minor is invalid\n", 1, 25, fp);
			}
			
			return false;
		}
		
		if (fp != NULL) {
			char buf[20];
			sprintf(buf, "(DEBUG) minor = %d\n", (int)minor);
			fwrite(buf, 1, strlen(buf), fp);
		}

		version->minor = (int)minor;
		return true;
	}
	else if (dot.location > sub.location) {
		sub.length = dot.location - sub.location;
		CFStringRef minorStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);
		
		if (fp != NULL) {
			char subinfo[50];
			sprintf(subinfo, "(DEBUG) sub.location = %d, sub.length = %d\n", sub.location, sub.length);
			fwrite(subinfo, 1, strlen(subinfo), fp);
		}

		if (minorStr == NULL) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) minorStr is NULL\n", 1, 25, fp);
			}
			
			return false;
		}
		
		// DEBUG
		if (fp != NULL) {
			CFIndex len = CFStringGetLength(minorStr);
			char *buf = (char*)malloc(len+1);
			CFStringGetCString(minorStr, buf, len+1, kCFStringEncodingASCII);
			fwrite("(DEBUG) minorString: ", 1,  21, fp);
			fwrite(buf, 1, len, fp);
			fwrite("\n", 1, 1, fp);
			free(buf);
		}
		// DEBUG

		SInt32 minor = CFStringGetIntValue(minorStr);
		CFRelease(minorStr);
		
		if ( (minor == INT_MAX) || (minor == INT_MIN) ) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) minor is invalid 2\n", 1, 27, fp);
			}
			
			return false;
		}
		
		if (fp != NULL) {
			char buf[20];
			sprintf(buf, "(DEBUG) minor = %d\n", (int)minor);
			fwrite(buf, 1, strlen(buf), fp);
		}

		version->minor = (int)minor;
	}
	
	sub.location = dot.location + dot.length;
	sub.length = len - (dot.location + dot.length);
	
	if (sub.length < 1) {
		return true;
	}
	
	// get the point release
	if (!CFStringFindWithOptions(versionStr, CFSTR("."), sub, 0, &dot)) {
		CFStringRef pointStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);
		
		if (fp != NULL) {
			char subinfo[50];
			sprintf(subinfo, "(DEBUG) sub.location = %d, sub.length = %d\n", sub.location, sub.length);
			fwrite(subinfo, 1, strlen(subinfo), fp);
		}
		
		if (pointStr == NULL) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) pointStr is NULL\n", 1, 25, fp);
			}
			
			return false;
		}
		
		// DEBUG
		if (fp != NULL) {
			CFIndex len = CFStringGetLength(pointStr);
			char *buf = (char*)malloc(len+1);
			CFStringGetCString(pointStr, buf, len+1, kCFStringEncodingASCII);
			fwrite("(DEBUG) pointString: ", 1,  21, fp);
			fwrite(buf, 1, len, fp);
			fwrite("\n", 1, 1, fp);
			free(buf);
		}
		// DEBUG

		SInt32 point = CFStringGetIntValue(pointStr);
		CFRelease(pointStr);
		
		if ( (point == INT_MAX) || (point == INT_MIN) ) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) point is invalid\n", 1, 25, fp);
			}
			
			return false;
		}
		
		if (fp != NULL) {
			char buf[20];
			sprintf(buf, "(DEBUG) point = %d\n", (int)point);
			fwrite(buf, 1, strlen(buf), fp);
		}

		version->point = (int)point;
	}
	else if (dot.location > sub.location) {
		sub.length = dot.location - sub.location;
		CFStringRef pointStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);
		
		if (fp != NULL) {
			char subinfo[50];
			sprintf(subinfo, "(DEBUG) sub.location = %d, sub.length = %d\n", sub.location, sub.length);
			fwrite(subinfo, 1, strlen(subinfo), fp);
		}

		if (pointStr == NULL) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) pointStr is NULL\n", 1, 25, fp);
			}
			
			return false;
		}
		
		// DEBUG
		if (fp != NULL) {
			CFIndex len = CFStringGetLength(pointStr);
			char *buf = (char*)malloc(len+1);
			CFStringGetCString(pointStr, buf, len+1, kCFStringEncodingASCII);
			fwrite("(DEBUG) pointString: ", 1,  21, fp);
			fwrite(buf, 1, len, fp);
			fwrite("\n", 1, 1, fp);
			free(buf);
		}
		// DEBUG

		SInt32 point = CFStringGetIntValue(pointStr);
		CFRelease(pointStr);
		
		if ( (point == INT_MAX) || (point == INT_MIN) ) {
			
			if (fp != NULL) {
				fwrite("(DEBUG) point is invalid 2\n", 1, 27, fp);
			}
			
			return false;
		}
		
		if (fp != NULL) {
			char buf[20];
			sprintf(buf, "(DEBUG) point = %d\n", (int)point);
			fwrite(buf, 1, strlen(buf), fp);
		}
		
		version->point = (int)point;
	}

	return true;
}
