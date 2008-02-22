/*
 *  MobDevInternals.cpp
 *  libPhoneInteraction
 *
 *  Created by The Operator on 28/01/08
 *  Copyright 2008 The Operator. All rights reserved.
 *
 *  This software is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2, as published by the Free Software Foundation.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  See the GNU General Public License version 2 for more details 
 */

#include "MobDevInternals.h"


MobDevInternals::MobDevInternals(const PIVersion& iTunesVersion)
{
	m_socketForPort = NULL;
	m_performOperation = NULL;
	m_sendCommandToDevice = NULL;
	m_sendFileToDevice = NULL;
	m_initializationError = NULL;
	m_bIsInitialized = SetupPrivateFunctions(iTunesVersion);
}

MobDevInternals::~MobDevInternals()
{

	if (m_initializationError) {
		free(m_initializationError);
		m_initializationError = NULL;
	}

}

bool MobDevInternals::IsInitialized()
{
	return m_bIsInitialized;
}

char *MobDevInternals::GetInitializationError()
{
	return m_initializationError;
}

void MobDevInternals::SetInitializationError(const char *msg)
{

	if (m_initializationError) {
		free(m_initializationError);
	}

	m_initializationError = (char*)malloc( (strlen(msg) + 1) * sizeof(char) );
	strcpy(m_initializationError, msg);
}

#if defined(WIN32)
bool MobDevInternals::SetupPrivateFunctions(const PIVersion& iTunesVersion)
{
	HMODULE hGetProcIDDLL = GetModuleHandle("iTunesMobileDevice.dll");
	
	if (!hGetProcIDDLL) {
		SetInitializationError("Error obtaining handle to iTunesMobileDevice.dll");
		return false;
	}
	
	if ( iTunesVersion.major != 7 ) {
		SetInitializationError("Error due to bad iTunes version");
		return false;
	}

	switch (iTunesVersion.minor) {
		case 6:

			switch (iTunesVersion.point) {
				case 0:
					m_sendCommandToDevice = (t_sendCommandToDevice)((char*)hGetProcIDDLL+0x00009290);
					m_sendFileToDevice = (t_sendFileToDevice)((char*)hGetProcIDDLL+0x00009410);
					m_socketForPort = (t_socketForPort)((char*)hGetProcIDDLL+0x00012830);
					m_performOperation = (t_performOperation)((char*)hGetProcIDDLL+0x000129C0);
					break;
				default:
					SetInitializationError("Error due to bad iTunes version");
					return false;
					break;
			}

			break;
		case 5:
			// iTunes 7.5 offsets submitted by David Wang
			m_sendCommandToDevice = (t_sendCommandToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FF0+0x10008160);
			m_sendFileToDevice = (t_sendFileToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FF0+0x100082E0);
			m_socketForPort = (t_socketForPort)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FF0+0x100130D0);
			m_performOperation = (t_performOperation)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FF0+0x100141C0);
			break;			
		case 4:
			m_sendCommandToDevice = (t_sendCommandToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x10008170);
			m_sendFileToDevice = (t_sendFileToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x100082F0);
			m_socketForPort = (t_socketForPort)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x10012F90);
			m_performOperation = (t_performOperation)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10008FD0+0x10014040);
			break;
		case 3:
			m_sendCommandToDevice= (t_sendCommandToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x10009290);
			m_sendFileToDevice= (t_sendFileToDevice)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x10009410);
			m_performOperation= (t_performOperation)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x100129C0);
			m_socketForPort= (t_socketForPort)((char*)GetProcAddress(hGetProcIDDLL, "AMRestorePerformRecoveryModeRestore")-0x10009F30+0x10012830);
			break;
		default:
			SetInitializationError("Error due to bad iTunes version");
			return false;
			break;
	}

	return true;
}
#elif defined(__APPLE__) && defined(__POWERPC__)
bool MobDevInternals::SetupPrivateFunctions(const PIVersion& iTunesVersion)
{
	
	if ( iTunesVersion.major != 7 ) {
		SetInitializationError("Error due to bad iTunes version");
		return false;
	}

	switch (iTunesVersion.minor) {
		case 6:

			switch (iTunesVersion.point) {
				case 1:
					m_socketForPort = (t_socketForPort)0x3c3a1470;
					m_performOperation = (t_performOperation)0x3c3a2064;
					m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a72b0;
					m_sendFileToDevice = (t_sendFileToDevice)0x3c3a7410;
					break;
				case 0:
					m_socketForPort = (t_socketForPort)0x3c3a1530;
					m_performOperation = (t_performOperation)0x3c3a2124;
					m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a72cc;
					m_sendFileToDevice = (t_sendFileToDevice)0x3c3a742c;
					break;
				default:
					SetInitializationError("Error due to bad iTunes version");
					return false;
					break;
			}

			break;
		case 5:
			// iTunes 7.5 offsets submitted by David Wang
			m_socketForPort = (t_socketForPort)0x3c3a11d8;
			m_performOperation = (t_performOperation)0x3c3a1884;
			m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a693c;
			m_sendFileToDevice = (t_sendFileToDevice)0x3c3a6a9c;
			break;
		case 4:
			m_socketForPort = (t_socketForPort)0x3c3a051c;
			m_performOperation = (t_performOperation)0x3c3a0bc8;
			m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a5bb0;
			m_sendFileToDevice = (t_sendFileToDevice)0x3c3a5d10;
			break;
		case 3:
			m_socketForPort = (t_socketForPort)0x3c3a0644;
			m_performOperation = (t_performOperation)0x3c3a0e14;
			m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a517c;
			m_sendFileToDevice = (t_sendFileToDevice)0x3c3a52dc;
			break;
		default:
			SetInitializationError("Error due to bad iTunes version");
			return false;
			break;
	}

	return true;
}
#elif defined(__APPLE__)
bool MobDevInternals::SetupPrivateFunctions(const PIVersion& iTunesVersion)
{
	
	if ( iTunesVersion.major != 7 ) {
		SetInitializationError("Error due to bad iTunes version");
		return false;
	}

	switch (iTunesVersion.minor) {
		case 6:

			switch (iTunesVersion.point) {
				case 1:
					m_socketForPort = (t_socketForPort)0x3c3a116b;
					m_performOperation = (t_performOperation)0x3c3a1c05;
					m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a71d7;
					m_sendFileToDevice = (t_sendFileToDevice)0x3c3a72fc;
					break;
				case 0:
					m_socketForPort = (t_socketForPort)0x3c3a122b;
					m_performOperation = (t_performOperation)0x3c3a1cc5;
					m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a71dd;
					m_sendFileToDevice = (t_sendFileToDevice)0x3c3a7302;
					break;
				default:
					SetInitializationError("Error due to bad iTunes version");
					return false;
					break;
			}

			break;
		case 5:
			// iTunes 7.5 offsets submitted by David Wang
			m_socketForPort = (t_socketForPort)0x3c39fcff;
			m_performOperation = (t_performOperation)0x3c3a02f5;
			m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a57a3;
			m_sendFileToDevice = (t_sendFileToDevice)0x3c3a59ef;
			break;
		case 4:
			m_socketForPort = (t_socketForPort)0x3c39ffa3;
			m_performOperation = (t_performOperation)0x3c3a0599;
			m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a597f;
			m_sendFileToDevice = (t_sendFileToDevice)0x3c3a5bcb;
			break;
		case 3:
			m_socketForPort = (t_socketForPort)0x3c39f36c;
			m_performOperation = (t_performOperation)0x3c39fa4b;
			m_sendCommandToDevice = (t_sendCommandToDevice)0x3c3a3e3b;
			m_sendFileToDevice = (t_sendFileToDevice)0x3c3a4087;
			break;
		default:
			SetInitializationError("Error due to bad iTunes version");
			return false;
			break;
	}

	return true;
}
#endif

int MobDevInternals::socketForPort(am_restore_device *rdev, unsigned int portnum)
{

	if (!m_socketForPort) return 1;

	int retval = 0;

#if defined (WIN32)
	asm("push %1\n\t"
		"push %3\n\t"
		"call *%0\n\t"
		"movl %%eax, %2"
		:
		:"m"(m_socketForPort), "g"(portnum), "m"(retval), "m"(rdev)
		:);
#elif defined (__APPLE__)
    retval = m_socketForPort(rdev, portnum);
#else
	retval = 1;
	printf("socketForPort not implemented on your platform\n");
#endif
	
	return retval;
}

int MobDevInternals::performOperation(am_restore_device *rdev, CFDictionaryRef cfdr)
{

	if (!m_performOperation) return 1;

	int retval = 0;
	
#if defined (WIN32)
	asm("movl %2, %%esi\n\t"
		"movl %3, %%ebx\n\t"
		"call *%0\n\t"
		"movl %%eax, %1"
		:
		:"m"(m_performOperation), "m"(retval), "m"(rdev), "m"(cfdr)
		:);
#elif defined (__APPLE__)
	retval = m_performOperation(rdev, cfdr);
#else
	retval = 1;
	printf("performOperation not implemented on your platform\n");
#endif
	
	return retval;      
}

int MobDevInternals::sendCommandToDevice(am_recovery_device *rdev, CFStringRef cfs)
{

	if (!m_sendCommandToDevice) return 1;

	int retval = 0;
	
#if defined (WIN32)
	asm("movl %3, %%esi\n\t"
		"push %1\n\t"
		"call *%0\n\tmovl %%eax, %2"
		:
		:"m"(m_sendCommandToDevice),  "m"(cfs), "m"(retval), "m"(rdev)
		:);
#elif defined (__APPLE__)
	retval = m_sendCommandToDevice(rdev, cfs);
#else
	retval = 1;
	printf("sendCommandToDevice not implemented on your platform\n");
#endif
	
    return retval;
}

int MobDevInternals::sendFileToDevice(am_recovery_device *rdev, CFStringRef filename)
{

	if (!m_sendFileToDevice) return 1;

	int retval = 0;
	
#if defined (WIN32)
	asm("movl %3, %%ecx\n\t"
		"push %1\n\t"
		"call *%0\n\t"
		"movl %%eax, %2"
		:
		:"m"(m_sendFileToDevice),  "m"(filename), "m"(retval), "m"(rdev)
		:);
#elif defined (__APPLE__)
	retval = m_sendFileToDevice(rdev, filename);
#else
	retval = 1;
	printf("sendFileToDevice not implemented on your platform\n");
#endif
	
	return retval;
}
