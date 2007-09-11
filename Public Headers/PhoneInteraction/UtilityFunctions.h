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


class UtilityFunctions
{

public:

	// Used to get the last path element from a given path (eg. /var/tmp -> tmp)
	static const char *getLastPathElement(const char *path);

	// These functions are used to generate an activation record or request
	// from a device ID, IMEI, ICCID, and PEM file
	static bool generateActivationRecord(CFDictionaryRef *activationRecord, const char *pemfile,
										 const char *deviceid, const char *imei, const char *iccid);
	static bool generateActivationRequest(CFDictionaryRef *activationrequest,
										  const char *pemfile, const char *deviceid,
										  const char *imei, const char *iccid);

};
