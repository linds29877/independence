/*
 *  PhoneInteraction.cpp
 *  libPhoneInteraction
 *
 *  Created by The Operator on 20/07/07.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>

#if defined(WIN32)
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>

#define sleep(x) Sleep((x)*1000)
#define S_IRGRP 0
#define S_IXGRP 0
#define S_IROTH 0
#define S_IXOTH 0

#endif

#include <string.h>
#include <unistd.h>

#include "PhoneInteraction/PhoneInteraction.h"
#include "PhoneInteraction/UtilityFunctions.h"
#include "PhoneInteraction/CFCompatibility.h"
#include "MobileDevice/MobileDevice.h"


using namespace std;

typedef unsigned int (*t_AMRUSBInterfaceReadPipe)(unsigned int readwrite_pipe, unsigned
												  int read_pipe, void *data, unsigned int *len) __attribute__ ((regparm(2)));
typedef unsigned int (*t_AMRUSBInterfaceWritePipe)(unsigned int readwrite_pipe, unsigned
												   int write_pipe, void *data, unsigned int len) __attribute__ ((regparm(2)));
typedef unsigned int (*t_performOperation)(struct am_restore_device *rdev,
                                           CFDictionaryRef op) __attribute__ ((regparm(2)));
typedef unsigned int (*t_sendFileToDevice)(struct am_recovery_device *rdev,
										   CFStringRef file) __attribute__ ((regparm(2)));
typedef int (*t_socketForPort)(struct am_restore_device *rdev, unsigned int port) __attribute__ ((regparm(2)));
typedef unsigned int (*t_sendCommandToDevice)(struct am_recovery_device *rdev,
                                              CFStringRef cmd) __attribute__ ((regparm(2)));


static PhoneInteraction *g_phoneInteraction = NULL;
static char *g_symlinks[] =
{
	"/etc",
	"/mach",
	"/private/etc/resolv.conf",
	"/private/var/db/localtime",
	"/System/Library/Frameworks/IOKit.framework/IOKit",
	"/System/Library/Frameworks/IOKit.framework/Resources",
	"/System/Library/Frameworks/IOKit.framework/Versions/Current",
	"/System/Library/Frameworks/MultitouchSupport.framework/MultitouchSupport",
	"/System/Library/Frameworks/MultitouchSupport.framework/Resources",
	"/System/Library/Frameworks/MultitouchSupport.framework/Versions/Current",
	"/System/Library/Frameworks/OfficeImport.framework/OfficeImport",
	"/System/Library/Frameworks/OfficeImport.framework/Resources",
	"/System/Library/Frameworks/OfficeImport.framework/Versions/Current",
	"/System/Library/Frameworks/System.framework/System",
	"/System/Library/Frameworks/System.framework/Resources",
	"/System/Library/Frameworks/System.framework/Versions/Current",
	"/System/Library/Frameworks/System.framework/Versions/B/System",
	"/tmp",
	"/usr/lib/libbz2.1.0.dylib",
	"/usr/lib/libbz2.dylib",
	"/usr/lib/libc.dylib",
	"/usr/lib/libcharset.1.0.0.dylib",
	"/usr/lib/libcharset.dylib",
	"/usr/lib/libcrypto.dylib",
	"/usr/lib/libcurses.dylib",
	"/usr/lib/libdbm.dylib",
	"/usr/lib/libdl.dylib",
	"/usr/lib/libdns_sd.dylib",
	"/usr/lib/libedit.dylib", 
	"/usr/lib/libform.dylib",
	"/usr/lib/libiconv.2.2.0.dylib",
	"/usr/lib/libiconv.dylib",
	"/usr/lib/libicucore.dylib",
	"/usr/lib/libinfo.dylib",
	"/usr/lib/libIOKit.A.dylib",
	"/usr/lib/libIOKit.dylib",
	"/usr/lib/libipsec.dylib",
	"/usr/lib/libkvm.dylib",
	"/usr/lib/libm.dylib",
	"/usr/lib/libncurses.dylib",
	"/usr/lib/libobjc.dylib",
	"/usr/lib/libpoll.dylib",
	"/usr/lib/libpthread.dylib",
	"/usr/lib/librpcsvc.dylib",
	"/usr/lib/libsqlite3.0.8.6.dylib",
	"/usr/lib/libsqlite3.dylib",
	"/usr/lib/libssl.dylib",
	"/usr/lib/libstdc++.6.dylib",
	"/usr/lib/libSystem.dylib",
	"/usr/lib/libtidy.dylib",
	"/usr/lib/libxml2.dylib",
	"/usr/lib/libz.1.1.3.dylib",
	"/usr/lib/libz.1.dylib",
	"/usr/lib/libz.dylib",
	"/var"
};
static char *g_symlinkOriginals[] =
{
	"/private/etc",
	"/mach_kernel",
	"/private/var/run/resolv.conf",
	"/usr/share/zoneinfo/US/Pacific",
	"/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit",
	"/System/Library/Frameworks/IOKit.framework/Versions/A/Resources",
	"/System/Library/Frameworks/IOKit.framework/Versions/A",
	"/System/Library/Frameworks/MultitouchSupport.framework/Versions/A/MultitouchSupport",
	"/System/Library/Frameworks/MultitouchSupport.framework/Versions/A/Resources",
	"/System/Library/Frameworks/MultitouchSupport.framework/Versions/A",
	"/System/Library/Frameworks/OfficeImport.framework/Versions/A/OfficeImport",
	"/System/Library/Frameworks/OfficeImport.framework/Versions/A/Resources",
	"/System/Library/Frameworks/OfficeImport.framework/Versions/A",
	"/usr/lib/libSystem.B.dylib",
	"/System/Library/Frameworks/System.framework/Versions/B/Resources",
	"/System/Library/Frameworks/System.framework/Versions/B",
	"/usr/lib/libSystem.B.dylib",
	"/var/tmp", 
	"/usr/lib/libbz2.1.0.3.dylib",
	"/usr/lib/libbz2.1.0.3.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libcharset.1.dylib",
	"/usr/lib/libcharset.1.dylib",
	"/usr/lib/libcrypto.0.9.7.dylib",
	"/usr/lib/libncurses.5.4.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libedit.2.dylib",
	"/usr/lib/libform.dylib",
	"/usr/lib/libiconv.2.dylib",
	"/usr/lib/libiconv.2.dylib",
	"/usr/lib/libicucore.A.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit",
	"/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit",
	"/usr/lib/libipsec.A.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libncurses.5.4.dylib",
	"/usr/lib/libobjc.A.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libsqlite3.0.dylib",
	"/usr/lib/libsqlite3.0.dylib",
	"/usr/lib/libssl.0.9.7.dylib",
	"/usr/lib/libstdc++.6.0.4.dylib",
	"/usr/lib/libSystem.B.dylib",
	"/usr/lib/libtidy.A.dylib",
	"/usr/lib/libxml2.2.dylib",
	"/usr/lib/libz.1.2.3.dylib",
	"/usr/lib/libz.1.2.3.dylib",
	"/usr/lib/libz.1.2.3.dylib",
	"/private/var"
};

static int g_numSymlinks = 55;

static int g_recoveryAttempts = 0;

static t_socketForPort g_socketForPort;
static t_performOperation g_performOperation;
static t_sendCommandToDevice g_sendCommandToDevice;
static t_sendFileToDevice g_sendFileToDevice;

// wrapper functions for private MobileDevice library calls

int PI_socketForPort(am_restore_device *rdev, unsigned int portnum)
{
	int retval = 0;

#if defined (WIN32)
	asm("push %1\n\t"
		"push %3\n\t"
		"call *%0\n\t"
		"movl %%eax, %2"
		:
		:"m"(g_socketForPort), "g"(portnum), "m"(retval), "m"(rdev)
		:);
#elif defined (__APPLE__)
    retval = g_socketForPort(rdev, portnum);
#else
	retval = 1;
	printf("socketForPort not implemented on your platform\n");
#endif

	return retval;
}

int PI_performOperation(am_restore_device *rdev, CFDictionaryRef cfdr)
{
	int retval = 0;

#if defined (WIN32)
	asm("movl %2, %%esi\n\t"
		"movl %3, %%ebx\n\t"
		"call *%0\n\t"
		"movl %%eax, %1"
		:
		:"m"(g_performOperation), "m"(retval), "m"(rdev), "m"(cfdr)
		:);
#elif defined (__APPLE__)
	retval = g_performOperation(rdev, cfdr);
#else
	retval = 1;
	printf("performOperation not implemented on your platform\n");
#endif

	return retval;      
}

int PI_sendCommandToDevice(am_recovery_device *rdev, CFStringRef cfs)
{
	int retval = 0;

#if defined (WIN32)
	asm("movl %3, %%esi\n\t"
		"push %1\n\t"
		"call *%0\n\tmovl %%eax, %2"
		:
		:"m"(g_sendCommandToDevice),  "m"(cfs), "m"(retval), "m"(rdev)
		:);
#elif defined (__APPLE__)
	retval = g_sendCommandToDevice(rdev, cfs);
#else
	retval = 1;
	printf("sendCommandToDevice not implemented on your platform\n");
#endif

    return retval;
}

int PI_sendFileToDevice(am_recovery_device *rdev, CFStringRef filename)
{
	int retval = 0;

#if defined (WIN32)
	asm("movl %3, %%ecx\n\t"
		"push %1\n\t"
		"call *%0\n\t"
		"movl %%eax, %2"
		:
		:"m"(g_sendFileToDevice),  "m"(filename), "m"(retval), "m"(rdev)
		:);
#elif defined (__APPLE__)
	retval = g_sendFileToDevice(rdev, filename);
#else
	retval = 1;
	printf("sendFileToDevice not implemented on your platform\n");
#endif

	return retval;
}

void deviceNotificationCallback(am_device_notification_callback_info *info)
{
#ifdef DEBUG
	CFShow(CFSTR("deviceNotificationCallback"));
#endif
	g_phoneInteraction->m_iPhone = info->dev;

	if (info->msg == ADNCI_MSG_CONNECTED) {

		if (g_phoneInteraction->m_switchingToRestoreMode) {
			g_phoneInteraction->m_switchingToRestoreMode = false;
			g_phoneInteraction->restoreModeStarted();
		}
		else {
			g_phoneInteraction->connectToPhone();

			if (g_phoneInteraction->m_recoveryOccurred) {
				g_phoneInteraction->m_recoveryOccurred = false;

				if (g_phoneInteraction->m_finishingJailbreak) {
					g_phoneInteraction->jailbreakFinished();
				}
				else if (g_phoneInteraction->m_returningToJail) {
					g_phoneInteraction->returnToJailFinished();
				}

			}

		}

	}
	else if (info->msg == ADNCI_MSG_DISCONNECTED) {

		if (g_phoneInteraction->m_inRestoreMode) {
			g_phoneInteraction->restoreModeFinished();
		}
		else {
			g_phoneInteraction->setConnected(false);
		}

	}
#ifdef DEBUG
	else {
		char msg[25];
		snprintf(msg, 25, "info->msg = %d\n", info->msg);
		CFStringRef cfsr = CFStringCreateWithCString(kCFAllocatorDefault, msg, kCFStringEncodingUTF8);

		if (cfsr) {
			CFShow(cfsr);
			CFRelease(cfsr);
		}

	}
#endif

}

void dfuConnectNotificationCallback(am_recovery_device *dev)
{
#ifdef DEBUG
	CFShow(CFSTR("dfuConnectNotificationCallback"));
#endif
}

void dfuDisconnectNotificationCallback(am_recovery_device *dev)
{
#ifdef DEBUG
	CFShow(CFSTR("dfuDisconnectNotificationCallback"));
#endif
}

void recoveryProgressCallback(unsigned int progress_number, unsigned int opcode)
{
#ifdef DEBUG
	CFShow(CFSTR("recoveryProgressCallback"));
#endif

	if (opcode == 9) {
		PI_sendCommandToDevice(g_phoneInteraction->m_recoveryDevice, CFSTR("setenv boot-args rd=md0 -progress"));

		// alternative boot args for restore mode boot spew
		//PI_sendCommandToDevice(g_phoneInteraction->m_recoveryDevice, CFSTR("setenv boot-args rd=md0 -v"));
	}
	else {
#ifdef DEBUG
		char msg[50];
		snprintf(msg, 50, "Progress number: %d, opcode: %d\n", progress_number, opcode);
		CFStringRef cfsr = CFStringCreateWithCString(kCFAllocatorDefault, msg, kCFStringEncodingUTF8);
		
		if (cfsr) {
			CFShow(cfsr);
			CFRelease(cfsr);
		}
#endif

	}

}

void recoveryConnectNotificationCallback(am_recovery_device *dev)
{
#ifdef DEBUG
	CFShow(CFSTR("recoveryConnectNotificationCallback"));
#endif

	if (g_phoneInteraction->m_waitingForRecovery) {
		g_phoneInteraction->recoveryModeStarted(dev);
	}
	else if ( g_phoneInteraction->m_finishingJailbreak || g_phoneInteraction->m_returningToJail ) {

		if (!g_phoneInteraction->m_recoveryOccurred) {
			g_phoneInteraction->m_recoveryOccurred = true;
			g_phoneInteraction->exitRecoveryMode(dev);
		}

	}
	else if (g_recoveryAttempts++ == 0) {
		// try once to save them from recovery mode
		g_phoneInteraction->exitRecoveryMode(dev);
	}

}

void recoveryDisconnectNotificationCallback(am_recovery_device *dev)
{
#ifdef DEBUG
	CFShow(CFSTR("recoveryDisconnectNotificationCallback"));
#endif

	if (!g_phoneInteraction->m_finishingJailbreak) {
		g_phoneInteraction->recoveryModeFinished(dev);
	}

}

PhoneInteraction::PhoneInteraction(void (*statusFunc)(const char*, bool),
								   void (*notifyFunc)(int, const char*))
{
	m_statusFunc = statusFunc;
	m_notifyFunc = notifyFunc;
	m_iPhone = NULL;
	m_recoveryDevice = NULL;
	m_connected = false;
	m_inRecoveryMode = false;
	m_switchingToRestoreMode = false;
	m_inRestoreMode = false;
	m_jailbroken = false;
	m_finishingJailbreak = false;
	m_returningToJail = false;
	m_hAFC = NULL;
	m_firmwarePath = NULL;
	m_waitingForRecovery = false;
	m_privateFunctionsSetup = false;
	m_recoveryOccurred = false;
	m_iTunesVersion.major = 0;
	m_iTunesVersion.minor = 0;
	m_iTunesVersion.point = 0;

	if (determineiTunesVersion()) {
		setupPrivateFunctions();
	}

#ifdef DEBUG
	printf("iTunes version: %d.%d.%d\n", m_iTunesVersion.major, m_iTunesVersion.minor,
		   m_iTunesVersion.point);
#endif

	if (m_statusFunc) {
		(*m_statusFunc)("Disconnected: waiting for iPhone", true);
	}
	
}

PhoneInteraction::~PhoneInteraction()
{
	disconnectFromPhone();

	if (m_firmwarePath != NULL) {
		free(m_firmwarePath);
		m_firmwarePath = NULL;
	}

}

PhoneInteraction* PhoneInteraction::getInstance(void (*statusFunc)(const char*, bool),
												void (*notifyFunc)(int, const char*))
{

	if (g_phoneInteraction == NULL) {
		g_phoneInteraction = new PhoneInteraction(statusFunc, notifyFunc);
		g_phoneInteraction->subscribeToNotifications();
	}

	return g_phoneInteraction;
}

#if defined(WIN32)
bool PhoneInteraction::determineiTunesVersion()
{
    HINSTANCE hDllInst = LoadLibrary("iTunesMobileDevice.dll");

	if (!hDllInst) {
		return false;
	}

    DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hDllInst, TEXT("DllGetVersion"));

    if (pDllGetVersion) {    // DLL supports version retrieval function
        DLLVERSIONINFO dvi;

        ZeroMemory(&dvi, sizeof(dvi));
        dvi.cbSize = sizeof(dvi);
        HRESULT hr = (*pDllGetVersion)(&dvi);

        if(SUCCEEDED(hr)) { // Finally, the version is at our hands
            m_iTunesVersion.major = (int)dvi.dwMajorVersion;
            m_iTunesVersion.minor = (int)dvi.dwMinorVersion;
            m_iTunesVersion.point = (int)dvi.dwBuildNumber;
        }
        else {  // GetProcAddress failed, the DLL cannot tell its version
            FreeLibrary(hDllInst);
            return false;
        }

        FreeLibrary(hDllInst);
    }
    else {
        FreeLibrary(hDllInst);

        TCHAR szPathStr[MAX_PATH];

       if (GetWindowsDirectory(szPathStr, MAX_PATH) == 0) {
            return false;
        }

        lstrcat(szPathStr, _T("\\..\\Program Files\\Common Files\\Apple\\Mobile Device Support\\bin\\iTunesMobileDevice.dll"));

        VS_FIXEDFILEINFO   *ffInfo;
        UINT    uVersionLen;
        DWORD   dwVerHnd=0;			            // An 'ignored' parameter, always '0'
        DWORD dwVerInfoSize = GetFileVersionInfoSize(szPathStr, &dwVerHnd);

        if (!dwVerInfoSize) return false;

        LPSTR lpstrVffInfo = (LPSTR)malloc(dwVerInfoSize);

        if (!GetFileVersionInfo(szPathStr, dwVerHnd, dwVerInfoSize, lpstrVffInfo)) {
            free(lpstrVffInfo);
            return false;
        }
    
        if (!VerQueryValue(lpstrVffInfo, _T("\\"),
                           (LPVOID*)&ffInfo, (UINT*)&uVersionLen)) {
            free(lpstrVffInfo);
            return false;
        }

        m_iTunesVersion.major = (int)((ffInfo->dwFileVersionMS >> 16) & 0xffff);
        m_iTunesVersion.minor = (int)(ffInfo->dwFileVersionMS & 0xffff);
        m_iTunesVersion.point = (int)((ffInfo->dwFileVersionLS >> 16) & 0xffff);
    }

    return true;
}
#else
bool PhoneInteraction::determineiTunesVersion()
{
	CFDictionaryRef iTunesVersionDict = NULL;
	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
												 CFSTR("/Applications/iTunes.app/Contents/version.plist"),
												 kCFURLPOSIXPathStyle, false);
	CFStringRef errString;
	CFReadStreamRef stream= CFReadStreamCreateWithFile(kCFAllocatorDefault, url);
	
	Boolean opened = CFReadStreamOpen(stream);
	
	if (opened == FALSE) {
		CFRelease(url);
		CFRelease(stream);
		return false;
	}
	
	CFPropertyListFormat format;
	iTunesVersionDict = (CFDictionaryRef)CFPropertyListCreateFromStream(kCFAllocatorDefault,
																		stream, 0,
																		kCFPropertyListMutableContainersAndLeaves,
																		&format, &errString);
	CFReadStreamClose(stream);
	CFRelease(stream);
	CFRelease(url);
	
	if (errString != NULL) {
		return false;
	}
	
	if (iTunesVersionDict == NULL) {
		return false;
	}

	CFStringRef versionStr = (CFStringRef)CFDictionaryGetValue(iTunesVersionDict, CFSTR("CFBundleVersion"));

	if (versionStr == NULL) {
		CFRelease(iTunesVersionDict);
		return false;
	}

	CFIndex len = CFStringGetLength(versionStr);

	if (len < 1) {
		CFRelease(iTunesVersionDict);
		return false;
	}

	// get the major version
	CFRange dot = CFStringFind(versionStr, CFSTR("."), 0);
	CFRange sub;

	if (dot.location == kCFNotFound) {
		SInt32 major = CFStringGetIntValue(versionStr);

		if ( (major == INT_MAX) || (major == INT_MIN) ) {
			CFRelease(iTunesVersionDict);
			return false;
		}

		m_iTunesVersion.major = (int)major;
		CFRelease(iTunesVersionDict);
		return true;
	}
	else if (dot.location > 0) {
		sub.location = 0;
		sub.length = dot.location;
		CFStringRef majorStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);

		if (majorStr == NULL) {
			CFRelease(iTunesVersionDict);
			return false;
		}

		SInt32 major = CFStringGetIntValue(majorStr);
		CFRelease(majorStr);

		if ( (major == INT_MAX) || (major == INT_MIN) ) {
			CFRelease(iTunesVersionDict);
			return false;
		}
		
		m_iTunesVersion.major = (int)major;
	}

	sub.location = dot.location + dot.length;
	sub.length = len - (dot.location + dot.length);

	if (sub.length < 1) {
		CFRelease(iTunesVersionDict);
		return true;
	}

	// get the minor version
	if (!CFStringFindWithOptions(versionStr, CFSTR("."), sub, 0, &dot)) {
		CFStringRef minorStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);

		if (minorStr == NULL) {
			CFRelease(iTunesVersionDict);
			return false;
		}

		SInt32 minor = CFStringGetIntValue(minorStr);
		CFRelease(minorStr);
		
		if ( (minor == INT_MAX) || (minor == INT_MIN) ) {
			CFRelease(iTunesVersionDict);
			return false;
		}
		
		m_iTunesVersion.minor = (int)minor;
		CFRelease(iTunesVersionDict);
		return true;
	}
	else if (dot.location > sub.location) {
		sub.length = dot.location - sub.location;
		CFStringRef minorStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);
		
		if (minorStr == NULL) {
			CFRelease(iTunesVersionDict);
			return false;
		}
		
		SInt32 minor = CFStringGetIntValue(minorStr);
		CFRelease(minorStr);
		
		if ( (minor == INT_MAX) || (minor == INT_MIN) ) {
			CFRelease(iTunesVersionDict);
			return false;
		}
		
		m_iTunesVersion.minor = (int)minor;
	}
	
	sub.location = dot.location + dot.length;
	sub.length = len - (dot.location + dot.length);
	
	if (sub.length < 1) {
		CFRelease(iTunesVersionDict);
		return true;
	}

	// get the point release
	if (!CFStringFindWithOptions(versionStr, CFSTR("."), sub, 0, &dot)) {
		CFStringRef pointStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);

		if (pointStr == NULL) {
			CFRelease(iTunesVersionDict);
			return false;
		}
		
		SInt32 point = CFStringGetIntValue(pointStr);
		CFRelease(pointStr);
		
		if ( (point == INT_MAX) || (point == INT_MIN) ) {
			CFRelease(iTunesVersionDict);
			return false;
		}
		
		m_iTunesVersion.point = (int)point;
	}
	else if (dot.location > sub.location) {
		sub.length = dot.location - sub.location;
		CFStringRef pointStr = CFStringCreateWithSubstring(kCFAllocatorDefault, versionStr,
														   sub);

		if (pointStr == NULL) {
			CFRelease(iTunesVersionDict);
			return false;
		}
		
		SInt32 point = CFStringGetIntValue(pointStr);
		CFRelease(pointStr);

		if ( (point == INT_MAX) || (point == INT_MIN) ) {
			CFRelease(iTunesVersionDict);
			return false;
		}
		
		m_iTunesVersion.point = (int)point;
	}
	
	CFRelease(iTunesVersionDict);
	return true;
}
#endif

#if defined(WIN32)
void PhoneInteraction::setupPrivateFunctions()
{
	HMODULE hGetProcIDDLL = GetModuleHandle("iTunesMobileDevice.dll");

	if (!hGetProcIDDLL) {
		(*m_notifyFunc)(NOTIFY_WIN32_INITIALIZATION_FAILED, "Error obtaining handle to iTunesMobileDevice.dll");
		return;
	}

	if ( (m_iTunesVersion.major < 7) || (m_iTunesVersion.major > 7) ) return;
	
	if ( (m_iTunesVersion.minor < 3) || (m_iTunesVersion.minor > 4) ) return;

	if (m_iTunesVersion.minor == 4) {
        g_sendCommandToDevice = (t_sendCommandToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x10008170);
        g_sendFileToDevice = (t_sendFileToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x100082F0);
        g_socketForPort = (t_socketForPort)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x10012F90);
        g_performOperation = (t_performOperation)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x10014040);
	}
	else {
		g_sendCommandToDevice= (t_sendCommandToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x10009290);
		g_sendFileToDevice= (t_sendFileToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x10009410);
		g_performOperation= (t_performOperation)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x100129C0);
		g_socketForPort= (t_socketForPort)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x10012830);
	}

	m_privateFunctionsSetup = true;
}
#elif defined(__APPLE__) && defined(__POWERPC__)
void PhoneInteraction::setupPrivateFunctions()
{

	if ( (m_iTunesVersion.major < 7) || (m_iTunesVersion.major > 7) ) return;
	
	if ( (m_iTunesVersion.minor < 3) || (m_iTunesVersion.minor > 4) ) return;
	
	if (m_iTunesVersion.minor == 4) {
		g_performOperation = (t_performOperation)0x3c3a0bc8;
		g_socketForPort = (t_socketForPort)0x3c3a051c;
		g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a5bb0;
		g_sendFileToDevice = (t_sendFileToDevice)0x3c3a5d10;
	}
	else {
		g_performOperation = (t_performOperation)0x3c3a0e14;
		g_socketForPort = (t_socketForPort)0x3c3a0644;
		g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a517c;
		g_sendFileToDevice = (t_sendFileToDevice)0x3c3a52dc;
	}

	m_privateFunctionsSetup = true;
}
#elif defined(__APPLE__)
void PhoneInteraction::setupPrivateFunctions()
{

	if ( (m_iTunesVersion.major < 7) || (m_iTunesVersion.major > 7) ) return;

	if ( (m_iTunesVersion.minor < 3) || (m_iTunesVersion.minor > 4) ) return;

	if (m_iTunesVersion.minor == 4) {
		g_performOperation = (t_performOperation)0x3c3a0599;
		g_socketForPort = (t_socketForPort)0x3c39ffa3;
		g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a597f;
		g_sendFileToDevice = (t_sendFileToDevice)0x3c3a5bcb;
	}
	else {
		g_performOperation = (t_performOperation)0x3c39fa4b;
		g_socketForPort = (t_socketForPort)0x3c39f36c;
		g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a3e3b;
		g_sendFileToDevice = (t_sendFileToDevice)0x3c3a4087;
	}

	m_privateFunctionsSetup = true;
}
#endif

bool PhoneInteraction::arePrivateFunctionsSetup()
{
	return m_privateFunctionsSetup;
}

void PhoneInteraction::subscribeToNotifications()
{

	if (!arePrivateFunctionsSetup()) {
		char msg[256];
		snprintf(msg, 256, "Unsupported version of iTunes is installed.\nDetected iTunes version is %d.%d.%d\n",
				 m_iTunesVersion.major, m_iTunesVersion.minor, m_iTunesVersion.point);
		(*m_notifyFunc)(NOTIFY_INITIALIZATION_FAILED, msg);
		return;
	}

	struct am_device_notification *notif;

	if (AMDeviceNotificationSubscribe(deviceNotificationCallback, 0, 0, 0, &notif)) {
		(*m_notifyFunc)(NOTIFY_INITIALIZATION_FAILED, "Error registering for callbacks from iPhone");
		return;
	}

	if (AMRestoreRegisterForDeviceNotifications(dfuConnectNotificationCallback,
												recoveryConnectNotificationCallback,
												dfuDisconnectNotificationCallback,
												recoveryDisconnectNotificationCallback,
												0, NULL)) {
		(*m_notifyFunc)(NOTIFY_INITIALIZATION_FAILED, "Error registering for callbacks from iPhone");
		return;
	}
	
	
	(*m_notifyFunc)(NOTIFY_INITIALIZATION_SUCCESS, "Notification subscription success!");
	return;
}

void PhoneInteraction::connectToPhone()
{

	if (isConnected()) return;

	if (m_iPhone == NULL) return;

	if (m_statusFunc) {
		(*m_statusFunc)("Connecting to iPhone...", true);
	}

	if ( AMDeviceConnect(m_iPhone) ) { 
		setConnected(false);
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Can't connect to phone.");
		return;
	}

	if ( !AMDeviceIsPaired(m_iPhone) ) {
		setConnected(false);
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Phone is not paired.  Make sure you run iTunes with your phone connected at least once.");
		return;
	}

	if ( AMDeviceValidatePairing(m_iPhone) ) {
		setConnected(false);
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Pairing is not valid.  Make sure your run iTunes with your phone connected at least once.");
		return;
	}

	if ( AMDeviceStartSession(m_iPhone) ) {
		setConnected(false);
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Error starting session.");
		return;
	}

	m_jailbroken = false;
	afc_connection *hAFC;

	if ( AMDeviceStartService(m_iPhone, AMSVC_AFC2, &hAFC, NULL) ) {

		if ( AMDeviceStartService(m_iPhone, AMSVC_AFC, &hAFC, NULL) ) {
			setConnected(false);
			(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Error connecting to AFC service.");
			return;
		}

	}
	else {
		// AFC2 is running.  We're jailbroken.
		m_jailbroken = true;
	}

	if (AFCConnectionOpen(hAFC, 0, &m_hAFC)) {
		setConnected(false);
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Error opening connection to AFC service");
		return;
	}

	if (!m_jailbroken) {
		// For older jailbreak compatibility: test to see if we can open a system
		// file.  If so, then we're jailbroken.
		afc_file_ref rAFC;

		if (!AFCFileRefOpen(m_hAFC, "/private/etc/master.passwd", 1, &rAFC)) {
			m_jailbroken = true;
			AFCFileRefClose(m_hAFC, rAFC);
		}

	}

	setConnected(true);
	(*m_notifyFunc)(NOTIFY_CONNECTION_SUCCESS, "Connection success!");
}

void PhoneInteraction::disconnectFromPhone()
{
	
	if (isConnected()) {
		AFCConnectionClose(m_hAFC);
		AMDeviceDisconnect(m_iPhone);
		AMDeviceStopSession(m_iPhone);
		m_jailbroken = false;
		m_iPhone = NULL;
		setConnected(false);
	}
	
}

void PhoneInteraction::setConnected(bool connected)
{
	m_connected = connected;

	if (m_connected) {
		(*m_notifyFunc)(NOTIFY_CONNECTED, "Connected");
	}
	else {
		(*m_notifyFunc)(NOTIFY_DISCONNECTED, "Disconnected");
	}

}

bool PhoneInteraction::isConnected()
{
	return m_connected;
}

bool PhoneInteraction::activate(const char* filename, const char *pemfile)
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Can't activate when no phone is connected.");
		return false;
	}

	if (isPhoneJailbroken()) {
		(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Can't activate when phone is jailbroken.");
		return false;
	}

	if ( (filename == NULL) && (pemfile == NULL) ) {
		(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Invalid parameters.");
		return false;
	}

	if (m_statusFunc) {
		(*m_statusFunc)("Activating...", true);
	}

	CFDictionaryRef activationRecord = NULL;

	if (filename == NULL) {

		// do everything for them
		CFStringRef cfdeviceid = AMDeviceCopyDeviceIdentifier(m_iPhone);

		if (cfdeviceid == NULL) {
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error obtaining the Device ID.");
			return false;
		}

		CFIndex cflen = CFStringGetLength(cfdeviceid);
		char *deviceid = (char*)malloc(cflen+1);

		if (CFStringGetCString(cfdeviceid, deviceid, cflen+1, kCFStringEncodingUTF8) == false) {
			free(deviceid);
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error obtaining the Device ID.");
			return false;
		}

		CFStringRef cfimei = AMDeviceCopyValue(m_iPhone, 0, CFSTR("InternationalMobileEquipmentIdentity"));

		if (cfimei == NULL) {
			free(deviceid);
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error obtaining the IMEI.");
			return false;
		}

		cflen = CFStringGetLength(cfimei);
		char *imei = (char*)malloc(cflen+1);

		if (CFStringGetCString(cfimei, imei, cflen+1, kCFStringEncodingUTF8) == false) {
			free(deviceid);
			free(imei);
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error obtaining the IMEI.");
			return false;
		}

		CFStringRef cficcid = AMDeviceCopyValue(m_iPhone, 0, CFSTR("IntegratedCircuitCardIdentity"));

		if (cficcid == NULL) {
			free(deviceid);
			free(imei);
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error obtaining the ICCID (ensure there is a SIM card in your phone).");
			return false;
		}

		cflen = CFStringGetLength(cficcid);
		char *iccid = (char*)malloc(cflen+1);

		if (CFStringGetCString(cficcid, iccid, cflen+1, kCFStringEncodingUTF8) == false) {
			free(deviceid);
			free(imei);
			free(iccid);
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error obtaining the ICCID.");
			return false;
		}
		
		if (!UtilityFunctions::generateActivationRecord(&activationRecord, pemfile,
														deviceid, imei, iccid)) {
			free(deviceid);
			free(imei);
			free(iccid);
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error generating activation record.");
			return false;
		}

		free(deviceid);
		free(imei);
		free(iccid);
	}
	else {
		CFDictionaryRef activationDict;

#if defined(__APPLE__)
		CFStringRef fileString = CFStringCreateWithCString(kCFAllocatorDefault, filename, kCFStringEncodingUTF8);
		CFStringRef errString;
		CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, fileString, kCFURLPOSIXPathStyle, false);
		CFReadStreamRef stream= CFReadStreamCreateWithFile(kCFAllocatorDefault, url);
	
		Boolean opened = CFReadStreamOpen(stream);
	
		if (opened == FALSE) {
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error creating dictionary from given plist file.");
			CFRelease(fileString);
			CFRelease(url);
			CFRelease(stream);
			return false;
		}
	
		CFPropertyListFormat format;
		activationDict = (CFDictionaryRef)CFPropertyListCreateFromStream(kCFAllocatorDefault,
																	 stream, 0,
																	 kCFPropertyListMutableContainersAndLeaves,
																	 &format, &errString);
		CFReadStreamClose(stream);
		CFRelease(stream);
		CFRelease(url);
		CFRelease(fileString);
	
		if (errString != NULL) {
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error creating dictionary from given plist file.");
			return false;
		}
	
#else
		// 0p: New CFCompatibility code (cross-platform) for reading plist files
		// (seems to work well)
		activationDict = PICreateDictionaryFromPlistFile(filename);
#endif

		if (activationDict == NULL) {
			(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error creating dictionary from given plist file.");
			return false;
		}

		activationRecord = (CFDictionaryRef)CFDictionaryGetValue(activationDict,
																 CFSTR("ActivationRecord"));

		if (activationRecord == NULL) {

			// it may be an activation record from a phone with version 1.0.1 firmware,
			// so check for the AccountToken in the original dictionary
			if (!CFDictionaryContainsKey(activationDict, CFSTR("AccountToken"))) {
				CFRelease(activationDict);
				(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error obtaining activation record from dictionary.");
				return false;
			}

			activationRecord = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, activationDict);

			if (activationRecord == NULL) {
				CFRelease(activationDict);
				(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, "Error obtaining activation record from dictionary.");
				return false;
			}

		}

		CFRelease(activationDict);
	}

	int retval;

	if ( (retval = AMDeviceActivate(m_iPhone, activationRecord)) ) {
		char resultCStr[50];
		snprintf(resultCStr, 50, "Activation failed with code %d.", retval);
		(*m_notifyFunc)(NOTIFY_ACTIVATION_FAILED, resultCStr);
		CFRelease(activationRecord);
		return false;
	}
	else {
		CFStringRef result = AMDeviceCopyValue(m_iPhone, 0, CFSTR("ActivationState"));
		CFMutableStringRef resultStr = CFStringCreateMutable(kCFAllocatorDefault, 0);
		CFStringAppend(resultStr, CFSTR("Activation succeeded."));

		if (result) {
			CFStringAppend(resultStr, CFSTR("\n\nActivation result: "));
			CFStringAppend(resultStr, result);
		}
		
		CFIndex length = CFStringGetLength(resultStr);
		char resultCStr[length+1];
		CFStringGetCString(resultStr, resultCStr, length+1, kCFStringEncodingUTF8);
		(*m_notifyFunc)(NOTIFY_ACTIVATION_SUCCESS, resultCStr);
		CFRelease(resultStr);
	}

	CFRelease(activationRecord);
	return true;
}

bool PhoneInteraction::deactivate()
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_DEACTIVATION_FAILED, "Can't deactivate when no phone is connected.");
		return false;
	}

	if (m_statusFunc) {
		(*m_statusFunc)("Deactivating...", true);
	}

	int retval;

	if ( (retval = AMDeviceDeactivate(m_iPhone)) ) {
		char resultCStr[50];
		snprintf(resultCStr, 50, "Deactivation failed with code %d.", retval);
		(*m_notifyFunc)(NOTIFY_DEACTIVATION_FAILED, resultCStr);
		return false;
	}
	else {
		CFStringRef result = AMDeviceCopyValue(m_iPhone, 0, CFSTR("ActivationState"));
		CFMutableStringRef resultStr = CFStringCreateMutable(kCFAllocatorDefault, 0);
		CFStringAppend(resultStr, CFSTR("Dectivation succeeded."));

		if (result) {
			CFStringAppend(resultStr, CFSTR("\n\nDeactivation result: "));
			CFStringAppend(resultStr, result);
		}
		
		CFIndex length = CFStringGetLength(resultStr);
		char resultCStr[length+1];
		CFStringGetCString(resultStr, resultCStr, length+1, kCFStringEncodingUTF8);
		(*m_notifyFunc)(NOTIFY_DEACTIVATION_SUCCESS, resultCStr);
		CFRelease(resultStr);
	}

	return true;
}

bool PhoneInteraction::isPhoneActivated()
{

	if (!isConnected()) {
		return false;
	}

	CFStringRef result = AMDeviceCopyValue(m_iPhone, 0, CFSTR("ActivationState"));

	if (result == NULL) {
		return false;
	}

	if ( (CFStringCompare(result, CFSTR("Activated"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) ||
		 (CFStringCompare(result, CFSTR("FactoryActivated"), kCFCompareCaseInsensitive) == kCFCompareEqualTo) ) {
		return true;
	}

	return false;
}

bool PhoneInteraction::putData(void *data, int len, char *dest, int failureMsg, int successMsg)
{
	
	if (!isConnected()) {
		
		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Can't write a file when no phone is connected.");
		}
		
		return false;
	}

	afc_file_ref rAFC;
	
	if (AFCFileRefOpen(m_hAFC, dest, 3, &rAFC)) {

		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error opening destination file for writing on phone.");
		}
		
		return false;
	}
	
	if (AFCFileRefWrite(m_hAFC, rAFC, data, len)) {
		AFCFileRefClose(m_hAFC, rAFC);
		
		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error writing destination file on phone.");
		}
		
		return false;
	}
	
	AFCFileRefClose(m_hAFC, rAFC);
	
	if (successMsg) {
		(*m_notifyFunc)(successMsg, "File successfully written to phone.");
	}
	
	return true;
}

bool PhoneInteraction::putFile(const char *src, char *dest, int failureMsg, int successMsg)
{

	if (!isConnected()) {

		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Can't write a file when no phone is connected.");
		}

		return false;
	}
	
	struct stat st;
	FILE* fp = fopen(src, "r");
	
	if (fp == NULL) {

		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error opening source file.");
		}

		return false;
	}
	
	if (stat(src, &st) < 0) {
		fclose(fp);

		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error getting source file size.");
		}

		return false;
	}
	
	void *data = malloc(st.st_size);
	memset(data, 0, st.st_size);

	off_t count = 0;

	while (count < st.st_size) {
		size_t retval = fread(data, 1, st.st_size - count, fp);
		
		if (retval == 0) break;
		
		count += retval;
	}
	
	fclose(fp);
	
	if (count < st.st_size) {
		free(data);

		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error reading source file.");
		}

		return false;
	}

	bool retval = putData(data, st.st_size, dest, failureMsg, successMsg);
	free(data);
	return retval;
}

bool PhoneInteraction::putFileRecursive(const char *filepath, char *destpath, int failureMsg, int successMsg)
{
	struct stat statbuf;
	
	if (stat(filepath, &statbuf) < 0) {

		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Can't get file info.");
		}

		return false;
	}
	
	if (S_ISREG(statbuf.st_mode)) {
		return putFile(filepath, destpath, failureMsg, successMsg);
	}
	else if (S_ISDIR(statbuf.st_mode)) {
		
		if (!createDirectory(destpath)) {
			
			if (failureMsg) {
				(*m_notifyFunc)(failureMsg, "Can't create directory on phone.");
			}
			
			return false;
		}
		
		DIR *dp;
		
		if ( (dp = opendir(filepath)) == NULL ) {
			
			if (failureMsg) {
				(*m_notifyFunc)(failureMsg, "Can't open source directory.");
			}
			
			return false;
		}
		
		char srcfile[PATH_MAX+1];
		int len = strlen(filepath);
		
		memset(srcfile, 0, PATH_MAX+1);
		strcpy(srcfile, filepath);
		
		if (srcfile[len-1] != '/') {
			strcat(srcfile, "/");
			len++;
		}
		
		char destfile[PATH_MAX+1];
		int len2 = strlen(destpath);
		
		memset(destfile, 0, PATH_MAX+1);
		strcpy(destfile, destpath);
		
		if (destfile[len2-1] != '/') {
			strcat(destfile, "/");
			len2++;
		}
		
		struct dirent *dirp;

		while ( (dirp = readdir(dp)) != NULL ) {
			
			if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) continue;
			
			strcpy(srcfile + len, dirp->d_name);
			strcpy(destfile + len2, dirp->d_name);
			
			if (!putFileRecursive(srcfile, destfile, failureMsg, successMsg)) {
				closedir(dp);
				return false;
			}
			
		}

		closedir(dp);
	}
	
	return true;
}

bool PhoneInteraction::putPEMOnPhone(const char *pemFile)
{
	return putFile(pemFile, "/System/Library/Lockdown/iPhoneActivation.pem", NOTIFY_PUTPEM_FAILED,
				   NOTIFY_PUTPEM_SUCCESS);
}

bool PhoneInteraction::putServicesFileOnPhone(const char *servicesFile)
{
	return putFile(servicesFile, "/System/Library/Lockdown/Services.plist", NOTIFY_PUTSERVICES_FAILED,
				   NOTIFY_PUTSERVICES_SUCCESS);
}

bool PhoneInteraction::putFstabFileOnPhone(const char *fstabFile)
{
	return putFile(fstabFile, "/etc/fstab", NOTIFY_PUTFSTAB_FAILED, NOTIFY_PUTFSTAB_SUCCESS);
}

bool PhoneInteraction::putRingtoneOnPhone(const char *ringtoneFile, bool bInSystemDir)
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Can't write a ringtone when no phone is connected.");
		return false;
	}

	struct afc_directory *dir;
	const char *ringtoneDir = "/var/root/Library/Ringtones";

	if (bInSystemDir) {
		ringtoneDir = "/Library/Ringtones";
	}

	if (AFCDirectoryOpen(m_hAFC, (char*)ringtoneDir, &dir)) {

		if (AFCDirectoryCreate(m_hAFC, (char*)ringtoneDir)) {
			(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error creating directory for ringtones.");
			return false;
		}

		if (AFCDirectoryOpen(m_hAFC, (char*)ringtoneDir, &dir)) {
			(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error opening directory for ringtones.");
			return false;
		}

	}

	AFCDirectoryClose(m_hAFC, dir);

	const char *ringtoneFilename = UtilityFunctions::getLastPathElement(ringtoneFile);
	char ringtonePath[PATH_MAX+1];
	strcpy(ringtonePath, ringtoneDir);
	strcat(ringtonePath, "/");
	strcat(ringtonePath, ringtoneFilename);

	if (!putFile(ringtoneFile, ringtonePath)) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error putting ringtone file on phone.");
		return false;
	}

	(*m_notifyFunc)(NOTIFY_PUTFILE_SUCCESS, "Successfully put ringtone on phone.");
	return true;
}

bool PhoneInteraction::putWallpaperOnPhone(const char *wallpaperFile, const char *wallpaperThumb,
										   bool bInSystemDir)
{
	struct afc_directory *dir;
	const char *wallpaperDir = "/var/root/Library/Wallpaper";

	if (bInSystemDir) {
		wallpaperDir = "/Library/Wallpaper";
	}

	if (AFCDirectoryOpen(m_hAFC, (char*)wallpaperDir, &dir)) {
		
		if (AFCDirectoryCreate(m_hAFC, (char*)wallpaperDir)) {
			(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error creating directory for Wallpaper.");
			return false;
		}
		
		if (AFCDirectoryOpen(m_hAFC, (char*)wallpaperDir, &dir)) {
			(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error opening directory for Wallpaper.");
			return false;
		}
		
	}
	
	AFCDirectoryClose(m_hAFC, dir);
	
	const char *wallpaperFilename = UtilityFunctions::getLastPathElement(wallpaperFile);

	// validate the main wallpaper filename (should not contain .thumbnail)
	if (strstr(wallpaperFilename, ".thumbnail") != NULL) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Main wallpaper filename cannot contain .thumbnail.  Please rename it.");
		return false;
	}

	char wallpaperPath[PATH_MAX+1];
	strcpy(wallpaperPath, wallpaperDir);
	strcat(wallpaperPath, "/");
	strcat(wallpaperPath, wallpaperFilename);

	if (!putFile(wallpaperFile, wallpaperPath)) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error putting wallpaper file on phone.");
		return false;
	}

	// 0p: Here things get a bit tricky.  Instead of blindly taking their wallpaper thumbnail
	// filename (which could be invalid), we generate a valid one from the wallpaper filename
	// and use that.
	int len = strlen(wallpaperFilename);
	char thumbnailFilename[len+11];
	int count = len-1;

	while (count >= 0) {

		if (wallpaperFilename[count] == '.') {
			strncpy(thumbnailFilename, wallpaperFilename, count);
			strcpy(thumbnailFilename + count, ".thumbnail");
			strcat(thumbnailFilename, wallpaperFilename + count);
			break;
		}

		count--;
	}

	if (count < 0) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error putting wallpaper file on phone.");
		return false;
	}

	char thumbnailPath[PATH_MAX+1];
	strcpy(thumbnailPath, wallpaperDir);
	strcat(thumbnailPath, "/");
	strcat(thumbnailPath, thumbnailFilename);

	if (!putFile(wallpaperThumb, thumbnailPath)) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error putting wallpaper thumbnail on phone.");
		return false;
	}
	
	(*m_notifyFunc)(NOTIFY_PUTFILE_SUCCESS, "Successfully put wallpaper on phone.");
	return true;
}

bool PhoneInteraction::putApplicationOnPhone(const char *applicationDir)
{
	const char *applicationFilename = UtilityFunctions::getLastPathElement(applicationDir);
	char applicationPath[PATH_MAX+1];
	strcpy(applicationPath, "/Applications/");
	strcat(applicationPath, applicationFilename);

	if (!putFileRecursive(applicationDir, applicationPath)) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error putting application on phone.");
		return false;
	}

	(*m_notifyFunc)(NOTIFY_PUTFILE_SUCCESS, "Successfully put application on phone.");
	return true;
}

bool PhoneInteraction::removeRingtone(const char *ringtoneFilename, bool bInSystemDir)
{
	const char *ringtoneDir = "/var/root/Library/Ringtones/";
	
	if (bInSystemDir) {
		ringtoneDir = "/Library/Ringtones/";
	}
	
	char ringtonePath[PATH_MAX+1];
	strcpy(ringtonePath, ringtoneDir);
	strcat(ringtonePath, ringtoneFilename);

	return removePath(ringtonePath);
}

bool PhoneInteraction::removeWallpaper(const char *wallpaperFilename, bool bInSystemDir)
{
	const char *wallpaperDir = "/var/root/Library/Wallpaper/";
	
	if (bInSystemDir) {
		wallpaperDir = "/Library/Wallpaper/";
	}
	
	char wallpaperPath[PATH_MAX+1];
	strcpy(wallpaperPath, wallpaperDir);
	strcat(wallpaperPath, wallpaperFilename);

	return removePath(wallpaperPath);
}

bool PhoneInteraction::removeApplication(const char *applicationFilename)
{
	char applicationPath[PATH_MAX+1];
	strcpy(applicationPath, "/Applications/");
	strcat(applicationPath, applicationFilename);

	return removePath(applicationPath);
}

bool PhoneInteraction::ringtoneExists(const char *ringtoneFile, bool bInSystemDir)
{
	const char *ringtoneDir = "/var/root/Library/Ringtones/";
	
	if (bInSystemDir) {
		ringtoneDir = "/Library/Ringtones/";
	}

	char filepath[PATH_MAX+1];
	memset(filepath, 0, PATH_MAX+1);
	strcpy(filepath, ringtoneDir);
	strcat(filepath, ringtoneFile);

	return fileExists(filepath);
}

bool PhoneInteraction::wallpaperExists(const char *wallpaperFile, bool bInSystemDir)
{
	const char *wallpaperDir = "/var/root/Library/Wallpaper/";
	
	if (bInSystemDir) {
		wallpaperDir = "/Library/Wallpaper/";
	}

	char filepath[PATH_MAX+1];
	memset(filepath, 0, PATH_MAX+1);
	strcpy(filepath, wallpaperDir);
	strcat(filepath, wallpaperFile);

	return fileExists(filepath);
}

bool PhoneInteraction::applicationExists(const char *applicationName)
{
	const char *applicationDir = "/Applications/";
	char filepath[PATH_MAX+1];

	memset(filepath, 0, PATH_MAX+1);
	strcpy(filepath, applicationDir);
	strcat(filepath, applicationName);

	return fileExists(filepath);
}

bool PhoneInteraction::writeDataToFile(void *buf, int size, const char *file,
									   int failureMsg, int successMsg)
{
    FILE *fp = fopen(file, "w");

    if (fp == NULL) {
		
		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error opening file for writing.");
		}
		
		return false;
    }
	
	off_t count = 0;
	
	while (count < size) {
		size_t retval = fwrite(buf, 1, size - count, fp);
		
		if (retval == 0) break;
		
		count += retval;
	}
	
    fclose(fp);
	
	if (count < size) {
		
		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error writing to file.");
		}
		
		return false;
	}
	
	return true;
}

int PhoneInteraction::getFileSize(const char *path)
{
    char *key, *val;
    unsigned int size;
    struct afc_dictionary *info;
	
    if (AFCFileInfoOpen(m_hAFC, (char*)path, &info)) {
        return -1;
	}
	
    while (1) {
        AFCKeyValueRead(info, &key, &val);
        if (!key || !val)
            break;
		
        if (!strcmp(key, "st_size")) {
            sscanf(val, "%u", &size);
            AFCKeyValueClose(info);
            return (int)size;
        }

    }

    AFCKeyValueClose(info);
    return -1;
}

bool PhoneInteraction::getFileData(void **buf, int *size, const char *file,
								   int failureMsg, int successMsg)
{

	if (!isConnected()) {
		
		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Can't read a file when no phone is connected.");
		}
		
		return false;
	}

	*size = getFileSize(file);

	if (*size == -1) {
		
		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error getting phone file size.");
		}
		
		return false;
	}
	else if (*size == 0) {
		return true;
	}

	*buf = malloc(*size);

	afc_file_ref rAFC;
	
	if (AFCFileRefOpen(m_hAFC, (char*)file, 1, &rAFC)) {

		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error getting phone file reference.");
		}
		
		return false;
	}

	unsigned int us_size = *size;

    if (AFCFileRefRead(m_hAFC, rAFC, (unsigned char*)(*buf), &us_size)) {
		AFCFileRefClose(m_hAFC, rAFC);
		free(*buf);
		
		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error reading from phone file.");
		}
		
		return false;
    }
	
	if (AFCFileRefClose(m_hAFC, rAFC)) {
		free(*buf);

		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Error closing phone file.");
		}
		
		return false;
	}

	return true;
}

bool PhoneInteraction::getFile(const char *src, const char *dest, int failureMsg,
							   int successMsg)
{

	if (!isConnected()) {
		
		if (failureMsg) {
			(*m_notifyFunc)(failureMsg, "Can't read a file when no phone is connected.");
		}
		
		return false;
	}

	void *buf;
	int size;

	if (!getFileData(&buf, &size, src, failureMsg, successMsg)) {
		return false;
	}

	if (size == -1) {
		return false;
	}
	else if (size == 0) {
		FILE *fp = fopen(dest, "w");
		fclose(fp);
		return true;
	}

	bool retval = writeDataToFile(buf, size, dest, failureMsg, successMsg);

	free(buf);

	return retval;
}

bool PhoneInteraction::getActivationFile(const char *dest)
{

	if (getFile("/var/root/Library/Lockdown/activation_record.plist", dest, 0, 0)) {
		(*m_notifyFunc)(NOTIFY_GET_ACTIVATION_SUCCESS, "Successfully saved activation file.");
		return true;
	}

	afc_directory *dir;

	if (AFCDirectoryOpen(m_hAFC, "/var/root/Library/Lockdown/activation_records", &dir)) {
		(*m_notifyFunc)(NOTIFY_GET_ACTIVATION_FAILED, "Can't find activation record.");
		return false;
	}

	char *filename = NULL;

	while (1) {

		if (AFCDirectoryRead(m_hAFC, dir, &filename)) {
			AFCDirectoryClose(m_hAFC, dir);
			(*m_notifyFunc)(NOTIFY_GET_ACTIVATION_FAILED, "Can't find activation record.");
			return false;
		}

		if (filename == NULL) {
			break;
		}

		int len = strlen(filename);

		if (len < 6) {
			continue;
		}

		if (!strcmp(filename + (len-6), ".plist")) {
			break;
		}

	}

	if (filename == NULL) {
		AFCDirectoryClose(m_hAFC, dir);
		(*m_notifyFunc)(NOTIFY_GET_ACTIVATION_FAILED, "Can't find activation record.");
		return false;
	}

	char filepath[PATH_MAX+1];
	memset(filepath, 0, PATH_MAX+1);
	strcpy(filepath, "/var/root/Library/Lockdown/activation_records/");
	strcat(filepath, filename);

	if (!getFile(filepath, dest, NOTIFY_GET_ACTIVATION_FAILED, 0)) {
		AFCDirectoryClose(m_hAFC, dir);
		return false;
	}

	AFCDirectoryClose(m_hAFC, dir);
	(*m_notifyFunc)(NOTIFY_GET_ACTIVATION_SUCCESS, "Successfully saved activation file.");
	return true;
}

bool PhoneInteraction::copyPhoneFilesystem(const char *dirpath, const char *dest,
										   bool ignoreUserFiles)
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_FSCOPY_FAILED, "Can't copy when no phone is connected.");
		return false;
	}

	if (m_statusFunc) {
		(*m_statusFunc)("Copying phone filesystem...", true);
	}

	bool retval = copyFilesystemRecursive(m_hAFC, dirpath, dest, dest, ignoreUserFiles);

	if (retval) {
		(*m_notifyFunc)(NOTIFY_FSCOPY_SUCCESS, "Filesystem copy succeeded.");
	}

	return retval;
}

bool PhoneInteraction::copyFilesystemRecursive(afc_connection *conn, const char *dirpath,
											   const char *dest, const char *basepath,
											   bool ignoreUserFiles)
{
	afc_directory *dir;

	if (AFCDirectoryOpen(conn, (char*)dirpath, &dir)) {
		(*m_notifyFunc)(NOTIFY_FSCOPY_FAILED, "Error opening directory on phone.");
		return false;
	}

	char phoneWorkingDir[PATH_MAX+1];
	char computerWorkingDir[PATH_MAX+1];
	char *fileName = NULL;

	strcpy(phoneWorkingDir, dirpath);
	strcpy(computerWorkingDir, dest);

	while (1) {
		AFCDirectoryRead(conn, dir, &fileName);

		if (fileName == NULL) break;

		if (strcmp(fileName, ".") && strcmp(fileName, "..") && strcmp(fileName, "/etc")) {
			char filePath[PATH_MAX+1];
			int len = strlen(phoneWorkingDir);
			struct afc_dictionary *info;
			char *key, *val;

			strcpy(filePath, phoneWorkingDir);

			if (filePath[len-1] != '/') {
				strcat(filePath, "/");
			}

			strcat(filePath, fileName);

#ifdef DEBUG
			printf("reading phone path: %s\n", filePath);
#endif

			bool isSymlink = false;

			// TODO: Figure out how to get symlinks on Windows
#if defined(__APPLE__)
			// this is an ugly hack because I can't tell if a file/dir is
			// a symlink or not.  so we compare it against a known list
			for (int i = 0; i < g_numSymlinks; i++) {

				if (!strcmp(g_symlinks[i], filePath)) {
					char origPath[PATH_MAX+1];
					strcpy(origPath, basepath);
					strcat(origPath, g_symlinkOriginals[i]);

					char linkPath[PATH_MAX+1];
					strcpy(linkPath, basepath);
					strcat(linkPath, g_symlinks[i]);

#ifdef DEBUG
					printf("creating symlink from %s to %s\n", linkPath, origPath);
#endif

					if (symlink(origPath, linkPath)) {
						AFCDirectoryClose(conn, dir);
						(*m_notifyFunc)(NOTIFY_FSCOPY_FAILED, "Error creating symlink.");
						return false;
					}

					isSymlink = true;
					break;
				}

			}
#endif

			if (isSymlink) continue;

			if (AFCFileInfoOpen(conn, filePath, &info)) {
				AFCDirectoryClose(conn, dir);
				(*m_notifyFunc)(NOTIFY_FSCOPY_FAILED, "Error opening file info.");
				return false;
			}

			while (1) {
				AFCKeyValueRead(info, &key, &val);

				if (!key || !val)
					break;

				//printf("key = %s, val = %s\n", key, val);

				if (strcmp(key, "st_ifmt")) continue;

				if (!strcmp(val, "S_IFDIR")) {
					bool ignore = false;

					if (ignoreUserFiles) {
						
						if (!strncmp(filePath, "/private/var/root/Media", 23)) {
							ignore = true;
						}
						else if (!strncmp(filePath, "/private/var/root/Library", 25)) {
							ignore = true;
							char newPath[PATH_MAX+1];
							len = strlen(computerWorkingDir);
							strcpy(newPath, computerWorkingDir);
							
							if (newPath[len-1] != '/') {
								strcat(newPath, "/");
							}
							
							strcat(newPath, "/private/var/root/Library");
#if defined(WIN32)
							mkdir(newPath);
#else
							mkdir(newPath, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
#endif

							memset(newPath, 0, PATH_MAX+1);
							strcpy(newPath, computerWorkingDir);

							if (newPath[len-1] != '/') {
								strcat(newPath, "/");
							}
							
							strcat(newPath, "/private/var/root/Library/Preferences");
#if defined(WIN32)
							mkdir(newPath);
#else
							mkdir(newPath, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
#endif
						}
						
					}

					if (!ignore) {
						char newPath[PATH_MAX+1];
						len = strlen(computerWorkingDir);
						strcpy(newPath, computerWorkingDir);
						
						if (newPath[len-1] != '/') {
							strcat(newPath, "/");
						}
						
						strcat(newPath, fileName);
#if defined(WIN32)
						mkdir(newPath);
#else
						mkdir(newPath, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
#endif
						
						if (!copyFilesystemRecursive(conn, filePath, newPath, basepath,
													 ignoreUserFiles)) {
							AFCDirectoryClose(conn, dir);
							return false;
						}
						
					}
#ifdef DEBUG
					else {
						printf("ignoring user file\n");
					}
#endif

				}
				else if (!strcmp(val, "S_IFREG")) {
					bool ignore = false;

					if (ignoreUserFiles) {

						if (!strcmp(filePath, "/com.apple.itunes.lock_sync")) {
							ignore = true;
						}

					}

					if (!ignore) {
						char newPath[PATH_MAX+1];
						len = strlen(computerWorkingDir);
						strcpy(newPath, computerWorkingDir);
						
						if (newPath[len-1] != '/') {
							strcat(newPath, "/");
						}
						
						strcat(newPath, fileName);
						
						if (!getFile(filePath, newPath, NOTIFY_FSCOPY_FAILED,
									 NOTIFY_FSCOPY_SUCCESS)) {
							AFCDirectoryClose(conn, dir);
							return false;
						}
						
					}

				}
#ifdef DEBUG
				else if (!strcmp(val, "S_IFCHR") || !strcmp(val, "S_IFBLK") || !strcmp(val, "S_IFSOCK")) {
					// special files, don't do anything
					printf("ignoring device or socket file\n");
				}
				else {
					printf("unknown file type: %s\n", val);
				}
#endif

			}
			
			AFCKeyValueClose(info);
		}

	}

	AFCDirectoryClose(conn, dir);
	return true;
}

int PhoneInteraction::numFilesInDirectory(const char *path)
{
	
	if (!isConnected()) {
		return -1;
	}
	
	afc_directory *dir;
	
	if (AFCDirectoryOpen(m_hAFC, (char*)path, &dir)) {
		return -1;
	}
	
	int count = 0;
	char *fileName = NULL;
	
	while (1) {
		AFCDirectoryRead(m_hAFC, dir, &fileName);
		if (fileName == NULL) break;
		count++;
	}
	
	AFCDirectoryClose(m_hAFC, dir);
	return count;
}

bool PhoneInteraction::fileExists(const char *path)
{
	
	if (!isConnected()) {
		return false;
	}
	
	struct afc_dictionary *info;
	
	if (AFCFileInfoOpen(m_hAFC, (char*)path, &info)) {
		return false;
	}

	return true;
}

bool PhoneInteraction::directoryFileList(const char *path, char ***list, int *length)
{

	if (!isConnected()) {
		return false;
	}

	int count = numFilesInDirectory(path);

	if (count == -1) {
		return false;
	}

	*list = (char**)malloc(count * sizeof(char*));
	afc_directory *dir;

	if (AFCDirectoryOpen(m_hAFC, (char*)path, &dir)) {
		free(*list);
		return false;
	}

	char *filename;
	count = 0;

	while (1) {
		AFCDirectoryRead(m_hAFC, dir, &filename);

		if (filename == NULL) break;

		(*list)[count] = (char*)malloc((strlen(filename) + 1) * sizeof(char));
		strcpy((*list)[count], filename);
		count++;
	}

	*length = count;
	return true;
}

bool PhoneInteraction::isDirectory(const char *path)
{
	
	if (!isConnected()) {
		return false;
	}

	struct afc_dictionary *info;

	if (AFCFileInfoOpen(m_hAFC, (char*)path, &info)) {
		return false;
	}
	
	char *key, *val;

	while (1) {
		AFCKeyValueRead(info, &key, &val);
		
		if (!key || !val) break;

		if (!strcmp(val, "S_IFDIR")) {
			AFCKeyValueClose(info);
			return true;
		}

	}

	AFCKeyValueClose(info);
	return false;
}

bool PhoneInteraction::createDirectory(const char *dir)
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_CREATE_DIR_FAILED, "Can't create a directory when no phone is connected.");
		return false;
	}

	if (AFCDirectoryCreate(m_hAFC, (char*)dir)) {
		(*m_notifyFunc)(NOTIFY_CREATE_DIR_FAILED, "Error creating directory.");
		return false;
	}
	
	(*m_notifyFunc)(NOTIFY_CREATE_DIR_SUCCESS, "Directory successfully created on phone.");
	return true;
}

bool PhoneInteraction::removePath(const char *path)
{
	
	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_REMOVE_PATH_FAILED, "Can't remove a file/directory when no phone is connected.");
		return false;
	}

	if (!removePathRecursive(path)) {
		(*m_notifyFunc)(NOTIFY_REMOVE_PATH_FAILED, "Error removing file/directory.");
		return false;
	}

	(*m_notifyFunc)(NOTIFY_REMOVE_PATH_SUCCESS, "Successfully removed file/directory.");
	return true;
}

bool PhoneInteraction::removePathRecursive(const char *path)
{

	if (!isDirectory(path)) {

		if (AFCRemovePath(m_hAFC, (char*)path)) {
			return false;
		}

	}
	else {

		afc_directory *dir;
	
		if (AFCDirectoryOpen(m_hAFC, (char*)path, &dir)) {
			return false;
		}

		char *filename = NULL;
		char filepath[PATH_MAX+1];
		int len = strlen(path);

		memset(filepath, 0, PATH_MAX+1);
		strcpy(filepath, path);

		if (filepath[len-1] != '/') {
			strcat(filepath, "/");
			len++;
		}

		while (1) {
			AFCDirectoryRead(m_hAFC, dir, &filename);

			if (filename == NULL) break;

			if (!strcmp(filename, ".") || !strcmp(filename, "..")) continue;

			strcpy(filepath + len, filename);

			if (!removePathRecursive(filepath)) {
				AFCDirectoryClose(m_hAFC, dir);
				return false;
			}

		}

		AFCDirectoryClose(m_hAFC, dir);

		if (AFCRemovePath(m_hAFC, (char*)path)) {
			return false;
		}

	}

	return true;
}

void PhoneInteraction::performJailbreak(const char *firmwarePath, const char *modifiedFstabPath,
										const char *modifiedServicesPath)
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Can't perform a jailbreak when no phone is connected.");
		return;
	}

	if (m_statusFunc) {
		(*m_statusFunc)("Performing jailbreak...", true);
	}

	if (!putFile(modifiedFstabPath, "fstab", 0, 0)) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error writing modified fstab to phone.");
		return;
	}

	if (!putFile(modifiedServicesPath, "Services.plist", 0, 0)) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error writing modified Services.plist to phone.");
		return;
	}

	if (m_firmwarePath != NULL) {
		free(m_firmwarePath);
	}

	size_t len = strlen(firmwarePath);

	if (firmwarePath[len-1] != '/') {
		m_firmwarePath = (char*)malloc(len + 2);
		strncpy(m_firmwarePath, firmwarePath, len);
		m_firmwarePath[len] = '/';
		m_firmwarePath[len+1] = 0;
	}
	else {
		m_firmwarePath = (char*)malloc(len + 1);
		strncpy(m_firmwarePath, firmwarePath, len);
		m_firmwarePath[len] = 0;
	}

	if (AMDeviceEnterRecovery(m_iPhone)) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error entering recovery mode.");
		return;
	}

	m_recoveryOccurred = false;
	m_waitingForRecovery = true;
	(*m_notifyFunc)(NOTIFY_JAILBREAK_RECOVERY_WAIT, "Waiting for jail break...");
}

void PhoneInteraction::returnToJail(const char *servicesFile, const char *fstabFile)
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Can't return to jail when no phone is connected.");
		return;
	}

	if (!putServicesFileOnPhone(servicesFile)) {
		(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error writing Services file to phone.");
		return;
	}
	
	if (!putFstabFileOnPhone(fstabFile)) {
		(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error writing fstab file to phone.");
		return;
	}

	if (AMDeviceEnterRecovery(m_iPhone)) {
		(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error entering recovery mode.");
		return;
	}
	
	m_recoveryOccurred = false;
	m_returningToJail = true;
	(*m_notifyFunc)(NOTIFY_JAILRETURN_RECOVERY_WAIT, "Waiting for return to jail...");
}

void PhoneInteraction::returnToJailFinished()
{
	m_returningToJail = false;
	(*m_notifyFunc)(NOTIFY_JAILRETURN_SUCCESS, "Return to jail succeeded!");
}

void PhoneInteraction::jailbreakFinished()
{
	m_finishingJailbreak = false;
	(*m_notifyFunc)(NOTIFY_JAILBREAK_SUCCESS, "Jailbreak succeeded!");
}

bool PhoneInteraction::isPhoneJailbroken()
{
	return m_jailbroken;
}

void PhoneInteraction::recoveryModeStarted(struct am_recovery_device *rdev)
{
	m_waitingForRecovery = false;
	m_inRecoveryMode = true;
	m_recoveryDevice = rdev;
	(*m_notifyFunc)(NOTIFY_JAILBREAK_RECOVERY_CONNECTED, "Recovery mode started");

#if 1

	// Good way to get into recovery mode as it doesn't require firmware filenames

	CFMutableDictionaryRef restoreOptions = AMRestoreCreateDefaultOptions(kCFAllocatorDefault);
	
	if (restoreOptions == NULL) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error creating restore options dictionary.");
		return;
	}
	
	CFStringRef cfFirmwarePath = CFStringCreateWithCString(kCFAllocatorDefault, m_firmwarePath,
														   kCFStringEncodingUTF8);
	
	if (cfFirmwarePath == NULL) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error creating path CFString.");
		return;
	}
	
    CFDictionarySetValue(restoreOptions, CFSTR("CreateFilesystemPartitions"), kCFBooleanFalse);
	CFDictionarySetValue(restoreOptions, CFSTR("RestoreBundlePath"), cfFirmwarePath);
    CFDictionarySetValue(restoreOptions, CFSTR("RestoreBootArgs"), CFSTR("rd=md0 -progress"));
	
	// alternative boot args for restore mode boot spew
	//CFDictionarySetValue(restoreOptions, CFSTR("RestoreBootArgs"), CFSTR("rd=md0 -v"));
	
	m_switchingToRestoreMode = true;
	
	if (AMRestorePerformRecoveryModeRestore(rdev, restoreOptions, (void*)recoveryProgressCallback, NULL)) {
		CFRelease(cfFirmwarePath);
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error switching to restore mode.");
		return;
	}
	
	CFRelease(cfFirmwarePath);
#else

	// Tried and true method using firmware filenames

	char ramdisk[PATH_MAX+1];
	char kerncache[PATH_MAX+1];
	unsigned int len = strlen(m_firmwarePath);

	memset(ramdisk, 0, PATH_MAX+1);
	memset(kerncache, 0, PATH_MAX+1);
	strcpy(ramdisk, m_firmwarePath);
	strcpy(kerncache, m_firmwarePath);

	if (ramdisk[len-1] != '/') {
		strcat(ramdisk, "/");
		strcat(kerncache, "/");
	}

	strcat(ramdisk, "694-5259-38.dmg");

	// check the firmware version
	struct stat st;

	if (stat(ramdisk, &st) == -1) {
		memset(ramdisk, 0, PATH_MAX+1);
		strcpy(ramdisk, m_firmwarePath);

		if (ramdisk[len-1] != '/') {
			strcat(ramdisk, "/");
		}

		// version 1.0.1 restore disk image
		strcat(ramdisk, "009-7662-6.dmg");

		if (stat(ramdisk, &st) == -1) {
			memset(ramdisk, 0, PATH_MAX+1);
			strcpy(ramdisk, m_firmwarePath);

			if (ramdisk[len-1] != '/') {
				strcat(ramdisk, "/");
			}

			// version 1.0.2 restore disk image
			strcat(ramdisk, "009-7698-4.dmg");

			if (stat(ramdisk, &st) == -1) {
				(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Unknown firmware version.");
				return;
			}
			else {
				strcat(kerncache, "kernelcache.release.s5l8900xrb");
			}

		}
		else {
			strcat(kerncache, "kernelcache.release.s5l8900xrb");
		}

	}
	else {
		strcat(kerncache, "kernelcache.restore.release.s5l8900xrb");
	}

	CFStringRef cfRamdisk = CFStringCreateWithCString(kCFAllocatorDefault, ramdisk,
													  kCFStringEncodingUTF8);

	if (cfRamdisk == NULL) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error creating path CFString.");
		return;
	}

	// send ramdisk file
	if (PI_sendFileToDevice(rdev, cfRamdisk)) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error sending ramdisk file to device.");
		return;
	}

	// load ramdisk
	if (PI_sendCommandToDevice(rdev, CFSTR("ramdisk"))) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error sending ramdisk command to device.");
		return;
	}

	CFStringRef cfKernCache = CFStringCreateWithCString(kCFAllocatorDefault, kerncache,
														kCFStringEncodingUTF8);

	if (cfKernCache == NULL) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error creating path CFString.");
		return;
	}

	// send kernel cache file
	if (PI_sendFileToDevice(rdev, cfKernCache)) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error sending kernel cache file to device.");
		return;
	}

	// set boot args
	//
	// alternative boot args for restore mode boot spew
	//if (PI_sendCommandToDevice(rdev, CFSTR("setenv boot-args rd=md0 -v"))) {
	if (PI_sendCommandToDevice(rdev, CFSTR("setenv boot-args rd=md0 -progress"))) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error setting boot args.");
		return;
	}

	// turn off autoboot
	if (PI_sendCommandToDevice(rdev, CFSTR("setenv auto-boot false"))) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error turning off auto-boot.");
		return;
	}
	
	m_switchingToRestoreMode = true;

	// boot kernel cache to get to restore mode
	if (PI_sendCommandToDevice(rdev, CFSTR("bootx"))) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error rebooting device.");
		return;
	}

#endif

}

void PhoneInteraction::recoveryModeFinished(am_recovery_device *dev)
{
	m_inRecoveryMode = false;
	m_recoveryDevice = NULL;
	setConnected(false);
	(*m_notifyFunc)(NOTIFY_JAILBREAK_RECOVERY_DISCONNECTED, "Recovery mode ended");
}

void PhoneInteraction::exitRecoveryMode(am_recovery_device *dev)
{

	// set device to auto boot
	if (PI_sendCommandToDevice(dev, CFSTR("setenv auto-boot true"))) {

		if (m_finishingJailbreak) {
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error turning on auto-boot.");
		}
		else if (m_returningToJail) {
			(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error turning on auto-boot.");
		}

		return;
	}

	// clear boot args
	if (PI_sendCommandToDevice(dev, CFSTR("setenv boot-args"))) {
		
		if (m_finishingJailbreak) {
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error setting boot args.");
		}
		else if (m_returningToJail) {
			(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error setting boot args.");
		}
		
		return;
	}

	// save the environment
	if (PI_sendCommandToDevice(dev, CFSTR("saveenv"))) {

		if (m_finishingJailbreak) {
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error saving environment.");
		}
		else if (m_returningToJail) {
			(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error saving environment.");
		}
		return;
	}

	// reboot into normal mode
	if (PI_sendCommandToDevice(dev, CFSTR("fsboot"))) {

		if (m_finishingJailbreak) {
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error trying to reboot into normal mode.");
		}
		else if (m_returningToJail) {
			(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error trying to reboot into normal mode.");
		}

		return;
	}

}

void PhoneInteraction::restoreModeStarted()
{
	m_inRestoreMode = true;
	(*m_notifyFunc)(NOTIFY_JAILBREAK_RESTORE_CONNECTED, "Restore mode started");

    m_restoreDevice = AMRestoreModeDeviceCreate(0, AMDeviceGetConnectionID(m_iPhone), 0);
    m_restoreDevice->port = PI_socketForPort(m_restoreDevice, 0xf27e);

	sleep(5);

	CFMutableDictionaryRef request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
															   &kCFTypeDictionaryKeyCallBacks,
															   &kCFTypeDictionaryValueCallBacks);

	if (request == NULL) {
		return;
	}

	// fs checks
	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("FilesystemCheck"));
	CFDictionarySetValue(request, CFSTR("DeviceName"), CFSTR("/dev/disk0s1"));
	PI_performOperation(m_restoreDevice, request);
	sleep(3);

	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);

	if (request == NULL) {
		return;
	}

	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("FilesystemCheck"));
	CFDictionarySetValue(request, CFSTR("DeviceName"), CFSTR("/dev/disk0s2"));
	PI_performOperation(m_restoreDevice, request);
	sleep(3);

	// mount disks
	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	
	if (request == NULL) {
		return;
	}
	
	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("Mount"));
	CFDictionarySetValue(request, CFSTR("DeviceName"), CFSTR("/dev/disk0s1"));
	CFDictionarySetValue(request, CFSTR("MountPoint"), CFSTR("/mnt1"));
	PI_performOperation(m_restoreDevice, request);

	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	
	if (request == NULL) {
		return;
	}

	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("Mount"));
	CFDictionarySetValue(request, CFSTR("DeviceName"), CFSTR("/dev/disk0s2"));
	CFDictionarySetValue(request, CFSTR("MountPoint"), CFSTR("/mnt2"));
	PI_performOperation(m_restoreDevice, request);

	// copy edited files to main fs
	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	
	if (request == NULL) {
		return;
	}

	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("Ditto"));
	CFDictionarySetValue(request, CFSTR("SourcePath"), CFSTR("/mnt2/root/Media/fstab"));
	CFDictionarySetValue(request, CFSTR("DestinationPath"), CFSTR("/mnt1/etc/fstab"));
	PI_performOperation(m_restoreDevice, request);

	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	
	if (request == NULL) {
		return;
	}

	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("Ditto"));
	CFDictionarySetValue(request, CFSTR("SourcePath"), CFSTR("/mnt2/root/Media/Services.plist"));
	CFDictionarySetValue(request, CFSTR("DestinationPath"), CFSTR("/mnt1/System/Library/Lockdown/Services.plist"));
	PI_performOperation(m_restoreDevice, request);

	// clean up
	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	
	if (request == NULL) {
		return;
	}

	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("RemovePath"));
	CFDictionarySetValue(request, CFSTR("Path"), CFSTR("/mnt2/root/Media/fstab"));
	PI_performOperation(m_restoreDevice, request);

	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	
	if (request == NULL) {
		return;
	}

	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("RemovePath"));
	CFDictionarySetValue(request, CFSTR("Path"), CFSTR("/mnt2/root/Media/Services.plist"));
	PI_performOperation(m_restoreDevice, request);

	// unmount disks
	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	
	if (request == NULL) {
		return;
	}

	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("Unmount"));
	CFDictionarySetValue(request, CFSTR("MountPoint"), CFSTR("/mnt2"));
	PI_performOperation(m_restoreDevice, request);

	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	
	if (request == NULL) {
		return;
	}

	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("Unmount"));
	CFDictionarySetValue(request, CFSTR("MountPoint"), CFSTR("/mnt1"));
	PI_performOperation(m_restoreDevice, request);

	// reboot
	request = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	
	if (request == NULL) {
		return;
	}

	CFDictionarySetValue(request, CFSTR("Operation"), CFSTR("Goodbye"));
	PI_performOperation(m_restoreDevice, request);

	m_finishingJailbreak = true;
}

void PhoneInteraction::restoreModeFinished()
{
	m_inRestoreMode = false;
	m_restoreDevice = NULL;
	(*m_notifyFunc)(NOTIFY_JAILBREAK_RESTORE_DISCONNECTED, "Restore mode ended");
}
