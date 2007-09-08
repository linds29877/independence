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
#include <sys/types.h>
#include <sys/stat.h>


typedef struct st_xml_keyval_pair
{
	char *key;
	char *value;
} xml_keyval_pair;

typedef struct st_xml_token
{
	char *id;
	xml_keyval_pair *keyvals;
	int numpairs;
} xml_token;


static void StripWhitespace(char **buf)
{
	int len = strlen(*buf);

	if (len == 0) return;

	char newbuf[len+1];
	int count = 0, index = 0;

	while (count < len) {

		if (!isspace((*buf)[count])) {
			newbuf[index++] = (*buf)[count];
		}

		count++;
	}

	newbuf[index] = 0;
	strcpy(*buf, newbuf);
}

static int GetIndexOfClosingTag(char *buf, int bufsize, const char *tokid)
{
	int len = strlen(tokid);
	char open[len+2];
	char close[len+3];

	open[0] = '<';
	strcpy(open + 1, tokid);

	close[0] = '<';
	close[1] = '/';
	strcpy(close + 2, tokid);

	char *tmp2 = strstr(buf, close);

	if (tmp2 == NULL) return -1;

	char *tmp = strstr(buf, open);

	if ( (tmp == NULL) || (tmp2 < tmp) ) {
		return tmp2 - buf;
	}

	while ( (tmp != NULL) && (tmp2 != NULL) && (tmp2 > tmp) ) {
		tmp = strstr(tmp2 + strlen(close), open);
		tmp2 = strstr(tmp2 + strlen(close), close);
	}

	if (tmp2 == NULL) return -1;

	if ( (tmp == NULL) || (tmp2 < tmp) ) {
		return tmp2 - buf;
	}

	return -1;
}

static bool GetNextToken(char **pData, int *pDatalen, xml_token *tok, bool *bClosed)
{
	int count = 0, idlen = 0, datalen = *pDatalen;
	bool bReadingToken = false;
	char *data = *pData, *id;

	// read the token name
	while (count < datalen) {

		if (bReadingToken) {

			if (isspace(data[count]) || data[count] == '>') {
				bReadingToken = false;
				break;
			}

			if (idlen == 0) {
				id = (char*)malloc(2);
			}
			else {
				id = (char*)realloc(id, idlen + 2);
			}

			id[idlen++] = data[count];
			id[idlen] = 0;
			count++;
		}
		else {

			if (data[count++] != '<') {
				continue;
			}

			bReadingToken = true;
		}

	}

	if (idlen == 0) return false;

	printf("token id: %s\n", id);
	tok->id = id;
	tok->keyvals = NULL;
	tok->numpairs = 0;

	// ignore content of comment style tokens
	if (id[0] == '!') {

		while (count < datalen) {

			if (data[count++] == '>') {
				*pData = *pData + count;
				*pDatalen = *pDatalen - count;

				if (data[count-2] == '/') {
					*bClosed = true;
				}
				else {
					*bClosed = false;
				}

				return true;
			}

		}

		return false;
	}

	bool bReadingKey = true, bReadingValue = false, bKeyValueRead = false, bInQuotes = false;
	char *keybuf, *valuebuf;
	int keybufsize = 0, valuebufsize = 0;

	// read the key/value pairs
	while (count < datalen) {

		if ( isspace(data[count]) && !bInQuotes ) {
			count++;
			continue;
		}

		if ( (data[count] == '>') && !bInQuotes ) {

			if (bReadingKey && (keybufsize > 0)) {
				free(keybuf);
			}
			else if (bReadingValue && (valuebufsize > 0)) {
				printf("  keyvalue: %s=%s\n", keybuf, valuebuf);
				
				if (tok->numpairs == 0) {
					tok->keyvals = (xml_keyval_pair*)malloc(sizeof(xml_keyval_pair));
				}
				else {
					tok->keyvals = (xml_keyval_pair*)realloc(tok->keyvals,
															 (tok->numpairs+1) * sizeof(xml_keyval_pair));
				}
				
				tok->keyvals[tok->numpairs].key = keybuf;
				tok->keyvals[tok->numpairs].value = valuebuf;
				tok->numpairs += 1;
				keybufsize = 0;
				valuebufsize = 0;
			}

			break;
		}

		if (bReadingKey) {

			if ( (data[count] == '"') || (data[count] =='\'') ) {

				if (bInQuotes) {
					bInQuotes = false;
					bReadingKey = false;
				}
				else {
					bInQuotes = true;
				}

			}
			else if ( (data[count] == '=') && !bInQuotes ) {
				bReadingKey = false;
				bReadingValue = true;
			}
			else {

				if (keybufsize == 0) {
					keybuf = (char*)malloc(2);
				}
				else {
					keybuf = (char*)realloc(keybuf, keybufsize + 2);
				}

				keybuf[keybufsize++] = data[count];
				keybuf[keybufsize] = 0;
			}

		}
		else if (bReadingValue) {
			
			if ( (data[count] == '"') || (data[count] =='\'') ) {
				
				if (bInQuotes) {
					bInQuotes = false;
					bReadingValue = false;
					bKeyValueRead = true;
				}
				else {
					bInQuotes = true;
				}
				
			}
			else if ( (data[count] == '=') && !bInQuotes ) {
				bReadingValue = false;
				bKeyValueRead = true;
			}
			else {
				
				if (valuebufsize == 0) {
					valuebuf = (char*)malloc(2);
				}
				else {
					valuebuf = (char*)realloc(valuebuf, valuebufsize + 2);
				}

				valuebuf[valuebufsize++] = data[count];
				valuebuf[valuebufsize] = 0;
			}
			
		}
		else if (data[count] == '=') {
			bReadingValue = true;
		}

		if (bKeyValueRead) {
			printf("  keyvalue: %s=%s\n", keybuf, valuebuf);

			if (tok->numpairs == 0) {
				tok->keyvals = (xml_keyval_pair*)malloc(sizeof(xml_keyval_pair));
			}
			else {
				tok->keyvals = (xml_keyval_pair*)realloc(tok->keyvals,
														 (tok->numpairs+1) * sizeof(xml_keyval_pair));
			}

			tok->keyvals[tok->numpairs].key = keybuf;
			tok->keyvals[tok->numpairs].value = valuebuf;
			tok->numpairs += 1;
			keybufsize = 0;
			valuebufsize = 0;
			bKeyValueRead = false;
		}

		count++;
	}

	// find the closing brace
	while (count < datalen) {

		if (data[count++] == '>') {
			*pData = *pData + count;
			*pDatalen = *pDatalen - count;

			if (data[count-2] == '/') {
				*bClosed = true;
			}
			else {
				*bClosed = false;
			}

			return true;
		}

	}

	return false;
}

static bool CreateDictionaryFromXMLRecursive(char *data, int size, CFMutableDictionaryRef *dict,
											 char *key)
{

	if (size == 0) return true;

	xml_token tok;
	char *buf = data;
	int bytesleft = size;
	bool bClosed;

	while (bytesleft) {
		bClosed = false;

		if (!GetNextToken(&buf, &bytesleft, &tok, &bClosed)) return false;

		if (bClosed) continue;

		int len = strlen(tok.id);

		switch (len)
		{
			case 3:
			{

				if (!strcasecmp(tok.id, "key")) {

					if ( (key != NULL) || (*dict == NULL) ) return false;

					int index = GetIndexOfClosingTag(buf, bytesleft, "key");

					if (index == -1) {
						return false;
					}

					char keybuf[index+1];
					strncpy(keybuf, buf, index);
					keybuf[index] = 0;
					StripWhitespace((char**)&keybuf);

					// empty keys are not allowed
					if (strlen(keybuf) == 0) return false;

					return CreateDictionaryFromXMLRecursive(buf, index, dict, keybuf);
				}

				break;
			}
			case 4:
			{

				if (!strcasecmp(tok.id, "dict")) {

					if (key == NULL) {

						if (*dict != NULL) return false;

						int index = GetIndexOfClosingTag(buf, bytesleft, "dict");

						if (index == -1) {
							return false;
						}

						*dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
														  &kCFTypeDictionaryKeyCallBacks,
														  &kCFTypeDictionaryValueCallBacks);
						
						if (*dict == NULL) {
							return false;
						}

						if (!CreateDictionaryFromXMLRecursive(buf, index, dict, NULL)) {
							CFRelease(*dict);
							return false;
						}

						return true;
					}
					else if (*dict != NULL) {
						int index = GetIndexOfClosingTag(buf, bytesleft, "dict");
						
						if (index == -1) {
							return false;
						}
						
						CFMutableDictionaryRef dict2 = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
																				 &kCFTypeDictionaryKeyCallBacks,
																				 &kCFTypeDictionaryValueCallBacks);
						
						if (dict2 == NULL) {
							return false;
						}

						if (!CreateDictionaryFromXMLRecursive(buf, index, &dict2, key)) {
							CFRelease(dict2);
							return false;
						}

						CFStringRef cfkey = CFStringCreateWithCString(kCFAllocatorDefault,
																	  key, kCFStringEncodingUTF8);

						if (cfkey == NULL) {
							CFRelease(dict2);
							return false;
						}

						CFDictionaryAddValue(*dict, cfkey, dict2);
						return true;
					}
					else {
						return false;
					}

				}
				else if (!strcasecmp(tok.id, "data")) {
					
					if ( (key == NULL) || (*dict == NULL) ) return false;

					int index = GetIndexOfClosingTag(buf, bytesleft, "data");

					if (index == -1) {
						return false;
					}

					char value[index+1];
					strncpy(value, buf, index);
					value[index] = 0;
					StripWhitespace((char**)&value);

					CFStringRef cfkey = CFStringCreateWithCString(kCFAllocatorDefault,
																  key, kCFStringEncodingUTF8);
					
					if (cfkey == NULL) {
						return false;
					}

					// TODO: Do I need to base64 decode the data here?
					CFDataRef cfvalue = CFDataCreate(kCFAllocatorDefault, (const UInt8*)value, strlen(value));

					if (cfvalue == NULL) {
						return false;
					}
					
					CFDictionaryAddValue(*dict, cfkey, cfvalue);
					return true;
				}
			
				break;
			}
			case 6:
			{

				if (!strcasecmp(tok.id, "string")) {
					
					if ( (key == NULL) || (*dict == NULL) ) return false;

					int index = GetIndexOfClosingTag(buf, bytesleft, "string");

					if (index == -1) {
						return false;
					}

					char value[index+1];
					strncpy(value, buf, index);
					value[index] = 0;
					StripWhitespace((char**)&value);

					CFStringRef cfkey = CFStringCreateWithCString(kCFAllocatorDefault,
																  key, kCFStringEncodingUTF8);
					
					if (cfkey == NULL) {
						return false;
					}
					
					CFStringRef cfvalue = CFStringCreateWithCString(kCFAllocatorDefault,
																	value, kCFStringEncodingUTF8);

					if (cfvalue == NULL) {
						return false;
					}

					CFDictionaryAddValue(*dict, cfkey, cfvalue);
					return true;
				}

				break;
			}
			default:
			{
				break;
			}

		}

		int index = GetIndexOfClosingTag(buf, bytesleft, tok.id);
		
		if (index == -1) return false;

		buf += (index + len + 3);
		bytesleft -= (index + len + 3);
	}

	return true;
}

CFDictionaryRef PICreateDictionaryFromPlistFile(const char *file)
{
	struct stat st;

	if (stat(file, &st) == -1) {
		return NULL;
	}

	if (st.st_size == 0) {
		return NULL;
	}

	FILE *fp = fopen(file, "r");

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

	// ignore the header
	xml_token tok;
	int bytesleft = st.st_size;
	bool bClosed;

	while (bytesleft > 0) {

		if (!GetNextToken((char**)&data, &bytesleft, &tok, &bClosed)) return NULL;

		if (!strcmp(tok.id, "plist")) {
			break;
		}

	}

	if (bytesleft <= 0) return NULL;

	// now get the main dictionary
	CFMutableDictionaryRef dict = NULL;

	if (CreateDictionaryFromXMLRecursive(data, bytesleft, &dict, NULL) == false) {
		CFRelease(dict);
		return NULL;
	}

	return dict;
}
