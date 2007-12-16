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
#include <mach-o/dyld.h>
#include <cstdlib>
#include <iostream>
#include <list>

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

#if defined (__APPLE__)
static char *g_symlinks[] =
{
	"etc",
	"mach",
	"private/etc/resolv.conf",
	"private/var/db/localtime",
	"private/var/logs/CrashReporter/LatestCrash.plist",
	"Library/Logs",
	"Library/Preferences",
	"System/Library/Frameworks/IOKit.framework/IOKit",
	"System/Library/Frameworks/IOKit.framework/Resources",
	"System/Library/Frameworks/IOKit.framework/Versions/Current",
	"System/Library/Frameworks/MultitouchSupport.framework/MultitouchSupport",
	"System/Library/Frameworks/MultitouchSupport.framework/Resources",
	"System/Library/Frameworks/MultitouchSupport.framework/Versions/Current",
	"System/Library/Frameworks/OfficeImport.framework/OfficeImport",
	"System/Library/Frameworks/OfficeImport.framework/Resources",
	"System/Library/Frameworks/OfficeImport.framework/Versions/Current",
	"System/Library/Frameworks/System.framework/System",
	"System/Library/Frameworks/System.framework/Resources",
	"System/Library/Frameworks/System.framework/Versions/Current",
	"System/Library/Frameworks/System.framework/Versions/B/System",
	"tmp",
	"usr/lib/libbz2.1.0.dylib",
	"usr/lib/libbz2.dylib",
	"usr/lib/libc.dylib",
	"usr/lib/libcharset.1.0.0.dylib",
	"usr/lib/libcharset.dylib",
	"usr/lib/libcrypto.dylib",
	"usr/lib/libcurses.dylib",
	"usr/lib/libdbm.dylib",
	"usr/lib/libdl.dylib",
	"usr/lib/libdns_sd.dylib",
	"usr/lib/libedit.dylib", 
	"usr/lib/libform.dylib",
	"usr/lib/libiconv.2.2.0.dylib",
	"usr/lib/libiconv.dylib",
	"usr/lib/libicucore.dylib",
	"usr/lib/libinfo.dylib",
	"usr/lib/libIOKit.A.dylib",
	"usr/lib/libIOKit.dylib",
	"usr/lib/libipsec.dylib",
	"usr/lib/libkvm.dylib",
	"usr/lib/libm.dylib",
	"usr/lib/libncurses.dylib",
	"usr/lib/libobjc.dylib",
	"usr/lib/libpoll.dylib",
	"usr/lib/libpthread.dylib",
	"usr/lib/librpcsvc.dylib",
	"usr/lib/libsqlite3.0.8.6.dylib",
	"usr/lib/libsqlite3.dylib",
	"usr/lib/libssl.dylib",
	"usr/lib/libstdc++.6.dylib",
	"usr/lib/libSystem.dylib",
	"usr/lib/libtidy.dylib",
	"usr/lib/libxml2.dylib",
	"usr/lib/libz.1.1.3.dylib",
	"usr/lib/libz.1.dylib",
	"usr/lib/libz.dylib",
	"var"
};
static char *g_symlinkOriginals[] =
{
	"private/etc",
	"mach_kernel",
	"../var/run/resolv.conf",
	"../../../usr/share/zoneinfo/US/Pacific",
	"../../../../Library/Logs/CrashReporter/sh-2007-09-12-001253.plist",
	"../private/var/logs",
	"../private/var/preferences",
	"Versions/A/IOKit",
	"Versions/A/Resources",
	"A",
	"Versions/A/MultitouchSupport",
	"Versions/A/Resources",
	"A",
	"Versions/A/OfficeImport",
	"Versions/A/Resources",
	"A",
	"../../../../usr/lib/libSystem.B.dylib",
	"Versions/B/Resources",
	"B",
	"../../../../../../usr/lib/libSystem.B.dylib",
	"var/tmp", 
	"libbz2.1.0.3.dylib",
	"libbz2.1.0.3.dylib",
	"libSystem.B.dylib",
	"libcharset.1.dylib",
	"libcharset.1.dylib",
	"libcrypto.0.9.7.dylib",
	"libncurses.5.4.dylib",
	"libSystem.B.dylib",
	"libSystem.B.dylib",
	"libSystem.B.dylib",
	"libedit.2.dylib",
	"libform.dylib",
	"libiconv.2.dylib",
	"libiconv.2.dylib",
	"libicucore.A.dylib",
	"libSystem.B.dylib",
	"../../System/Library/Frameworks/IOKit.framework/Versions/A/IOKit",
	"../../System/Library/Frameworks/IOKit.framework/Versions/A/IOKit",
	"libipsec.A.dylib",
	"libSystem.B.dylib",
	"libSystem.B.dylib",
	"libncurses.5.4.dylib",
	"libobjc.A.dylib",
	"libSystem.B.dylib",
	"libSystem.B.dylib",
	"libSystem.B.dylib",
	"libsqlite3.0.dylib",
	"libsqlite3.0.dylib",
	"libssl.0.9.7.dylib",
	"libstdc++.6.0.4.dylib",
	"libSystem.B.dylib",
	"libtidy.A.dylib",
	"libxml2.2.dylib",
	"libz.1.2.3.dylib",
	"libz.1.2.3.dylib",
	"libz.1.2.3.dylib",
	"private/var"
};

static int g_numSymlinks = 58;
#endif

static int g_recoveryAttempts = 0;

static t_socketForPort g_socketForPort;
static t_performOperation g_performOperation;
static t_sendCommandToDevice g_sendCommandToDevice;
static t_sendFileToDevice g_sendFileToDevice;


static void FreePointerList(std::list<unsigned char*>& list)
{
	std::list<unsigned char*>::iterator iter;
	
	for (iter = list.begin(); iter != list.end(); iter++) {
		free(*iter);
	}
	
}

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
			else if (g_phoneInteraction->m_performingNewJailbreak) {
				g_phoneInteraction->newJailbreakStageTwo();
			}

		}

	}
	else if (info->msg == ADNCI_MSG_DISCONNECTED) {

		if (g_phoneInteraction->m_inRestoreMode) {
			g_phoneInteraction->restoreModeFinished();
		}
		else {
			g_phoneInteraction->setConnected(false);
			g_phoneInteraction->setConnectedToAFC(false);
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
	g_phoneInteraction->dfuModeStarted(dev);
}

void dfuDisconnectNotificationCallback(am_recovery_device *dev)
{
#ifdef DEBUG
	CFShow(CFSTR("dfuDisconnectNotificationCallback"));
#endif
	g_phoneInteraction->dfuModeFinished(dev);
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
	g_phoneInteraction->recoveryModeStarted(dev);
}

void recoveryDisconnectNotificationCallback(am_recovery_device *dev)
{
#ifdef DEBUG
	CFShow(CFSTR("recoveryDisconnectNotificationCallback"));
#endif
	g_phoneInteraction->recoveryModeFinished(dev);
}

PhoneInteraction::PhoneInteraction(void (*statusFunc)(const char*, bool),
								   void (*notifyFunc)(int, const char*),
								   bool bUsingPrivateFunctions)
{
	m_statusFunc = statusFunc;
	m_notifyFunc = notifyFunc;
	m_iPhone = NULL;
	m_recoveryDevice = NULL;
	m_connected = false;
	m_afcConnected = false;
	m_inRecoveryMode = false;
	m_switchingToRestoreMode = false;
	m_inRestoreMode = false;
	m_jailbroken = false;
	m_finishingJailbreak = false;
	m_returningToJail = false;
	m_hAFC = NULL;
	m_firmwarePath = NULL;
	m_waitingForRecovery = false;
	m_enteringDFUMode = false;
	m_enteringRecoveryMode = false;
	m_inDFUMode = false;
	m_dfuDevice = NULL;
	m_privateFunctionsSetup = false;
	m_recoveryOccurred = false;
	m_performingNewJailbreak = false;
	m_iTunesVersion.major = 0;
	m_iTunesVersion.minor = 0;
	m_iTunesVersion.point = 0;
	m_usingPrivateFunctions = bUsingPrivateFunctions;
	m_firmwareVersion = NULL;
	m_productVersion = NULL;
	m_buildVersion = NULL;
	m_basebandVersion = NULL;
	m_serialNumber = NULL;
	m_activationState = NULL;
	m_servicesPath = NULL;

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
												void (*notifyFunc)(int, const char*),
												bool bUsingPrivateFunctions)
{

	if (g_phoneInteraction == NULL) {
		g_phoneInteraction = new PhoneInteraction(statusFunc, notifyFunc, bUsingPrivateFunctions);
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

	// first, check for an internal version of MobileDevice library
	CFBundleRef bundle = CFBundleGetMainBundle();

	if (bundle != NULL) {
		CFURLRef execPath = CFBundleCopyExecutableURL(bundle);

		if (execPath != NULL) {
			CFURLRef mainPath = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, execPath);
			CFURLRef mobDevPath = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, mainPath, CFSTR("libMobDev.dylib"), false);
			FSRef mobDevRef;

			CFRelease(execPath);
			CFRelease(mainPath);

			if (CFURLGetFSRef(mobDevPath, &mobDevRef)) {
				FSCatalogInfo mobDevInfo;

				if (FSGetCatalogInfo(&mobDevRef, kFSCatInfoCreateDate, &mobDevInfo, NULL, NULL, NULL) == noErr) {
					
					// ok, the library exists in the application bundle -- the version should be 7.4.2
					if (ConvertCFStringToPIVersion(CFSTR("7.4.2"), &m_iTunesVersion)) {
						CFRelease(mobDevPath);
						return true;
					}

				}

			}

			CFRelease(mobDevPath);
		}

	}

	// no internal version so we must rely on the version of MobileDevice installed with iTunes
	CFURLRef appUrl, versionPlistUrl;

	// try searching for iTunes.app using LaunchServices
	if (LSFindApplicationForInfo(kLSUnknownCreator, NULL, CFSTR("iTunes.app"), NULL, &appUrl) != noErr) {

		// try searching for bundleID com.apple.iTunes using LaunchServices
		if (LSFindApplicationForInfo(kLSUnknownCreator, CFSTR("com.apple.iTunes"), NULL, NULL, &appUrl) != noErr) {
			struct stat st;

			// as a last resort, try the default location
			if (stat("/Applications/iTunes.app", &st)) {
				return false;
			}

			appUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
												   CFSTR("/Applications/iTunes.app"),
												   kCFURLPOSIXPathStyle, true);
		}

	}

	if (appUrl == NULL) {
		return false;
	}
	
	versionPlistUrl = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault,
															appUrl, CFSTR("Contents/version.plist"), false);
	CFRelease(appUrl);

	if (versionPlistUrl == NULL) {
		return false;
	}

	CFDictionaryRef iTunesVersionDict = NULL;
	CFDataRef versionPlistData = NULL;
	SInt32 errorCode;
	Boolean status = CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, versionPlistUrl,
															  &versionPlistData, NULL, NULL,
															  &errorCode);

	if (!status) {
		CFRelease(versionPlistUrl);
		return false;
	}

	CFStringRef errString;
	iTunesVersionDict = (CFDictionaryRef)CFPropertyListCreateFromXMLData(kCFAllocatorDefault,
																		 versionPlistData, kCFPropertyListImmutable,
																		 &errString);

	CFRelease(versionPlistData);
	CFRelease(versionPlistUrl);

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

	if (!ConvertCFStringToPIVersion(versionStr, &m_iTunesVersion)) {
		CFRelease(iTunesVersionDict);
		return false;
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

	if ( m_iTunesVersion.major != 7 ) return;

	switch (m_iTunesVersion.minor) {
		// iTunes 7.5 offsets submitted by David Wang
		case 5:
			g_sendCommandToDevice = (t_sendCommandToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FF0+0x10008160);
			g_sendFileToDevice = (t_sendFileToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FF0+0x100082E0);
			g_socketForPort = (t_socketForPort)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FF0+0x100130D0);
			g_performOperation = (t_performOperation)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FF0+0x100141C0);
			m_privateFunctionsSetup = true;
			break;			
		case 4:
			g_sendCommandToDevice = (t_sendCommandToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x10008170);
			g_sendFileToDevice = (t_sendFileToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x100082F0);
			g_socketForPort = (t_socketForPort)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x10012F90);
			g_performOperation = (t_performOperation)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x10014040);
			m_privateFunctionsSetup = true;
			break;
		case 3:
			g_sendCommandToDevice= (t_sendCommandToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x10009290);
			g_sendFileToDevice= (t_sendFileToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x10009410);
			g_performOperation= (t_performOperation)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x100129C0);
			g_socketForPort= (t_socketForPort)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x10012830);
			m_privateFunctionsSetup = true;
			break;
		default:
			break;
	}

}
#elif defined(__APPLE__) && defined(__POWERPC__)
void PhoneInteraction::setupPrivateFunctions()
{

	if ( m_iTunesVersion.major != 7 ) return;
	
	switch (m_iTunesVersion.minor) {
		// iTunes 7.5 offsets submitted by David Wang
		case 5:
			g_performOperation = (t_performOperation)0x3c3a1884;
			g_socketForPort = (t_socketForPort)0x3c3a11d8;
			g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a693c;
			g_sendFileToDevice = (t_sendFileToDevice)0x3c3a6a9c;
			m_privateFunctionsSetup = true;
			break;
		case 4:
			g_performOperation = (t_performOperation)0x3c3a0bc8;
			g_socketForPort = (t_socketForPort)0x3c3a051c;
			g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a5bb0;
			g_sendFileToDevice = (t_sendFileToDevice)0x3c3a5d10;
			m_privateFunctionsSetup = true;
			break;
		case 3:
			g_performOperation = (t_performOperation)0x3c3a0e14;
			g_socketForPort = (t_socketForPort)0x3c3a0644;
			g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a517c;
			g_sendFileToDevice = (t_sendFileToDevice)0x3c3a52dc;
			m_privateFunctionsSetup = true;
			break;
		default:
			break;
	}

}
#elif defined(__APPLE__)
void PhoneInteraction::setupPrivateFunctions()
{

	if ( m_iTunesVersion.major != 7 ) return;

	switch (m_iTunesVersion.minor) {
		// iTunes 7.5 offsets submitted by David Wang
		case 5:
			g_performOperation = (t_performOperation)0x3c3a02f5;
			g_socketForPort = (t_socketForPort)0x3c39fcff;
			g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a57a3;
			g_sendFileToDevice = (t_sendFileToDevice)0x3c3a59ef;
			m_privateFunctionsSetup = true;
			break;
		case 4:
			g_performOperation = (t_performOperation)0x3c3a0599;
			g_socketForPort = (t_socketForPort)0x3c39ffa3;
			g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a597f;
			g_sendFileToDevice = (t_sendFileToDevice)0x3c3a5bcb;
			m_privateFunctionsSetup = true;
			break;
		case 3:
			g_performOperation = (t_performOperation)0x3c39fa4b;
			g_socketForPort = (t_socketForPort)0x3c39f36c;
			g_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a3e3b;
			g_sendFileToDevice = (t_sendFileToDevice)0x3c3a4087;
			m_privateFunctionsSetup = true;
			break;
		default:
			break;
	}

}
#endif

bool PhoneInteraction::arePrivateFunctionsSetup()
{
	return m_privateFunctionsSetup;
}

void PhoneInteraction::subscribeToNotifications()
{

	if (!arePrivateFunctionsSetup() && m_usingPrivateFunctions) {
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
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Can't connect to phone.");
		return;
	}

	if ( !AMDeviceIsPaired(m_iPhone) ) {
		AMDevicePair(m_iPhone);

		if ( !AMDeviceIsPaired(m_iPhone) ) {
			(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Phone is not paired.  Make sure you run iTunes with your phone connected at least once.");
			return;
		}

	}

	if ( AMDeviceValidatePairing(m_iPhone) ) {
		AMDevicePair(m_iPhone);

		if ( AMDeviceValidatePairing(m_iPhone) ) {
			(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Pairing is not valid.  Make sure your run iTunes with your phone connected at least once.");
			return;
		}

	}

	if ( AMDeviceStartSession(m_iPhone) ) {
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Error starting session.");
		return;
	}

	if (m_productVersion != NULL) {
		free(m_productVersion);
		m_productVersion = NULL;
	}

	if (!readValue("ProductVersion", &m_productVersion)) {
		// sometimes this fails -- wait for a second and try again
		sleep(1);

		if (!readValue("ProductVersion", &m_productVersion)) {
			(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Couldn't get product version.");
			return;
		}
		
	}

	// waiting .1 seconds between reads seems to help stability
	usleep(100000);

	if (m_firmwareVersion != NULL) {
		free(m_firmwareVersion);
		m_firmwareVersion = NULL;
	}
	
	if (!readValue("FirmwareVersion", &m_firmwareVersion)) {
		sleep(1);
		
		if (!readValue("FirmwareVersion", &m_firmwareVersion)) {
			(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Couldn't get firmware version.");
			return;
		}
		
	}
	
	usleep(100000);

	if (m_buildVersion != NULL) {
		free(m_buildVersion);
		m_buildVersion = NULL;
	}
	
	if (!readValue("BuildVersion", &m_buildVersion)) {
		sleep(1);

		if (!readValue("BuildVersion", &m_buildVersion)) {
			(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Couldn't get build version.");
			return;
		}
		
	}
	
	usleep(100000);

	if (m_basebandVersion != NULL) {
		free(m_basebandVersion);
		m_basebandVersion = NULL;
	}
	
	if (!readValue("BasebandVersion", &m_basebandVersion)) {
		sleep(1);

		if (!readValue("BasebandVersion", &m_basebandVersion)) {
			(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Couldn't get baseband version.");
			return;
		}

	}
	
	usleep(100000);
	
	if (m_serialNumber != NULL) {
		free(m_serialNumber);
		m_serialNumber = NULL;
	}
	
	if (!readValue("SerialNumber", &m_serialNumber)) {
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Couldn't get serial number.");
		return;
	}
	
	usleep(100000);

	if (m_activationState != NULL) {
		free(m_activationState);
		m_activationState = NULL;
	}
	
	if (!readValue("ActivationState", &m_activationState)) {
		sleep(1);
		
		if (!readValue("ActivationState", &m_activationState)) {
			(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Couldn't get activation state.");
			return;
		}
		
	}

#ifdef DEBUG
	printf("ProductVersion: %s\nFirmwareVersion: %s\nBuildVersion: %s\nBasebandVersion: %s\nSerialNumber: %s\nActivationState: %s\n",
		   m_productVersion, m_firmwareVersion, m_buildVersion, m_basebandVersion, m_serialNumber, m_activationState);
#endif

	PIVersion productVersion;

	if (!ConvertCStringToPIVersion(m_productVersion, &productVersion)) {
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, "Connection failed: Error in product version.");
		return;
	}

	// we only support firmware versions 1.0 to 1.1.2
	if ( productVersion.major != 1 ) {
		char msg[128];
		snprintf(msg, 128, "Unsupported version of phone firmware installed.\nDetected version is %d.%d.%d\n",
				 productVersion.major, productVersion.minor, productVersion.point);
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, msg);
		return;
	}

	if (productVersion.minor > 1) {
		char msg[128];
		snprintf(msg, 128, "Unsupported version of phone firmware installed.\nDetected version is %d.%d.%d\n",
				 productVersion.major, productVersion.minor, productVersion.point);
		(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, msg);
		return;
	}

	if (productVersion.minor == 1) {

		if (productVersion.point > 2) {
			char msg[128];
			snprintf(msg, 128, "Unsupported version of phone firmware installed.\nDetected version is %d.%d.%d\n",
					 productVersion.major, productVersion.minor, productVersion.point);
			(*m_notifyFunc)(NOTIFY_CONNECTION_FAILED, msg);
			return;
		}

	}
	
	connectToAFC();
	setConnected(true);
	(*m_notifyFunc)(NOTIFY_CONNECTION_SUCCESS, "Connection success!");
}

void PhoneInteraction::connectToAFC()
{

	if (isConnectedToAFC()) return;

	if (m_statusFunc) {
		(*m_statusFunc)("Connecting to AFC...", true);
	}

	m_jailbroken = false;
	afc_connection *hAFC;

	if ( AMDeviceStartService(m_iPhone, AMSVC_AFC2, &hAFC, NULL) ) {
		
		if ( AMDeviceStartService(m_iPhone, AMSVC_AFC, &hAFC, NULL) ) {
			(*m_notifyFunc)(NOTIFY_AFC_CONNECTION_FAILED, "Connection failed: Error connecting to AFC service.");
			return;
		}

	}
	else {

		// AFC2 is running.  We're jailbroken.
		m_jailbroken = true;
	}

	if (AFCConnectionOpen(hAFC, 0, &m_hAFC)) {
		(*m_notifyFunc)(NOTIFY_AFC_CONNECTION_FAILED, "Connection failed: Error opening connection to AFC service");
		return;
	}

#if defined(__APPLE__)
	AFCPlatformInit();
#endif

	if (!m_jailbroken) {
		
		// For older jailbreak compatibility: test to see if we can open a system
		// file.  If so, then we're jailbroken.
		//
		// This doesn't hold anymore since the hack 1.1.1 upgrade will allow you to read
		// system files, but you still don't have write access.  So check to see if we
		// can actually write to a file as well.
		afc_file_ref rAFC, rAFC2;
		
		// first check if we can open a system file to verify we're using the
		// correct filesystem (ie. not Media)
		if (!AFCFileRefOpen(m_hAFC, "/private/etc/master.passwd", 1, &rAFC)) {

			if (AFCFileRefClose(m_hAFC, rAFC)) {
				printf("couldn't close /private/etc/master.passwd\n");
			}
			
			// now check if we can write a file
			if (!AFCFileRefOpen(m_hAFC, "/jailbreak_check", 3, &rAFC2)) {
				m_jailbroken = true;

				if (AFCFileRefClose(m_hAFC, rAFC2)) {
					printf("couldn't close /jailbreak_check\n");
				}

				if (AFCRemovePath(m_hAFC, "/jailbreak_check")) {
					printf("couldn't remove /jailbreak_check\n");
				}

			}
			
		}
		
	}

	setConnectedToAFC(true);
	(*m_notifyFunc)(NOTIFY_AFC_CONNECTION_SUCCESS, "AFC Connection success!");
}

void PhoneInteraction::disconnectFromPhone()
{
	
	if (isConnected()) {

		if (isConnectedToAFC()) {
			AFCConnectionClose(m_hAFC);
			setConnectedToAFC(false);
		}

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

		if (m_productVersion != NULL) {
			free(m_productVersion);
			m_productVersion = NULL;
		}

		if (m_firmwareVersion != NULL) {
			free(m_firmwareVersion);
			m_firmwareVersion = NULL;
		}
		
		if (m_buildVersion != NULL) {
			free(m_buildVersion);
			m_buildVersion = NULL;
		}
		
		if (m_basebandVersion != NULL) {
			free(m_basebandVersion);
			m_basebandVersion = NULL;
		}

		if (m_serialNumber != NULL) {
			free(m_serialNumber);
			m_serialNumber = NULL;
		}
		
		if (m_activationState != NULL) {
			free(m_activationState);
			m_activationState = NULL;
		}
		
		(*m_notifyFunc)(NOTIFY_DISCONNECTED, "Disconnected");
	}

}

bool PhoneInteraction::isConnected()
{
	return m_connected;
}

void PhoneInteraction::setConnectedToAFC(bool connected)
{
	m_afcConnected = connected;

	if (m_afcConnected) {
		(*m_notifyFunc)(NOTIFY_AFC_CONNECTED, "AFC Connected");
	}
	else {
		(*m_notifyFunc)(NOTIFY_AFC_DISCONNECTED, "AFC Disconnected");
	}
	
}

bool PhoneInteraction::isConnectedToAFC()
{
	return m_afcConnected;
}

bool PhoneInteraction::enableThirdPartyApplications(bool undo)
{
	char *phoneProdVer = getPhoneProductVersion();

	if (!strcmp(phoneProdVer, "1.1.1")) {
		unsigned char *buf;
		int size;
	
		if (!getFileData((void**)&buf, &size, "/System/Library/CoreServices/SpringBoard.app/SpringBoard", 0, 0)) {
			return false;
		}
		
		if (undo) {
			buf[0x7C564] = 0x09;
			buf[0x7C565] = 0x01;
			buf[0x7C567] = 0x0A;
		}
		else {
			buf[0x7C564] = 0x00;
			buf[0x7C565] = 0x00;
			buf[0x7C567] = 0x00;
		}
		
		if (!putData(buf, size, "/System/Library/CoreServices/SpringBoard.app/SpringBoard", 0, 0)) {
			free(buf);
			return false;
		}

		free(buf);
	}

	return true;
}

bool PhoneInteraction::enableCustomRingtones(bool undo)
{
	char *phoneProdVer = getPhoneProductVersion();
	
	if (!strcmp(phoneProdVer, "1.1.1")) {
		unsigned char *buf;
		int size;
		
		if (!getFileData((void**)&buf, &size, "/System/Library/Frameworks/MeCCA.framework/MeCCA", 0, 0)) {
			return false;
		}
		
		if (undo) {
			buf[0x14458] = 0x05;
			buf[0x1445C] = 0x6C;
			buf[0x1445D] = 0x10;
			buf[0x1445E] = 0x9F;
			buf[0x1445F] = 0xE5;
			buf[0x14462] = 0x8F;
			buf[0x14463] = 0xE0;
			buf[0x14464] = 0x6D;
			buf[0x14465] = 0x28;
			buf[0x14466] = 0x03;
			buf[0x14467] = 0xEB;
			buf[0x14468] = 0x00;
			buf[0x1446A] = 0x50;
			buf[0x1446B] = 0xE3;
			buf[0x1446C] = 0x01;
			buf[0x1446D] = 0x30;
			buf[0x1446E] = 0xA0;
			buf[0x1446F] = 0x03;
			buf[0x14470] = 0x00;
			buf[0x14472] = 0x00;
			buf[0x14473] = 0x0A;
			buf[0x14474] = 0x00;
			buf[0x14477] = 0xE3;
		}
		else {
			buf[0x14458] = 0x06;
			buf[0x1445C] = 0x03;
			buf[0x1445D] = 0x30;
			buf[0x1445E] = 0x43;
			buf[0x1445F] = 0xE0;
			buf[0x14462] = 0x50;
			buf[0x14463] = 0xE5;
			buf[0x14464] = 0x20;
			buf[0x14465] = 0x00;
			buf[0x14466] = 0x51;
			buf[0x14467] = 0xE3;
			buf[0x14468] = 0x02;
			buf[0x1446A] = 0x00;
			buf[0x1446B] = 0x0A;
			buf[0x1446C] = 0x03;
			buf[0x1446D] = 0x10;
			buf[0x1446E] = 0x90;
			buf[0x1446F] = 0xE5;
			buf[0x14470] = 0x72;
			buf[0x14472] = 0x51;
			buf[0x14473] = 0xE3;
			buf[0x14474] = 0x01;
			buf[0x14477] = 0x03;
		}
		
		if (!putData(buf, size, "/System/Library/Frameworks/MeCCA.framework/MeCCA", 0, 0)) {
			free(buf);
			return false;
		}
		
		free(buf);
	}

	return true;
}

bool PhoneInteraction::enableYouTube(bool bUndo)
{
	char *devicePublicKey = NULL;
	char *devicePrivateKey = NULL;
	char *dataArkPlist = NULL;

	if (bUndo) {
		devicePublicKey =
			"-----BEGIN RSA PUBLIC KEY-----\n"
			"MIGJAoGBAMOOJTKvYMLX245nLzq6nQnCwbziDH4KGaNKpJ36rRKJJJvqGJQhgDVH\n"
			"lZDXTVo13c7zVBIMQPkGnVwxlowDJiOJ99/HPGGnaN/6ZkVcrqjY4XPIa02TqAnH\n"
			"d9Y3CfrVtXGXIecHQqnWgshPuF2bPMZYGpuIt1DlS6uzpbK++yktAgMBAAE=\n"
			"-----END RSA PUBLIC KEY-----\n";

		devicePrivateKey =
			"-----BEGIN RSA PRIVATE KEY-----\n"
			"MIICXgIBAAKBgQDDjiUyr2DC19uOZy86up0JwsG84gx+ChmjSqSd+q0SiSSb6hiU\n"
			"IYA1R5WQ101aNd3O81QSDED5Bp1cMZaMAyYjifffxzxhp2jf+mZFXK6o2OFzyGtN\n"
			"k6gJx3fWNwn61bVxlyHnB0Kp1oLIT7hdmzzGWBqbiLdQ5Uurs6WyvvspLQIDAQAB\n"
			"AoGBAKJxaaT42kAAX3mjbTAz5E8/YQAuJoJskW97idNRcIN9ONPrWg9y5LVdiXiP\n"
			"4MmXBXHr32I+m5pBTGZRKuWTmBFnZ7HpNgAlea7E/Fb9q/slP5cH6Lmz+Glszkpe\n"
			"i1qYCuM72eOQFYp8hQUYrbmBSUx61Fh3MnceRpMsGjFP15phAkEA+ocxi78HAsIK\n"
			"WxeYs4o9P18MqvfCNH3xakk9wujZs2jecnuqJKUMlWpbqDNehWuyRtraVqSk7uqA\n"
			"VYriDwlKQwJBAMfTk2fORFssIzzEes44lJlYlmes8aymg8fL9hpq+JB15zDaAT/L\n"
			"MmfO3vPy6Pg8FBXo84OBUXv6Fd/ULZj+H88CQQDNL+UYe8CWNa6dpNngUpyPRp3t\n"
			"eTaKH4yWbJ41ANks4/ss8LQNh1CjH5UqUchcpjRBbAXfaMHdHBs39KPphMZZAkB3\n"
			"6qc+F1F8KTuoPvy3fsrmT3xLEUUi5/aTUvoIloM+JhMshNdVEjrYgxPW78IRHfSr\n"
			"xVVFaLientC7ttf6RR6PAkEAhU9+JeAu20NAy9qmaL5+6Oi+QH84Txo8GGCw4idV\n"
			"XLmNJhMAdnIa2nMZ9B1QJ/OW7e5z1MbXDCII72ELULz3kw==\n"
			"-----END RSA PRIVATE KEY-----\n";
		
		dataArkPlist =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
			"<plist version=\"1.0\">\n"
			"<dict>\n"
			"<key>-DeviceName</key>\n"
			"<string></string>\n"
			"<key>-FirmwareVersion</key>\n"
			"<string>iBoot-159</string>\n"
			"<key>-ProtocolVersion</key>\n"
			"<string>1</string>\n"
			"<key>-SBLockdownEverRegisteredKey</key>\n"
			"<false/>\n"
			"<key>com.apple.mobile.lockdown_cache-ActivationState</key>\n"
			"<string>Unactivated</string>\n"
			"</dict>\n"
			"</plist>\n";
	}
	else {
		devicePublicKey =
			"-----BEGIN RSA PUBLIC KEY-----\n"
			"MIGJAoGBAOWLB1z60B43IwITntQMKBZqnubi1J4xrrNJkoX/FRkcKShzd0fpSEze\n"
			"6/Yq16d5ci85uKtjTJtGDZcq9mhYmMg6jvTzPxox5gOboHu4aUxcegcGP8mDZTr7\n"
			"O3mkeMkWDQVGvgsKL0ua4XbYUwbbpzhRS7N5ifDErRYKwRYcYz/VAgMBAAE=\n"
			"-----END RSA PUBLIC KEY-----\n";

		devicePrivateKey =
			"-----BEGIN RSA PRIVATE KEY-----\n"
			"MIICWwIBAAKBgQDliwdc+tAeNyMCE57UDCgWap7m4tSeMa6zSZKF/xUZHCkoc3dH\n"
			"6UhM3uv2KteneXIvObirY0ybRg2XKvZoWJjIOo708z8aMeYDm6B7uGlMXHoHBj/J\n"
			"g2U6+zt5pHjJFg0FRr4LCi9LmuF22FMG26c4UUuzeYnwxK0WCsEWHGM/1QIDAQAB\n"
			"AoGAbmfamMxCgeX/PqZ1RIS8W1vZJjCAF77Jyo5enXi9iyBSY5R2EO6RyfeHAxZE\n"
			"N9dgJnra6gSO+jhNnSIa9sF2ah8PnA+CA/Mq3G+kdjqIByJfeBCCz6hR6/p5LLIZ\n"
			"gnTOBs5GkHCbbz9ZKQuFi8OmKPgtj/URLZ6o9wPRUY/JP0ECQQD39LTYgZGAqECI\n"
			"nVIxFDs3p7JV9iAZa3eiKR17BspOEcDBPvixxmrwWa3TzaRtI9o+0HknloPHYGeD\n"
			"3V0jV61dAkEA7P1nC5GzE3QtkrRey9/D9KQd834bm07Swkxegbjn8QXP8hmEV8LQ\n"
			"bvL7vOLiAf8uEo7pq54ADS4Zem3B5bW82QJASmqp4BS664cTnyzAHzS4NRLiZgQx\n"
			"TA/B3uxCCctW6ilP1W+lyg0HyUzQ67FbONo6xQFiayw0LqFTT/Me4d2NjQJAY5D7\n"
			"EANApzWyR+Z7xU/XthqVcs1Sr+dn6LXJJtsWp0531REfZve0NkjjtrHjnk8lfiqI\n"
			"xc912hO6JJOkWOwH0QJAGod7UZwiZuA6WNaFjF0hXsjAGVNT6bVAvm6X3RFeK7qY\n"
			"owiCq9JAN8ZX5l0VWNDp6MGCVly9OtMNK4Lp6a2Q2Q==\n"
			"-----END RSA PRIVATE KEY-----\n";

		dataArkPlist =
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
			"<plist version=\"1.0\">\n"
			"<dict>\n"
			"        <key>-ActivationStateAcknowledged</key>\n"
			"        <true/>\n"
			"        <key>-DeviceCertificate</key>\n"
			"        <data>\n"
			"        LS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSURQekNDQXFpZ0F3SUJBZ0lLQTRC\n"
			"        UllCOVBRUms3NlRBTkJna3Foa2lHOXcwQkFRc0ZBREJhTVFzd0NRWUQKVlFRR0V3SlZV\n"
			"        ekVUTUJFR0ExVUVDaE1LUVhCd2JHVWdTVzVqTGpFVk1CTUdBMVVFQ3hNTVFYQndiR1Vn\n"
			"        YVZCbwpiMjVsTVI4d0hRWURWUVFERXhaQmNIQnNaU0JwVUdodmJtVWdSR1YyYVdObElF\n"
			"        TkJNQjRYRFRBM01EY3dOekl6Ck1EZ3dNMW9YRFRFd01EY3dOekl6TURnd00xb3dnWWN4\n"
			"        TVRBdkJnTlZCQU1US0RNNE5URmhOamRqT0dFM01ETTMKTnpjek9HWXhOVEJsTTJGaFlU\n"
			"        WTJOV1U1TkRFNU5tUXpOekV4Q3pBSkJnTlZCQVlUQWxWVE1Rc3dDUVlEVlFRSQpFd0pE\n"
			"        UVRFU01CQUdBMVVFQnhNSlEzVndaWEowYVc1dk1STXdFUVlEVlFRS0V3cEJjSEJzWlNC\n"
			"        SmJtTXVNUTh3CkRRWURWUVFMRXdacFVHaHZibVV3Z1o4d0RRWUpLb1pJaHZjTkFRRUJC\n"
			"        UUFEZ1kwQU1JR0pBb0dCQU9XTEIxejYKMEI0M0l3SVRudFFNS0JacW51YmkxSjR4cnJO\n"
			"        SmtvWC9GUmtjS1NoemQwZnBTRXplNi9ZcTE2ZDVjaTg1dUt0agpUSnRHRFpjcTltaFlt\n"
			"        TWc2anZUelB4b3g1Z09ib0h1NGFVeGNlZ2NHUDhtRFpUcjdPM21rZU1rV0RRVkd2Z3NL\n"
			"        CkwwdWE0WGJZVXdiYnB6aFJTN041aWZERXJSWUt3UlljWXovVkFnTUJBQUdqZ2Qwd2dk\n"
			"        b3dnWUlHQTFVZEl3UjcKTUhtQUZMTCtJU05FaHBWcWVkV0JKbzV6RU5pblRJNTBvVjZr\n"
			"        WERCYU1Rc3dDUVlEVlFRR0V3SlZVekVUTUJFRwpBMVVFQ2hNS1FYQndiR1VnU1c1akxq\n"
			"        RVZNQk1HQTFVRUN4TU1RWEJ3YkdVZ2FWQm9iMjVsTVI4d0hRWURWUVFECkV4WkJjSEJz\n"
			"        WlNCcFVHaHZibVVnUkdWMmFXTmxJRU5CZ2dFQk1CMEdBMVVkRGdRV0JCVHBsVTJrZE5z\n"
			"        eFpFbmEKTXFUSnVaVWdaQzM1WHpBTUJnTlZIUk1CQWY4RUFqQUFNQTRHQTFVZER3RUIv\n"
			"        d1FFQXdJRm9EQVdCZ05WSFNVQgpBZjhFRERBS0JnZ3JCZ0VGQlFjREFUQU5CZ2txaGtp\n"
			"        Rzl3MEJBUXNGQUFPQmdRQ3FjbVNyTnFnWldGdkZJYzluCkNwMktacUVRK0ozdTNLS2h4\n"
			"        ZUwzR05KZ2twS3JQa3R3RC8rRGhIMlFDRUNtRUNMNTI0MHVLWGkvTzFlZjZZMkIKWjM4\n"
			"        SVFuOHVoci9oYkRXUWFpV1o2OFhQWUEvTzRFZ3lKMHp6cmlzMks3UzBPSG9vTm1uZVow\n"
			"        cEVMMElib2ZaOApKRjhHbjRaK0I2dHR6MEtQSDdJWTJ3LzRYZz09Ci0tLS0tRU5EIENF\n"
			"        UlRJRklDQVRFLS0tLS0K\n"
			"        </data>\n"
			"        <key>-DeviceName</key>\n"
			"        <string></string>\n"
			"        <key>-FirmwareVersion</key>\n"
			"        <string>iBoot-159</string>\n"
			"        <key>-PasswordProtected</key>\n"
			"        <false/>\n"
			"        <key>-ProtocolVersion</key>\n"
			"        <string>1</string>\n"
			"        <key>-SBLockdownEverRegisteredKey</key>\n"
			"        <true/>\n"
			"        <key>com.apple.mobile.lockdown_cache-ActivationState</key>\n"
			"        <string>Activated</string>\n"
			"</dict>\n"
			"</plist>\n";
	}

	if (!putData(devicePublicKey, strlen(devicePublicKey), "/var/root/Library/Lockdown/device_public_key.pem", 0, 0)) {
		return false;
	}
	
	if (!putData(devicePrivateKey, strlen(devicePrivateKey), "/var/root/Library/Lockdown/device_private_key.pem", 0, 0)) {
		return false;
	}

	if (!putData(dataArkPlist, strlen(dataArkPlist), "/var/root/Library/Lockdown/data_ark.plist", 0, 0)) {
		return false;
	}

	return true;
}

bool PhoneInteraction::factoryActivate(bool undo)
{
	unsigned char *buf;
	int size;
	
	if (!getFileData((void**)&buf, &size, "/usr/libexec/lockdownd", 0, 0)) {
		return false;
	}

	char *phoneProdVer = getPhoneProductVersion();

	if (!strcmp(phoneProdVer, "1.0")) {

		if (undo) {
			buf[0x90A4] = 0x01;
			buf[0x90A7] = 0x03;
			buf[0x90A8] = 0x3C;
			buf[0x90AB] = 0x05;
			buf[0x90B3] = 0x0A; 
			buf[0x9263] = 0x0A;            
		}
		else {
			buf[0x90A4] = 0x00;
			buf[0x90A7] = 0xE3;
			buf[0x90A8] = 0x68;
			buf[0x90AB] = 0xE5;
			buf[0x90B3] = 0xEA; 
			buf[0x9263] = 0xEA;
		}

	}
	else if (!strcmp(phoneProdVer, "1.0.1")) {

		if (undo) {
			buf[0x94C4] = 0x01;
			buf[0x94C7] = 0x03;
			buf[0x94C8] = 0x3C;
			buf[0x94CB] = 0x05;
			buf[0x94D3] = 0x0A; 
			buf[0x9683] = 0x0A;            
		}
		else {
			buf[0x94C4] = 0x00;
			buf[0x94C7] = 0xE3;
			buf[0x94C8] = 0x68;
			buf[0x94CB] = 0xE5;
			buf[0x94D3] = 0xEA; 
			buf[0x9683] = 0xEA;
		}

	}
	else if (!strcmp(phoneProdVer, "1.0.2")) {
		
		if (undo) {
			buf[0x94F0] = 0x01;
			buf[0x94F3] = 0x03;
			buf[0x94F4] = 0x3C;
			buf[0x94F7] = 0x05;
			buf[0x94FF] = 0x0A; 
			buf[0x96AF] = 0x0A;            
		}
		else {
			buf[0x94F0] = 0x00;
			buf[0x94F3] = 0xE3;
			buf[0x94F4] = 0x68;
			buf[0x94F7] = 0xE5;
			buf[0x94FF] = 0xEA; 
			buf[0x96AF] = 0xEA;
		}
		
	}
	else if (!strcmp(phoneProdVer, "1.1.1")) {

		if (undo) {
			buf[0xB810] = 0x04;
			buf[0xB812] = 0x00;
			buf[0xB813] = 0x1A;
			buf[0xB814] = 0x24;
			buf[0xB818] = 0x01;
		}
		else {
			buf[0xB810] = 0x00;
			buf[0xB812] = 0xA0;
			buf[0xB813] = 0xE1;
			buf[0xB814] = 0x54;
			buf[0xB818] = 0x00;
		}
		
	}
	else if (!strcmp(phoneProdVer, "1.1.2")) {
		
		if (undo) {
			buf[0x4B3B] = 0x1A;
			buf[0xC5C8] = 0x04;
			buf[0xC5CA] = 0x00;
			buf[0xC5CB] = 0x1A;
			buf[0xC5CC] = 0x01;
			buf[0xC5D4] = 0x88;
		}
		else {
			buf[0x4B3B] = 0xEA;
			buf[0xC5C8] = 0x00;
			buf[0xC5CA] = 0xA0;
			buf[0xC5CB] = 0xE1;
			buf[0xC5CC] = 0x00;
			buf[0xC5D4] = 0xEC;
		}
		
	}
	else {
		free(buf);
		return false;
	}
		
	if (!putData(buf, size, "/usr/libexec/lockdownd", 0, 0)) {
		free(buf);
		return false;
	}
	
	free(buf);
	return true;
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

		if (m_activationState != NULL) {
			free(m_activationState);
		}

		CFIndex cflen = CFStringGetLength(result);
		m_activationState = (char*)malloc(cflen+1);

		if (CFStringGetCString(result, m_activationState, cflen+1, kCFStringEncodingUTF8) == false) {
			free(m_activationState);
			m_activationState = NULL;
		}

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
		
		if (m_activationState != NULL) {
			free(m_activationState);
		}
		
		CFIndex cflen = CFStringGetLength(result);
		m_activationState = (char*)malloc(cflen+1);
		
		if (CFStringGetCString(result, m_activationState, cflen+1, kCFStringEncodingUTF8) == false) {
			free(m_activationState);
			m_activationState = NULL;
		}
		

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

bool PhoneInteraction::readValue(const char *key, char **value)
{
	CFStringRef cfkey = CFStringCreateWithCString(kCFAllocatorDefault, key, kCFStringEncodingUTF8);

	if (cfkey == NULL) return false;

	CFStringRef cfvalue = AMDeviceCopyValue(m_iPhone, 0, cfkey);
	CFRelease(cfkey);

	if (cfvalue == NULL) {
		*value = NULL;
		return false;
	}

	CFIndex cflen = CFStringGetLength(cfvalue);
	*value = (char*)malloc(cflen+1);

	if (CFStringGetCString(cfvalue, *value, cflen+1, kCFStringEncodingUTF8) == false) {
		free(value);
		*value = NULL;
		return false;
	}

	return true;
}

char *PhoneInteraction::getPhoneFirmwareVersion()
{
	return m_firmwareVersion;
}

char *PhoneInteraction::getPhoneProductVersion()
{
	return m_productVersion;
}

char *PhoneInteraction::getPhoneBuildVersion()
{
	return m_buildVersion;
}

char *PhoneInteraction::getPhoneBasebandVersion()
{
	return m_basebandVersion;
}

char *PhoneInteraction::getPhoneSerialNumber()
{
	return m_serialNumber;
}

char *PhoneInteraction::getPhoneActivationState()
{
	return m_activationState;
}

PIVersion PhoneInteraction::getiTunesVersion()
{
	return m_iTunesVersion;
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

	// 0p: People may want to just create an empty file
	if (len > 0) {

		if (AFCFileRefWrite(m_hAFC, rAFC, data, len)) {
			AFCFileRefClose(m_hAFC, rAFC);
			
			if (failureMsg) {
				(*m_notifyFunc)(failureMsg, "Error writing destination file on phone.");
			}
			
			return false;
		}
		
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

char *PhoneInteraction::getUserRingtoneName(const char *filename)
{

	if (!isConnected()) {
		return NULL;
	}
	
	char *value = getPhoneProductVersion();
	
	if (!strncmp(value, "1.0", 3)) {
		return (char*)filename;
	}

	const char *ringtoneConfigFile = "/private/var/root/Media/iTunes_Control/iTunes/Ringtones.plist";

	if (!fileExists(ringtoneConfigFile)) {
		return NULL;
	}

	// file exists, read it into a dictionary
	unsigned char *buf;
	int bufsize;
	
	if (!getFileData((void**)&buf, &bufsize, ringtoneConfigFile, 0, 0)) {
		return NULL;
	}
	
	CFDataRef pData = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)buf,
												  (CFIndex)bufsize, kCFAllocatorNull);
	
	if (pData == NULL) {
		free(buf);
		return NULL;
	}
	
	CFDictionaryRef configFileContents = (CFDictionaryRef)CFPropertyListCreateFromXMLData(kCFAllocatorDefault, pData,
																						  kCFPropertyListImmutable,
																						  NULL);

	if (configFileContents == NULL) {
		free(buf);
		CFRelease(pData);
		return NULL;
	}

	free(buf);
	CFRelease(pData);

	CFDictionaryRef ringtoneList = (CFDictionaryRef)CFDictionaryGetValue(configFileContents,
																		 CFSTR("Ringtones"));

	if (ringtoneList == NULL) {
		CFRelease(configFileContents);
		return NULL;
	}

	CFStringRef filenameStr = CFStringCreateWithCString(kCFAllocatorDefault, filename, kCFStringEncodingUTF8);

	if (filenameStr == NULL) {
		CFRelease(configFileContents);
		return NULL;
	}

	CFDictionaryRef ringtoneDict = (CFDictionaryRef)CFDictionaryGetValue(ringtoneList, filenameStr);
	CFRelease(filenameStr);

	if (ringtoneDict == NULL) {
		CFRelease(configFileContents);
		return NULL;
	}

	CFStringRef ringtone = (CFStringRef)CFDictionaryGetValue(ringtoneDict, CFSTR("Name"));

	if (ringtone == NULL) {
		CFRelease(configFileContents);
		return NULL;
	}

	CFIndex len = CFStringGetLength(ringtone);
	char *ringtoneName = (char*)malloc(len+1);

	if (!CFStringGetCString(ringtone, ringtoneName, len+1, kCFStringEncodingUTF8)) {
		CFRelease(configFileContents);
		free(ringtoneName);
		return NULL;
	}

	CFRelease(configFileContents);
	return ringtoneName;
}

bool PhoneInteraction::getUserRingtoneFilenames(const char *ringtoneName, char ***filenames, int *numFiles)
{
	
	if (!isConnected()) {
		return false;
	}
	
	char *value = getPhoneProductVersion();
	
	if (!strncmp(value, "1.0", 3)) {
		*numFiles = 1;
		*filenames = (char**)malloc(sizeof(char*));
		(*filenames)[0] = (char*)malloc(strlen(ringtoneName)+1);
		strcpy((*filenames)[0], ringtoneName);
		return true;
	}

	CFDictionaryRef ringtones = getUserRingtoneDictionary();
	CFDictionaryRef ringtoneList = (CFDictionaryRef)CFDictionaryGetValue(ringtones, CFSTR("Ringtones"));
	CFStringRef ringtoneNameStr = CFStringCreateWithCString(kCFAllocatorDefault, ringtoneName, kCFStringEncodingUTF8);

	if (ringtoneNameStr == NULL) {
		CFRelease(ringtones);
		return false;
	}

	CFIndex count = CFDictionaryGetCount(ringtoneList);
	CFTypeRef keys[count], values[count];
	CFDictionaryGetKeysAndValues(ringtoneList, keys, values);
	CFDictionaryRef dictValue;
	CFStringRef valName;
	int fileCount = 0;

	// count the number of matching files
	for (int i = 0; i < count; i++) {
		dictValue = (CFDictionaryRef)values[i];
		valName = (CFStringRef)CFDictionaryGetValue(dictValue, CFSTR("Name"));

		if (valName != NULL) {

			if (CFStringCompare(ringtoneNameStr, valName, 0) == kCFCompareEqualTo) {
				fileCount++;
			}

		}

	}

	*numFiles = fileCount;
	*filenames = (char**)malloc(fileCount * sizeof(char*));
	fileCount = 0;

	// now add them to the list
	for (int i = 0; i < count; i++) {
		dictValue = (CFDictionaryRef)values[i];
		valName = (CFStringRef)CFDictionaryGetValue(dictValue, CFSTR("Name"));
		
		if (valName != NULL) {

			if (CFStringCompare(ringtoneNameStr, valName, 0) == kCFCompareEqualTo) {
				CFIndex keyLength = CFStringGetLength((CFStringRef)keys[i]);
				(*filenames)[fileCount] = (char*)malloc(keyLength+1);
				CFStringGetCString((CFStringRef)keys[i], (*filenames)[fileCount], keyLength+1,
								   kCFStringEncodingUTF8);
				fileCount++;
			}
			
		}
		
	}

	CFRelease(ringtones);
	CFRelease(ringtoneNameStr);
	return true;
}

CFDictionaryRef PhoneInteraction::getUserRingtoneDictionary()
{
	char *value = getPhoneProductVersion();

	if (!strncmp(value, "1.0", 3)) {
		return NULL;
	}

	// read in the user ringtone plist file
	const char *ringtoneConfigFilePath = "/private/var/root/Media/iTunes_Control/iTunes/Ringtones.plist";
	
	CFDictionaryRef ringtoneConfigFileContents = NULL;

	if (fileExists(ringtoneConfigFilePath)) {

		// file exists, read it into a dictionary
		unsigned char *buf;
		int bufsize;
		
		if (!getFileData((void**)&buf, &bufsize, ringtoneConfigFilePath, 0, 0)) {
			return NULL;
		}
		
		CFDataRef pData = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)buf,
													  (CFIndex)bufsize, kCFAllocatorNull);
		
		if (pData == NULL) {
			free(buf);
			return NULL;
		}
		
		ringtoneConfigFileContents = (CFDictionaryRef)CFPropertyListCreateFromXMLData(kCFAllocatorDefault, pData,
																					  kCFPropertyListImmutable,
																					  NULL);

		if (ringtoneConfigFileContents == NULL) {
			free(buf);
			CFRelease(pData);
			return NULL;
		}
		
		free(buf);
		CFRelease(pData);
	}

	return ringtoneConfigFileContents;
}

bool PhoneInteraction::putRingtoneOnPhone(const char *ringtoneFile, const char *ringtoneName,
										  bool bInSystemDir)
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Can't write a ringtone when no phone is connected.");
		return false;
	}

	char *value = getPhoneProductVersion();

	// easier just to use iTunes in 1.1.2
	if (!strcmp(value, "1.1.2")) {
		return false;
	}

	struct afc_directory *dir;
	const char *ringtoneDir = NULL;

	if (!strncmp(value, "1.0", 3)) {

		if (bInSystemDir) {
			ringtoneDir = "/Library/Ringtones";
		}
		else {
			ringtoneDir = "/private/var/root/Library/Ringtones";
		}

	}
	else {

		if (bInSystemDir) {
			ringtoneDir = "/Library/Ringtones";
		}
		else {
			return false;
		}

	}

	if (ringtoneDir == NULL) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Couldn't determine ringtone directory.");
		return false;
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

	if (!strncmp(value, "1.0", 3)) {
		strcat(ringtonePath, ringtoneFilename);

		if (!putFile(ringtoneFile, ringtonePath)) {
			(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error putting ringtone file on phone.");
			return false;
		}

	}
	else {
		const char *ringtoneExtension = UtilityFunctions::getFileExtension(ringtoneFilename);
		int len = strlen(ringtoneFilename);
		
		if (ringtoneExtension != NULL) {
			len -= strlen(ringtoneExtension) + 1;
		}
		
		strncat(ringtonePath, ringtoneFilename, len);
		
		if (ringtoneFilename[len-1] != ' ') {
			strcat(ringtonePath, " ");
		}
		
		strcat(ringtonePath, ".m4r");
		
		if (!putFile(ringtoneFile, ringtonePath)) {
			(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error putting ringtone file on phone.");
			return false;
		}

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

bool PhoneInteraction::putApplicationOnPhone(const char *sourceDir, const char *destName)
{
	char destPath[PATH_MAX+1];
	strcpy(destPath, "/Applications/");

	if (destName != NULL) {
		int len = strlen(destName);
		strcat(destPath, destName);

		if ( (len < 4) || strncmp(destName + len - 4, ".app", 4) ) {
			strcat(destPath, ".app");
		}

	}
	else {
		const char *appName = UtilityFunctions::getLastPathElement(sourceDir);
		strcat(destPath, appName);
	}

	if (!putFileRecursive(sourceDir, destPath)) {
		(*m_notifyFunc)(NOTIFY_PUTFILE_FAILED, "Error putting application on phone.");
		return false;
	}

	(*m_notifyFunc)(NOTIFY_PUTFILE_SUCCESS, "Successfully put application on phone.");
	return true;
}

bool PhoneInteraction::removeRingtone(const char *ringtoneFilename, bool bInSystemDir)
{
	const char *ringtoneDir = NULL;
	char ringtonePath[PATH_MAX+1];
	char *value = getPhoneProductVersion();

	// easier just to use iTunes in 1.1.2
	if (!strcmp(value, "1.1.2")) {
		return false;
	}

	if (!strncmp(value, "1.0", 3)) {
		
		if (bInSystemDir) {
			ringtoneDir = "/Library/Ringtones";
		}
		else {
			ringtoneDir = "/private/var/root/Library/Ringtones";
		}
		
	}
	else {

		if (bInSystemDir) {
			ringtoneDir = "/Library/Ringtones";
		}
		else {
			return false;
		}

	}

	strcpy(ringtonePath, ringtoneDir);
	strcat(ringtonePath, "/");
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

bool PhoneInteraction::ringtoneExists(const char *ringtoneName, bool bInSystemDir)
{
	char *value = getPhoneProductVersion();

	if (!strncmp(value, "1.0", 3)) {
		const char *ringtoneDir = "/var/root/Library/Ringtones/";
	
		if (bInSystemDir) {
			ringtoneDir = "/Library/Ringtones/";
		}

		char filepath[PATH_MAX+1];
		strcpy(filepath, ringtoneDir);
		strcat(filepath, ringtoneName);
		return fileExists(filepath);
	}
	else if (bInSystemDir) {
		char filepath[PATH_MAX+1];
		strcpy(filepath, "/Library/Ringtones/");
		strcat(filepath, ringtoneName);
		return fileExists(filepath);
	}

	CFStringRef ringtoneNameStr = CFStringCreateWithCString(kCFAllocatorDefault, ringtoneName, kCFStringEncodingUTF8);
	
	if (ringtoneNameStr == NULL) {
		return false;
	}

	CFDictionaryRef ringtones = getUserRingtoneDictionary();
	CFDictionaryRef contents = (CFDictionaryRef)CFDictionaryGetValue(ringtones, CFSTR("Ringtones"));
	CFIndex count = CFDictionaryGetCount(contents);
	CFTypeRef keys[count], values[count];
	CFDictionaryGetKeysAndValues(contents, keys, values);
	CFDictionaryRef dictValue;
	CFStringRef valName;
	
	for (int i = 0; i < count; i++) {
		dictValue = (CFDictionaryRef)values[i];
		valName = (CFStringRef)CFDictionaryGetValue(dictValue, CFSTR("Name"));
		
		if (valName != NULL) {
			
			if (CFStringCompare(ringtoneNameStr, valName, 0) == kCFCompareEqualTo) {
				CFRelease(ringtoneNameStr);
				CFRelease(ringtones);
				return true;
			}
			
		}
		
	}

	CFRelease(ringtoneNameStr);
	CFRelease(ringtones);
	return false;
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

	bool retval = copyFilesystemRecursive(m_hAFC, dirpath, dirpath, dest, dest, ignoreUserFiles);

	if (retval) {
		(*m_notifyFunc)(NOTIFY_FSCOPY_SUCCESS, "Filesystem copy succeeded.");
	}

	return retval;
}

bool PhoneInteraction::copyFilesystemRecursive(afc_connection *conn, const char *phoneBasepath,
											   const char *dirpath, const char *dest,
											   const char *basepath, bool ignoreUserFiles)
{
	afc_directory *dir;

	if (AFCDirectoryOpen(conn, (char*)dirpath, &dir)) {
		(*m_notifyFunc)(NOTIFY_FSCOPY_FAILED, "Error opening directory on phone.");
		return false;
	}

	char phoneWorkingDir[PATH_MAX+1];
	char computerWorkingDir[PATH_MAX+1];
	char *fileName = NULL;
#if defined(__APPLE__)
	int phoneBaselen = strlen(phoneBasepath);
	int baselen = strlen(basepath);
#endif

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
				char fullPath[PATH_MAX+1];
				strcpy(fullPath, phoneBasepath);

				if (fullPath[phoneBaselen-1] != '/') {
					strcat(fullPath, "/");
				}

				strcat(fullPath, g_symlinks[i]);

				if (!strcmp(fullPath, filePath)) {
					char origPath[PATH_MAX+1];
					strcpy(origPath, g_symlinkOriginals[i]);

					char linkPath[PATH_MAX+1];
					strcpy(linkPath, basepath);

					if (linkPath[baselen-1] != '/') {
						strcat(linkPath, "/");
					}

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
						
						if (!strncmp(filePath, "/private/var/root/Media", 23) ||
							!strncmp(filePath, "/var/root/Media/private/var/root/Media", 46) ||
							!strncmp(filePath, "/var/root/Media/private/var/root/backup", 47) ) {
							ignore = true;
						}
						else if (!strncmp(filePath, "/private/var/root/Library", 25) ||
								 !strncmp(filePath, "/var/root/Library/private/var/root/Library", 50) ) {
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
						
						if (!copyFilesystemRecursive(conn, phoneBasepath, filePath, newPath, basepath,
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

bool PhoneInteraction::renamePath(const char *oldpath, const char *newpath)
{
	
	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_RENAME_PATH_FAILED, "Can't rename a file/directory when no phone is connected.");
		return false;
	}
	
	if (AFCRenamePath(m_hAFC, (char*)oldpath, (char*)newpath)) {
		(*m_notifyFunc)(NOTIFY_RENAME_PATH_FAILED, "Error renaming file/directory.");
		return false;
	}

	(*m_notifyFunc)(NOTIFY_RENAME_PATH_SUCCESS, "Successfully renamed file/directory.");
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

void PhoneInteraction::performNewJailbreak(const char *modifiedServicesPath)
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Can't perform a jailbreak when no phone is connected.");
		return;
	}

	// check to ensure they've already performed the hack upgrade to 1.1.x
	if (!fileExists("disk")) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error, couldn't perform jailbreak.  You need to downgrade to version 1.0.2, then perform the special firmware (see Help documentation for more details).");
		return;
	}

	if (m_statusFunc) {
		(*m_statusFunc)("Performing new jailbreak...", false);
	}

	(*m_notifyFunc)(NOTIFY_JAILBREAK_RECOVERY_WAIT, "Waiting for jail break...");

	afc_file_ref rAFC;

	if (AFCFileRefOpen(m_hAFC, "disk", 1, &rAFC)) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't open /private/var/root/Media/disk on phone.");
		return;
	}

	// Since we allow people to jailbreak and rejail, the search patterns can occur in
	// multiple files (eg. memory cache files, etc).  So we need to make sure we modify
	// all instances we find of them.
	std::list<unsigned long long> fstabOffsets, servicesOffsets;

	// TODO: A bit of a hack.  Is there any way to determine the disk size?
	unsigned int disksize = 1024 * 1024 * 300;

	// use a 4 MB memory buffer to read the disk
	unsigned int bufsize = 1024 * 1024 * 4;
	unsigned char *buf = (unsigned char*)malloc(bufsize);
	unsigned long long mainoffset = 0;

	// fstab search string
	char *searchstr1 = "/dev/disk0s1 / hfs ro";

	// Services.plist search string
	char *searchstr2 = "<key>com.apple.afc</key>\n";

	unsigned int searchstrlen1 = strlen(searchstr1);
	unsigned int searchstrlen2 = strlen(searchstr2);
	int searchoffset1 = 0, searchoffset2 = 0;

	while (mainoffset < disksize) {

#ifdef DEBUG
		printf("Reading from offset %d...\n", mainoffset);
#endif

		if (AFCFileRefRead(m_hAFC, rAFC, buf, &bufsize)) {
			AFCFileRefClose(m_hAFC, rAFC);
			free(buf);
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't read from /private/var/root/Media/disk.");
			return;
		}

		void *ptr = memchr(buf, searchstr1[searchoffset1], bufsize);

		while (ptr != NULL) {
			unsigned long long bufoffset = (unsigned long long)((unsigned int)ptr - (unsigned int)buf);
			unsigned int bufleft = bufsize - bufoffset;
			unsigned int searchleft = searchstrlen1 - searchoffset1;

			if (bufleft < searchleft) {

				if (!memcmp(ptr, searchstr1+searchoffset1, bufleft)) {
					searchoffset1 += bufleft;
					break;
				}

			}
			else if (!memcmp(ptr, searchstr1+searchoffset1, searchleft)) {
				fstabOffsets.push_back(mainoffset + bufoffset - searchoffset1);
				ptr = (char*)ptr + (searchleft - 1);
				searchoffset1 = 0;
				bufoffset += (searchleft - 1);
			}

			ptr = memchr(((char*)ptr)+1, searchstr1[searchoffset1], bufsize - (bufoffset+1));
		}

		ptr = memchr(buf, searchstr2[searchoffset2], bufsize);

		while (ptr != NULL) {
			unsigned long long bufoffset = (unsigned long long)((unsigned int)ptr - (unsigned int)buf);
			unsigned int bufleft = bufsize - bufoffset;
			unsigned int searchleft = searchstrlen2 - searchoffset2;

			if (bufleft < searchleft) {
					
				if (!memcmp(ptr, searchstr2+searchoffset2, bufleft)) {
					searchoffset2 += bufleft;
					break;
				}
					
			}
			else if (!memcmp(ptr, searchstr2+searchoffset2, searchleft)) {
				servicesOffsets.push_back(mainoffset + bufoffset - searchoffset2);
				ptr = (char*)ptr + (searchleft - 1);
				searchoffset2 = 0;
				bufoffset += (searchleft - 1);
			}
				
			ptr = memchr(((char*)ptr)+1, searchstr2[searchoffset2], bufsize - (bufoffset+1));
		}

		mainoffset += bufsize;
	}

	free(buf);

	if (!fstabOffsets.size() && !servicesOffsets.size()) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't find files to modify on /private/var/root/Media/disk.  Perhaps your phone is already jailbroken?");
		return;
	}

	std::list<unsigned long long>::iterator iter;
	std::list<unsigned char*> fstabBuffers;
	unsigned long long modulus;
	unsigned char *tmpBuf;

	bufsize = 4096;

	for (iter = fstabOffsets.begin(); iter != fstabOffsets.end(); iter++) {

		// disk reads need to be performed on 1k boundaries
		modulus = (*iter) % 1024;

		if ( modulus ) {
			(*iter) -= modulus;
		}

		if (AFCFileRefSeek(m_hAFC, rAFC, (*iter), 0)) {
			AFCFileRefClose(m_hAFC, rAFC);
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't seek to correct position in /private/var/root/Media/disk.");
			return;
		}

		tmpBuf = (unsigned char*)malloc(bufsize);

		if (AFCFileRefRead(m_hAFC, rAFC, tmpBuf, &bufsize)) {
			AFCFileRefClose(m_hAFC, rAFC);
			FreePointerList(fstabBuffers);
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't read from /private/var/root/Media/disk.");
			return;
		}

		// change mount type from ro to rw
		tmpBuf[modulus+20] = 'w';
		fstabBuffers.push_back(tmpBuf);
	}

	std::list<unsigned char*> servicesBuffers;
	const char *newStr = "<key>com.apple.afc</key><dict><key>AllowUnactivatedService</key><true/><key>Label</key><string>com.apple.afc</string><key>ProgramArguments</key><array><string>/usr/libexec/afcd</string><string>--lockdown</string><string>-d</string><string>/</string></array></dict><key>com.apple.crashreportcopy</key><dict>";
	int newStrLen = strlen(newStr);

	for (iter = servicesOffsets.begin(); iter != servicesOffsets.end(); iter++) {

		// disk reads need to be performed on 1k boundaries
		modulus = (*iter) % 1024;
		
		if ( modulus ) {
			(*iter) -= modulus;
		}
		
		if (AFCFileRefSeek(m_hAFC, rAFC, (*iter), 0)) {
			AFCFileRefClose(m_hAFC, rAFC);
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't seek to correct position in /private/var/root/Media/disk.");
			return;
		}
		
		tmpBuf = (unsigned char*)malloc(bufsize);
		
		if (AFCFileRefRead(m_hAFC, rAFC, tmpBuf, &bufsize)) {
			AFCFileRefClose(m_hAFC, rAFC);
			FreePointerList(fstabBuffers);
			FreePointerList(servicesBuffers);
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't read from /private/var/root/Media/disk.");
			return;
		}

		// modify AFC service so that it has access to the full filesystem
		memcpy(tmpBuf+modulus, (void*)newStr, newStrLen);
		servicesBuffers.push_back(tmpBuf);
	}

	if (AFCFileRefClose(m_hAFC, rAFC)) {
		FreePointerList(fstabBuffers);
		FreePointerList(servicesBuffers);
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error closing /private/var/root/Media/disk.");
		return;
	}
	
	if (AFCFileRefOpen(m_hAFC, "disk", 3, &rAFC)) {
		FreePointerList(fstabBuffers);
		FreePointerList(servicesBuffers);
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't open /private/var/root/Media/disk on phone.");
		return;
	}

	std::list<unsigned char*>::iterator iter2;

	for (iter = fstabOffsets.begin(), iter2 = fstabBuffers.begin(); (iter != fstabOffsets.end()) && (iter2 != fstabBuffers.end()); iter++, iter2++) {

		if (AFCFileRefSeek(m_hAFC, rAFC, (*iter), 0)) {
			AFCFileRefClose(m_hAFC, rAFC);
			FreePointerList(fstabBuffers);
			FreePointerList(servicesBuffers);
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't seek to correct position in /private/var/root/Media/disk for writing.");
			return;
		}
	
		if (AFCFileRefWrite(m_hAFC, rAFC, (*iter2), bufsize)) {
			AFCFileRefClose(m_hAFC, rAFC);
			FreePointerList(fstabBuffers);
			FreePointerList(servicesBuffers);
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error writing modified fstab file to /private/var/root/Media/disk.");
			return;
		}

	}

	FreePointerList(fstabBuffers);

	for (iter = servicesOffsets.begin(), iter2 = servicesBuffers.begin(); (iter != servicesOffsets.end()) && (iter2 != servicesBuffers.end()); iter++, iter2++) {

		if (AFCFileRefSeek(m_hAFC, rAFC, (*iter), 0)) {
			AFCFileRefClose(m_hAFC, rAFC);
			FreePointerList(servicesBuffers);
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error.  Couldn't seek to correct position in /private/var/root/Media/disk for writing.");
			return;
		}
		
		if (AFCFileRefWrite(m_hAFC, rAFC, (*iter2), bufsize)) {
			AFCFileRefClose(m_hAFC, rAFC);
			FreePointerList(servicesBuffers);
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error writing modified Services.plist file to /private/var/root/Media/disk.");
			return;
		}
		
	}

	FreePointerList(servicesBuffers);

	if (AFCFileRefClose(m_hAFC, rAFC)) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error closing /private/var/root/Media/disk.");
		return;
	}

	(*m_notifyFunc)(NOTIFY_NEW_JAILBREAK_STAGE_ONE_WAIT, "Waiting for reboot...");

	if (m_servicesPath != NULL) {
		free(m_servicesPath);
	}

	m_servicesPath = (char*)malloc(strlen(modifiedServicesPath)+1);
	strcpy(m_servicesPath, modifiedServicesPath);

	m_performingNewJailbreak = true;
}

void PhoneInteraction::performJailbreak(const char *firmwarePath, const char *modifiedFstabPath,
										const char *modifiedServicesPath)
{

	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Can't perform a jailbreak when no phone is connected.");
		return;
	}

	char *phoneProdVer = getPhoneProductVersion();

	if (!strncmp(phoneProdVer, "1.1", 3)) {
		performNewJailbreak(modifiedServicesPath);
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

	if (!enableThirdPartyApplications(true)) {
		(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error disabling 3rd party application support.");
		return;
	}
	
	if (!enableCustomRingtones(true)) {
		(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error disabling ringtone customization.");
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

void PhoneInteraction::newJailbreakStageTwo()
{
	m_performingNewJailbreak = false;

	if (!putFile(m_servicesPath, "/System/Library/Lockdown/Services.plist", 0, 0)) {
		free(m_servicesPath);
		m_servicesPath = NULL;
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error writing modified Services.plist to phone.");
		return;
	}

	free(m_servicesPath);
	m_servicesPath = NULL;

	if (!enableThirdPartyApplications()) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error enabling 3rd party application support.");
		return;
	}

	if (!enableCustomRingtones()) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error enabling ringtone customization.");
		return;
	}

	(*m_notifyFunc)(NOTIFY_NEW_JAILBREAK_STAGE_TWO_WAIT, "Waiting for reboot...");
	
	m_recoveryOccurred = true;
	m_finishingJailbreak = true;
}

void PhoneInteraction::returnToJailFinished()
{
	m_returningToJail = false;

	if (isPhoneJailbroken()) {
		(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Return to jail failed.");
	}
	else {
		(*m_notifyFunc)(NOTIFY_JAILRETURN_SUCCESS, "Return to jail succeeded!");
	}

}

void PhoneInteraction::jailbreakFinished()
{
	m_finishingJailbreak = false;
	
	if (isPhoneJailbroken()) {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_SUCCESS, "Jailbreak succeeded!");
	}
	else {
		(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Jailbreak failed.");
	}

}

bool PhoneInteraction::isPhoneJailbroken()
{
	return m_jailbroken;
}

void PhoneInteraction::recoveryModeStarted_dfu(struct am_recovery_device *rdev)
{
	char dfuFile[PATH_MAX+1];
	unsigned int len = strlen(m_firmwarePath);

	memset(dfuFile, 0, PATH_MAX+1);
	strcpy(dfuFile, m_firmwarePath);
	
	if (dfuFile[len-1] != '/') {
		strcat(dfuFile, "/");
	}
	
	strcat(dfuFile, "Firmware/dfu/WTF.s5l8900xall.RELEASE.dfu");

	// check the dfu version
	struct stat st;
	
	if (stat(dfuFile, &st) == -1) {
		(*m_notifyFunc)(NOTIFY_DFU_FAILED, "Unknown firmware version.");
		return;
	}
	
	CFStringRef cfDFU = CFStringCreateWithCString(kCFAllocatorDefault, dfuFile,
												  kCFStringEncodingUTF8);
	
	if (cfDFU == NULL) {
		(*m_notifyFunc)(NOTIFY_DFU_FAILED, "Error creating path CFString.");
		return;
	}
	
	// send DFU file
	if (PI_sendFileToDevice(rdev, cfDFU)) {
		(*m_notifyFunc)(NOTIFY_DFU_FAILED, "Error sending DFU file to device.");
		return;
	}
	
	// load ramdisk
	if (PI_sendCommandToDevice(rdev, CFSTR("go"))) {
		(*m_notifyFunc)(NOTIFY_DFU_FAILED, "Error sending go command to device.");
		return;
	}

}

void PhoneInteraction::recoveryModeStarted(struct am_recovery_device *rdev)
{
	m_inRecoveryMode = true;
	m_recoveryDevice = rdev;
	(*m_notifyFunc)(NOTIFY_RECOVERY_CONNECTED, "Recovery mode started");

	if ( m_enteringRecoveryMode ) {
		m_enteringRecoveryMode = false;
		(*m_notifyFunc)(NOTIFY_RECOVERY_SUCCESS, "Successfully entered recovery mode");
		return;
	}
	else if ( m_enteringDFUMode ) {
		recoveryModeStarted_dfu(rdev);
		return;
	}
	else if ( m_finishingJailbreak || m_returningToJail ) {
		
		if (!m_recoveryOccurred) {
			m_recoveryOccurred = true;
			exitRecoveryMode(rdev);
		}

		return;
	}
	else if ( !m_waitingForRecovery ) {

		// try once to save them from recovery mode
		if (g_recoveryAttempts++ == 0) {
			exitRecoveryMode(rdev);
		}

		return;
	}

	m_waitingForRecovery = false;

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
	if (PI_sendCommandToDevice(rdev, CFSTR("setenv boot-args rd=md0 -v"))) {
	//if (PI_sendCommandToDevice(rdev, CFSTR("setenv boot-args rd=md0 -progress"))) {
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
	(*m_notifyFunc)(NOTIFY_RECOVERY_DISCONNECTED, "Recovery mode ended");
}

void PhoneInteraction::enterRecoveryMode()
{
	
	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_RECOVERY_FAILED, "Can't enter DFU mode when no phone is connected.");
		return;
	}
	
	m_enteringRecoveryMode = true;
	
	if (AMDeviceEnterRecovery(m_iPhone)) {
		(*m_notifyFunc)(NOTIFY_RECOVERY_FAILED, "Error entering recovery mode.");
		return;
	}
	
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

	// clear boot args (not needed)
	/*
	if (PI_sendCommandToDevice(dev, CFSTR("setenv boot-args"))) {
		
		if (m_finishingJailbreak) {
			(*m_notifyFunc)(NOTIFY_JAILBREAK_FAILED, "Error setting boot args.");
		}
		else if (m_returningToJail) {
			(*m_notifyFunc)(NOTIFY_JAILRETURN_FAILED, "Error setting boot args.");
		}
		
		return;
	}
	 */

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
	(*m_notifyFunc)(NOTIFY_RESTORE_CONNECTED, "Restore mode started");

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

	(*m_notifyFunc)(NOTIFY_RESTORE_DISCONNECTED, "Restore mode ended");
}

void PhoneInteraction::enterDFUMode(const char *firmwarePath)
{
	
	if (!isConnected()) {
		(*m_notifyFunc)(NOTIFY_DFU_FAILED, "Can't enter DFU mode when no phone is connected.");
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
		(*m_notifyFunc)(NOTIFY_DFU_FAILED, "Error entering recovery mode.");
		return;
	}
	
	m_recoveryOccurred = false;
	m_waitingForRecovery = true;
	m_enteringDFUMode = true;
	(*m_notifyFunc)(NOTIFY_DFU_RECOVERY_WAIT, "Entering DFU mode...");
}

void PhoneInteraction::dfuModeStarted(am_recovery_device *dev)
{
	m_inDFUMode = true;
	m_dfuDevice = dev;

	(*m_notifyFunc)(NOTIFY_DFU_CONNECTED, "DFU mode started");

	if (m_enteringDFUMode) {
		m_enteringDFUMode = false;
		(*m_notifyFunc)(NOTIFY_DFU_SUCCESS, "Successfully entered DFU mode");
	}

}

void PhoneInteraction::dfuModeFinished(am_recovery_device *dev)
{
	m_inDFUMode = false;
	m_dfuDevice = NULL;

	(*m_notifyFunc)(NOTIFY_DFU_DISCONNECTED, "DFU mode ended");
}
