/*
 *  MobDevInternals.h
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

#pragma once

#ifdef WIN32
#include <CoreFoundation.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif


// data types from MobileDevice.h (can't directly include it here)
struct am_recovery_device;
struct am_restore_device;

// Some typedefs to make function pointer variables easier to read
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

enum
{
	ARCH_NATIVE = 0,
	ARCH_PPC,
	ARCH_INTEL
};

class MobDevInternals
{

public:
	MobDevInternals();
	MobDevInternals(unsigned int arch, const char *file);
	~MobDevInternals();

	bool IsInitialized();
	char *GetInitializationError();
	void PrintPrivateFunctionOffsets();

	int socketForPort(am_restore_device *rdev, unsigned int portnum);
	int performOperation(am_restore_device *rdev, CFDictionaryRef cfdr);
	int sendCommandToDevice(am_recovery_device *rdev, CFStringRef cfs);
	int sendFileToDevice(am_recovery_device *rdev, CFStringRef filename);

private:
	unsigned int m_arch;
	const char *m_mobDevFile;

	bool m_bIsInitialized;
	char *m_initializationError;

	unsigned char *m_mobDevBuffer;
	uint32_t m_mobDevSize;
	uint32_t m_mobDevLoadOffset;

	t_socketForPort m_socketForPort;
	t_performOperation m_performOperation;
	t_sendCommandToDevice m_sendCommandToDevice;
	t_sendFileToDevice m_sendFileToDevice;

	void SetInitializationError(const char *msg);
	bool SetupPrivateFunctions();

	void *ScanMobDevForBinaryPatterns(unsigned char *patterns, unsigned int numpats,
									  unsigned int *patlens, unsigned int totalpatbytes,
									  long int sysversion);
	void *ScanMobDevForSFP();
	void *ScanMobDevForPO();
	void *ScanMobDevForSCTD();
	void *ScanMobDevForSFTD();
	void *ScanMobDevForFunction(int func);
	bool ParseMobDevMacho32(FILE *fp);
	bool OpenMobDevLibrary();

};
