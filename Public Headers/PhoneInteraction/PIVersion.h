/*
 *  PIVersion.h
 *  libPhoneInteraction
 *
 *  Created by The Operator on 15/10/07.
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

#ifdef WIN32
#include <CoreFoundation.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif


typedef struct pi_version
{
	int major;
	int minor;
	int point;
} PIVersion;


bool ConvertCStringToPIVersion(const char *string, PIVersion *version);
bool ConvertCFStringToPIVersion(CFStringRef string, PIVersion *version, FILE *fp = NULL);
