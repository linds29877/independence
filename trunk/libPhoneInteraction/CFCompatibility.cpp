/*
 *  CFCompatibility.cpp
 *  libPhoneInteraction
 *
 *  Created by The Operator on 05/09/07.
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

#include "CFCompatibility.h"


#if 0
static void GetNextToken(const char *data, int datalen, char *tok, int *len)
{
}

static bool CreateDictionaryFromXMLRecursive(const char *data, int size, CFMutableDictionaryRef dict,
											 char *key)
{

	if (size == 0) return true;

	char *tok;
	int len;

	GetNextToken(data, size, tok, &len);

	if (tok == NULL) return false;

	switch (len)
	{
		case 3:
		{

			if (!strncasecmp(tok, "key", 3)) {
			}

			break;
		}
		case 4:
		{
			break;
		}
		case 5:
		{
			break;
		}
		case 6:
		{
			break;
		}
		case 8:
		{
			break;
		}
		default:
		{
			break;
		}

	}

	if (!strncasecmp(tok, "?xml", 4)) {
	}
	else if (!strncasecmp(tok, "!DOCTYPE", )) {
	}
	else if (!strcasecmp("plist")) {
	}
	else if (!strcasecmp("dict")) {
	}
	else if (!strcasecmp("data")) {
	}
	else if (!strcasecmp("string")) {
	}

}

CFDictionaryRef PICreateDictionaryFromXMLFile(const char *file)
{
	struct stat st;

	if (stat(file, &st) == -1) {
		return NULL;
	}

	if (st.st_size == 0) {
		return NULL;
	}

	FILE *fp = fopen(file);

	if (fp == NULL) {
		return NULL;
	}

	char data[st.st_size];
	size_t count = 0, offset = 0, retval;

	while (count < st.st_size) {

		if ( (retval = fread(data + offset, 1, st.st_size - count, fp)) == 0 ) {
			break;
		}

		count += retval;
	}

	fclose(fp);

	if (count < st.st_size) {
		return NULL;
	}

	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
															&kCFTypeDictionaryKeyCallBacks,
															&kCFTypeDictionaryValueCallBacks);

	if (dict == NULL) {
		return NULL;
	}

	if (CreateDictionaryFromXMLRecursive(data, st.st_size, dict, NULL) == false) {
		CFRelease(dict);
		return NULL;
	}

	return dict;
}
#else
CFDictionaryRef PICreateDictionaryFromXMLFile(const char */*file*/)
{
	return NULL;
}
#endif
