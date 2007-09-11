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
#include "base64.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static int PIGetIndexOfClosingTag(char *buf, int bufsize, const char *tokid)
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
		unsigned int offset1 = (unsigned int)buf;
		unsigned int offset2 = (unsigned int)tmp2;
		return (int)(offset2 - offset1);
	}

	while ( (tmp != NULL) && (tmp2 != NULL) && (tmp2 > tmp) ) {
		tmp = strstr(tmp2 + strlen(close), open);
		tmp2 = strstr(tmp2 + strlen(close), close);
	}

	if (tmp2 == NULL) return -1;

	if ( (tmp == NULL) || (tmp2 < tmp) ) {
		unsigned int offset1 = (unsigned int)buf;
		unsigned int offset2 = (unsigned int)tmp2;
		return (int)(offset2 - offset1);
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

#ifdef DEBUG
	printf("token id: %s\n", id);
#endif

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
#ifdef DEBUG
				printf("  keyvalue: %s=%s\n", keybuf, valuebuf);
#endif

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
#ifdef DEBUG
			printf("  keyvalue: %s=%s\n", keybuf, valuebuf);
#endif

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
	int bytesleft = size, closeindex;
	bool bClosed, bAllocatedKey = false;

	while (bytesleft) {

		// skip whitespace
		if (isspace(buf[0])) {
			buf++;
			bytesleft--;
			continue;
		}

		bClosed = false;
		closeindex = -1;

		// get XML token
		if (!GetNextToken(&buf, &bytesleft, &tok, &bClosed)) {

			if (bAllocatedKey) free(key);

			return false;
		}

		// closed XML token?
		if (bClosed) {

			if (key == NULL) continue;

			if (*dict == NULL) {

				if (bAllocatedKey) free(key);

				return false;
			}

			CFBooleanRef value = NULL;

			if (!strcasecmp(tok.id, "true")) {
				value = kCFBooleanTrue;
			}
			else if (!strcasecmp(tok.id, "false")) {
				value = kCFBooleanFalse;
			}

			if (value != NULL) {
				CFStringRef cfkey = CFStringCreateWithCString(kCFAllocatorDefault,
															  key, kCFStringEncodingUTF8);

				if (bAllocatedKey) {
					free(key);
					key = NULL;
					bAllocatedKey = false;
				}

				if (cfkey == NULL) {
					return false;
				}

				CFDictionaryAddValue(*dict, cfkey, value);
			}
			else if (bAllocatedKey) {
				free(key);
				key = NULL;
				bAllocatedKey = false;
			}

			continue;
		}

		// check for token types we know about
		if (!strcasecmp(tok.id, "key")) {

			if (bAllocatedKey) free(key);

			if (*dict == NULL) return false;

			closeindex = PIGetIndexOfClosingTag(buf, bytesleft, "key");
			
			if (closeindex == -1) return false;
			
			key = (char*)malloc(closeindex+1);
			strncpy(key, buf, closeindex);
			key[closeindex] = 0;
			StripWhitespace(&key);
#ifdef DEBUG
			printf("key = %s\n", key);
#endif

			// empty keys are not allowed
			if (strlen(key) == 0) {
				free(key);
				return false;
			}

			bAllocatedKey = true;
			buf += closeindex + 6;
			bytesleft -= closeindex + 6;
		}
		else if (!strcasecmp(tok.id, "dict")) {
			
			if (key == NULL) {
				
				if (*dict != NULL) return false;
				
				closeindex = PIGetIndexOfClosingTag(buf, bytesleft, "dict");
				
				if (closeindex == -1) {
					return false;
				}
				
				*dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
												  &kCFTypeDictionaryKeyCallBacks,
												  &kCFTypeDictionaryValueCallBacks);
				
				if (*dict == NULL) {
					return false;
				}
				
				if (!CreateDictionaryFromXMLRecursive(buf, closeindex, dict, NULL)) {
					CFRelease(*dict);
					return false;
				}
				
				return true;
			}
			else if (*dict != NULL) {
				closeindex = PIGetIndexOfClosingTag(buf, bytesleft, "dict");
				
				if (closeindex == -1) {

					if (bAllocatedKey) free(key);

					return false;
				}
				
				CFMutableDictionaryRef dict2 = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
																		 &kCFTypeDictionaryKeyCallBacks,
																		 &kCFTypeDictionaryValueCallBacks);
				
				if (dict2 == NULL) {

					if (bAllocatedKey) free(key);

					return false;
				}
				
				if (!CreateDictionaryFromXMLRecursive(buf, closeindex, &dict2, NULL)) {

					if (bAllocatedKey) free(key);

					CFRelease(dict2);
					return false;
				}
				
				CFStringRef cfkey = CFStringCreateWithCString(kCFAllocatorDefault,
															  key, kCFStringEncodingUTF8);

				if (bAllocatedKey) {
					free(key);
					key = NULL;
					bAllocatedKey = false;
				}

				if (cfkey == NULL) {
					CFRelease(dict2);
					return false;
				}
				
				CFDictionaryAddValue(*dict, cfkey, dict2);
				
				buf += closeindex + 7;
				bytesleft -= closeindex + 7;
			}
			else {

				if (bAllocatedKey) free(key);

				return false;
			}
			
		}
		else if (!strcasecmp(tok.id, "data")) {
			
			if ( (key == NULL) || (*dict == NULL) ) return false;
			
			closeindex = PIGetIndexOfClosingTag(buf, bytesleft, "data");
			
			if (closeindex == -1) {

				if (bAllocatedKey) free(key);

				return false;
			}
			
			CFStringRef cfkey = CFStringCreateWithCString(kCFAllocatorDefault,
														  key, kCFStringEncodingUTF8);

			if (bAllocatedKey) {
				free(key);
				key = NULL;
				bAllocatedKey = false;
			}
			
			if (cfkey == NULL) {
				return false;
			}
			
			char *value = (char*)malloc(closeindex+1);
			strncpy(value, buf, closeindex);
			value[closeindex] = 0;
			StripWhitespace(&value);
			
			if (strlen(value) > 0) {
				// Base64 decode the data
				int decodelen = Base64decode_len(value);
				
				if (decodelen > 0) {
					char *decodedval = (char*)malloc(decodelen);
					Base64decode(decodedval, value);
					free(value);
					value = decodedval;
				}
				
			}
			
			CFDataRef cfvalue = CFDataCreate(kCFAllocatorDefault, (const UInt8*)value, strlen(value));
			free(value);
			
			if (cfvalue == NULL) {
				return false;
			}
			
			CFDictionaryAddValue(*dict, cfkey, cfvalue);
			
			buf += closeindex + 7;
			bytesleft -= closeindex + 7;
		}
		else if (!strcasecmp(tok.id, "string")) {
			
			if ( (key == NULL) || (*dict == NULL) ) return false;
			
			closeindex = PIGetIndexOfClosingTag(buf, bytesleft, "string");
			
			if (closeindex == -1) {

				if (bAllocatedKey) free(key);

				return false;
			}
			
			CFStringRef cfkey = CFStringCreateWithCString(kCFAllocatorDefault,
														  key, kCFStringEncodingUTF8);

			if (bAllocatedKey) {
				free(key);
				key = NULL;
				bAllocatedKey = false;
			}
			
			if (cfkey == NULL) {
				return false;
			}
			
			char *value = (char*)malloc(closeindex+1);
			strncpy(value, buf, closeindex);
			value[closeindex] = 0;
			StripWhitespace(&value);
#ifdef DEBUG
			printf("string = %s\n", value);
#endif

			CFStringRef cfvalue = CFStringCreateWithCString(kCFAllocatorDefault,
															value, kCFStringEncodingUTF8);
			free(value);
			
			if (cfvalue == NULL) {
				return false;
			}
			
			CFDictionaryAddValue(*dict, cfkey, cfvalue);
			
			buf += closeindex + 9;
			bytesleft -= closeindex + 9;
		}
		else if (!strcasecmp(tok.id, "?xml") || !strcasecmp(tok.id, "!DOCTYPE") ||
				 !strcasecmp(tok.id, "plist")) {
			// Ignore
		}
		else {

			// unknown token -- just skip it
			if (bAllocatedKey) {
				free(key);
				key = NULL;
				bAllocatedKey = false;
			}

			closeindex = PIGetIndexOfClosingTag(buf, bytesleft, tok.id);

			if (closeindex == -1) {
				return false;
			}

			buf += closeindex + strlen(tok.id) + 3;
			bytesleft -= closeindex + strlen(tok.id) + 3;
		}

	}

	if (bAllocatedKey) free(key);

	return true;
}

CFDictionaryRef PICreateDictionaryFromPlistFile(const char *file)
{
	struct stat st;

	// check that file exists
	if (stat(file, &st) == -1) {
		return NULL;
	}

	if (st.st_size == 0) {
		return NULL;
	}

	// read in the file
	FILE *fp = fopen(file, "r");

	if (fp == NULL) {
		return NULL;
	}

	char *data = (char*)malloc(st.st_size);
	off_t count = 0;
	size_t retval;

	while (count < st.st_size) {

		if ( (retval = fread(data + count, 1, st.st_size - count, fp)) == 0 ) {
			break;
		}

		count += retval;
	}

	fclose(fp);

	if (count < st.st_size) {
		free(data);
		return NULL;
	}

	// get the dictionary from the file data
	int bytesleft = st.st_size;
	CFMutableDictionaryRef dict = NULL;

	if (CreateDictionaryFromXMLRecursive(data, bytesleft, &dict, NULL) == false) {
		free(data);
		return NULL;
	}

	free(data);
	return dict;
}

static bool writeDictToFileRecursive(CFDictionaryRef dict, int level, FILE *fp)
{
	for (int i = 0; i < level; i++) fwrite("\t", 1, 1, fp);
	fwrite("<dict>\n", 1, 7, fp);

	CFIndex len = CFDictionaryGetCount(dict);

	if (len == 0) {
		for (int i = 0; i < level; i++) fwrite("\t", 1, 1, fp);
		fwrite("</dict>\n", 1, 8, fp);

		return true;
	}

	CFStringRef *keys = (CFStringRef*)malloc(len * sizeof(CFStringRef));
	CFTypeRef *values = (CFTypeRef*)malloc(len * sizeof(CFTypeRef));

	CFDictionaryGetKeysAndValues(dict, (const void**)keys, (const void**)values);

	for (CFIndex ci = 0; ci < len; ci++) {
		for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
		fwrite("<key>", 1, 5, fp);

		CFIndex cflen = CFStringGetLength(keys[ci]);

		if (cflen > 0) {
			char buf[cflen+1];

			if (CFStringGetCString(keys[ci], buf, cflen+1, kCFStringEncodingUTF8) == false) {
				free(keys);
				free(values);
				return false;
			}

			fwrite(buf, 1, cflen, fp);
		}

		fwrite("</key>\n", 1, 7, fp);
		CFTypeID valtype = CFGetTypeID(values[ci]);

		if (valtype == CFStringGetTypeID()) {
			for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
			fwrite("<string>", 1, 8, fp);

			cflen = CFStringGetLength((CFStringRef)values[ci]);

			if (cflen > 0) {
				char buf[cflen+1];
				
				if (CFStringGetCString((CFStringRef)values[ci], buf, cflen+1, kCFStringEncodingUTF8) == false) {
					free(keys);
					free(values);
					return false;
				}
				
				fwrite(buf, 1, cflen, fp);
			}

			fwrite("</string>\n", 1, 10, fp);
		}
		else if (valtype == CFDictionaryGetTypeID()) {

			if (!writeDictToFileRecursive((CFDictionaryRef)values[ci], level+1, fp)) {
				free(keys);
				free(values);
				return false;
			}

		}
		else if (valtype == CFDataGetTypeID()) {
			for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
			fwrite("<data>\n", 1, 7, fp);

			CFIndex datalen = CFDataGetLength((CFDataRef)values[ci]);

			if (datalen > 0) {
				int encodedlen = Base64encode_len((int)datalen);
				char encodeddata[encodedlen];

				Base64encode(encodeddata, (const char*)CFDataGetBytePtr((CFDataRef)values[ci]),
							 (int)datalen);

				encodedlen = strlen(encodeddata);
				int count = 0;

				while (count < encodedlen) {
					for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);

					if ( (encodedlen-count) > 60 ) {
						fwrite(encodeddata+count, 1, 60, fp);
						count += 60;
					}
					else {
						fwrite(encodeddata+count, 1, encodedlen-count, fp);
						count = encodedlen;
					}

					fwrite("\n", 1, 1, fp);
				}

			}

			for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
			fwrite("</data>\n", 1, 8, fp);
		}
		else if (valtype == CFBooleanGetTypeID()) {

			if (CFBooleanGetValue((CFBooleanRef)values[ci]) == true) {
				for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
				fwrite("<true/>\n", 1, 8, fp);
			}
			else {
				for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
				fwrite("<false/>\n", 1, 9, fp);
			}

		}
		else if (valtype == CFArrayGetTypeID()) {
			// TODO: Array output is not supported yet
			for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
			fwrite("<array>\n", 1, 8, fp);
			for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
			fwrite("</array>\n", 1, 9, fp);
		}
		else if (valtype == CFDateGetTypeID()) {
			// TODO: Date output is not supported yet
			for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
			fwrite("<date>\n", 1, 7, fp);
			for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
			fwrite("</date>\n", 1, 8, fp);
		}
		else if (valtype == CFNumberGetTypeID()) {
			// TODO: Number output is not supported yet
			for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
			fwrite("<real>\n", 1, 7, fp);
			for (int i = 0; i <= level; i++) fwrite("\t", 1, 1, fp);
			fwrite("</real>\n", 1, 8, fp);
		}
		else {
			// unknown type
			free(keys);
			free(values);
			return false;
		}

	}

	free(keys);
	free(values);

	for (int i = 0; i < level; i++) fwrite("\t", 1, 1, fp);
	fwrite("</dict>\n", 1, 8, fp);

	return true;
}

bool PICreatePlistFileFromDictionary(CFDictionaryRef dict, const char *file)
{
	FILE *fp = fopen(file, "w");

	if (fp == NULL) {
		return false;
	}

	fwrite("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", 1, 39, fp);
	fwrite("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n", 1, 112, fp);
	fwrite("<plist version=\"1.0\">\n", 1, 22, fp);

	if (!writeDictToFileRecursive(dict, 0, fp)) {
		fclose(fp);
		remove(file);
		return false;
	}

	fwrite("</plist>\n", 1, 9, fp);
	fflush(fp);
	fclose(fp);
	return true;
}
