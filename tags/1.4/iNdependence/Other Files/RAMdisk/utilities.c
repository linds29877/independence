#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ftw.h>

#include <sys/stat.h>
#include <sys/utsname.h>

#include <CoreFoundation/CoreFoundation.h>

#include "utilities.h"


void sig_chld_ignore(int signal)
{
	return;
}

void sig_chld_waitpid(int signal)
{
	while(waitpid(-1, 0, WNOHANG) > 0);
}

void cmd_system(char * argv[])
{
	pid_t fork_pid;
	
	signal(SIGCHLD, &sig_chld_ignore);
	if((fork_pid = fork()) != 0)
	{
		while(waitpid(fork_pid, NULL, WNOHANG) <= 0)
			usleep(300);
	} else {
		execv(argv[0], argv);
		exit(0);
	}

	signal(SIGCHLD, &sig_chld_waitpid);
}

int isIpod() {
	struct utsname u;
	uname(&u);
	if(strncmp("iPod", u.machine, 4) == 0) {
		return 1;
	} else {
		return 0;
	}
}

int isIphone() {
	struct utsname u;
	uname(&u);
	if(strncmp("iPhone", u.machine, 6) == 0) {
		return 1;
	} else {
		return 0;
	}
}

char* firmwareVersion() {
	CFPropertyListRef propertyList;
	CFStringRef errorString;
	CFURLRef url;
	CFDataRef resourceData;
	Boolean status;
	SInt32 errorCode;
	char* version;

	url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR("/System/Library/CoreServices/SystemVersion.plist"), kCFURLPOSIXPathStyle, false);

	status = CFURLCreateDataAndPropertiesFromResource(
			kCFAllocatorDefault,
			url,
			&resourceData,
			NULL,
			NULL,
			&errorCode);

	propertyList = CFPropertyListCreateFromXMLData( kCFAllocatorDefault,
							resourceData,
							kCFPropertyListImmutable,
							&errorString);

	CFRelease(url);
	CFRelease(resourceData);

	version = strdup(CFStringGetCStringPtr(CFDictionaryGetValue(propertyList, CFSTR("ProductVersion")), CFStringGetSystemEncoding()));

	CFRelease(propertyList);

	return version;
}

const char* deviceName() {
	if(isIpod())
		return "iPod";
	else if(isIphone())
		return "iPhone";
	else
		return "unknown device";
}

int fileExists(const char* fileName) {
	struct stat status;
	if(stat(fileName, &status) == 0) {
		return 1;
	} else {
		return 0;
	}
}

unsigned int fileSize(const char* fileName) {
	struct stat status;
	if(stat(fileName, &status) == 0) {
		return status.st_size;
	} else {
		return 0;
	}

}

void fileCopySimple(const char* orig, const char* dest) {
        size_t read;
        char buffer[4096];
        FILE* fOrig;
        FILE* fDest;

	fOrig = fopen(orig, "rb");

	if (fOrig != NULL) {
		fDest = fopen(dest, "wb");

	        while (!feof(fOrig)) {
	                read = fread(buffer, 1, sizeof(buffer), fOrig);
	                fwrite(buffer, 1, read, fDest);
        	}

	        fclose(fDest);
        	fclose(fOrig);
	}

}

int recursiveDelete(const char *path, const struct stat *status, int flag, struct FTW *buf)
{
	switch (flag) {
		case FTW_DNR:
		case FTW_DP:
			rmdir(path);
			break;
		case FTW_F:
		case FTW_NS:
		case FTW_SL:
		case FTW_SLN:
			unlink(path);
			break;
	}

	return 0;
}

void deleteDir(const char *path) {
	nftw(path, recursiveDelete, 5, FTW_DEPTH);
}


char* activationState() {
	CFPropertyListRef propertyList;
	CFStringRef errorString;
	CFURLRef url;
	CFDataRef resourceData;
	Boolean status;
	SInt32 errorCode;
	char* activationState;
	
	url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR("/var/root/Library/Lockdown/data_ark.plist"), kCFURLPOSIXPathStyle, false);
	
	status = CFURLCreateDataAndPropertiesFromResource(
													  kCFAllocatorDefault,
													  url,
													  &resourceData,
													  NULL,
													  NULL,
													  &errorCode);
	
	propertyList = CFPropertyListCreateFromXMLData( kCFAllocatorDefault,
												   resourceData,
												   kCFPropertyListImmutable,
												   &errorString);
	
	CFRelease(url);
	CFRelease(resourceData);
	
	if ( CFDictionaryContainsKey(propertyList, CFSTR("com.apple.mobile.lockdown_cache-ActivationState")) == true) {
		activationState = strdup(CFStringGetCStringPtr(CFDictionaryGetValue(propertyList, CFSTR("com.apple.mobile.lockdown_cache-ActivationState")),  CFStringGetSystemEncoding()));
	} else {
		activationState = "Unactivated";
	}
		
	CFRelease(propertyList);
	
	return activationState;
}
