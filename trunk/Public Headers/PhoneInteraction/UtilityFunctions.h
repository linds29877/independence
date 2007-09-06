/*
 *  UtilityFunctions.h
 *  libPhoneInteraction
 *
 *  Created by The Operator on 19/07/07.
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

#pragma once

#if defined(WIN32)
#include <CoreFoundation.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif


enum {
	NOTIFY_ACTIVATION_GEN_SUCCESS = 200,
	NOTIFY_ACTIVATION_GEN_FAILED
};


class UtilityFunctions
{

public:

	// Used to find the device ID from the lockdown file
	static bool findDeviceID(char *buf);

	// Used to validate an IMEI
	static bool validateIMEI(const char *imei);

	// Used to validate an ICCID
	static bool validateICCID(const char *iccid);

	// Used to get the last path element from a given path (eg. /var/tmp -> tmp)
	static const char *getLastPathElement(const char *path);

	// These functions are used together to generate an activation plist dictionary
	// from a device ID, IMEI, ICCID, and PEM file
	static void generateAccountToken(char *token, const char *deviceid,
									const char *imei, const char *iccid);
	static bool generateAccountTokenSignature(CFDataRef *signature,
											  const char *token, const char *pemfile);
	static bool generateActivationRecord(CFDictionaryRef *activationrecord,
										 CFDataRef token, CFDataRef signature);
	static bool generateActivationRequest(CFDictionaryRef *activationrequest,
										  CFDictionaryRef activationrecord);
	static bool generateActivationRequest_All(CFDictionaryRef *activationrequest,
											  const char *pemfile, const char *deviceid,
											  const char *imei, const char *iccid,
											  void (*notifyFunc)(int, const char*));

};
