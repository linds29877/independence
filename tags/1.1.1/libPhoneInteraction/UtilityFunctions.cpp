/*
 *  UtilityFunctions.cpp
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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>

#include "PhoneInteraction/UtilityFunctions.h"

#if defined(WIN32)
#define PATH_MAX 1024
#endif


bool UtilityFunctions::findDeviceID(char *buf)
{
#if defined(WIN32)
	char *homedir = getenv("APPDATA");
#else
	char *homedir = getenv("HOME");
#endif
	int len = strlen(homedir);

	if ( len == 0 ) return false;

	char path[PATH_MAX];
	strcpy(path, homedir);

#if defined(WIN32)
	strcat(path, "\\Apple Computer\\Lockdown");
#else
	strcat(path, "/Library/Lockdown");
#endif

	struct stat statbuf;

	if ( stat(path, &statbuf) < 0 ) return false;

	if ( S_ISDIR(statbuf.st_mode) == 0 ) return false;

	DIR *dp = opendir(path);

	if ( dp == NULL ) return false;

	struct dirent *dirp;

	while ( (dirp = readdir(dp)) != NULL ) {
		int len = strlen(dirp->d_name);

		if (len < 7) continue;

		if (strcmp(".plist", dirp->d_name + (len - 6))) continue;

		strncpy(buf, dirp->d_name, len - 6);
		buf[len-6] = 0;
		break;
	}

	closedir(dp);
	return true;
}

bool UtilityFunctions::validateIMEI(const char *imei)
{
	int len = 0;

	while (*(imei + len)) {
		len++;
	}

	if (len != 15) return false;

	return true;
}

bool UtilityFunctions::validateICCID(const char *iccid)
{
	int len = 0;

	while (*(iccid + len)) {
		len++;
	}

	if ( (len < 6) || (len > 20) ) return false;

	return true;
}

const char *UtilityFunctions::getLastPathElement(const char *path)
{
	const char *filename = path;
	int len = strlen(path);

	if (len > 0) {
		int count = 1;
		filename = path + (len-count);

		while ( (count < len) && (*filename != '/') ) {
			count++;
			filename = path + (len-count);
		}

		if (*filename == '/') {
			return filename + 1;
		}

	}

	return filename;
}

void UtilityFunctions::generateAccountToken(char *token, const char *deviceid,
											const char *imei, const char *iccid)
{
	sprintf(token, "{\n\t\"UniqueDeviceID\" = \"%s\";\n\t\"InternationalMobileEquipmentIdentity\" = \"%s\";\n\t\"IntegratedCircuitCardIdentity\" = \"%s\";\n}\n",
			  deviceid, imei, iccid);
}

bool UtilityFunctions::generateAccountTokenSignature(CFDataRef *signature,
													 const char *token,
													 const char *pemfile)
{
	FILE *fp = fopen(pemfile, "r");

	if (fp == NULL) {
		return false;
	}

	ERR_load_crypto_strings();
	EVP_PKEY *pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
	fclose(fp);

	if (pkey == NULL) {
		ERR_print_errors_fp(stderr);
		return false;
	}

	EVP_MD_CTX md_ctx;

	EVP_SignInit(&md_ctx, EVP_sha1());
	EVP_SignUpdate(&md_ctx, token, strlen(token));

	unsigned char sig_buf[4096];
	unsigned int sig_len = 4096;

	if (EVP_SignFinal(&md_ctx, sig_buf, &sig_len, pkey) != 1) {
		ERR_print_errors_fp(stderr);
		return false;
	}

	*signature = CFDataCreate(NULL, sig_buf, sig_len);

	if (*signature == NULL) {
		return false;
	}

	return true;
}

bool UtilityFunctions::generateActivationRecord(CFDictionaryRef *activationrecord,
												CFDataRef token, CFDataRef signature)
{
	const void *keys[] = {
		CFSTR("AccountToken"),
		CFSTR("AccountTokenCertificate"),
		CFSTR("AccountTokenSignature"),
		CFSTR("DeviceCertificate"),
		CFSTR("FairPlayKeyData")
	};
	const void *values[] = {
		token,
		CFDataCreate(NULL, NULL, 0),
		signature,
		CFDataCreate(NULL, NULL, 0),
		CFDataCreate(NULL, NULL, 0)
	};

	*activationrecord = CFDictionaryCreate( NULL, keys, values, 5,
											&kCFCopyStringDictionaryKeyCallBacks,
											&kCFTypeDictionaryValueCallBacks);

	if (*activationrecord == NULL) return false;

	return true;
}

bool UtilityFunctions::generateActivationRequest(CFDictionaryRef *activationrequest,
												CFDictionaryRef activationrecord)
{
	const void *keys[] = { CFSTR("ActivationRecord"), CFSTR("Request") };
	const void *values[] = { activationrecord, CFSTR("Activate") };
	*activationrequest = CFDictionaryCreate( NULL, keys, values, 2,
											 &kCFCopyStringDictionaryKeyCallBacks,
											 &kCFTypeDictionaryValueCallBacks);											 

	if (*activationrequest == NULL) return false;
	
	return true;
}

bool UtilityFunctions::generateActivationRequest_All(CFDictionaryRef *activationrequest,
													 const char *pemfile, const char *deviceid,
													 const char *imeiVal, const char *iccidVal,
													 void (*notifyFunc)(int, const char*))
{
	char token[1024];
	UtilityFunctions::generateAccountToken(token, deviceid, imeiVal, iccidVal);

	CFDataRef tokensignature;
	
	if (!UtilityFunctions::generateAccountTokenSignature(&tokensignature, token,
														 pemfile)) {

		if (notifyFunc != NULL) {
			(*notifyFunc)(NOTIFY_ACTIVATION_GEN_FAILED, "Error occurred while generating token signature");
		}

		return false;
	}
	
	CFDataRef accounttoken = CFDataCreate(NULL, (const UInt8*)token, strlen(token));
	
	if (accounttoken == NULL) {
		CFRelease(tokensignature);

		if (notifyFunc != NULL) {
			(*notifyFunc)(NOTIFY_ACTIVATION_GEN_FAILED, "Error occurred while creating the account token");
		}

		return false;
	}
	
	CFDictionaryRef activationrecord;
	
	if (!UtilityFunctions::generateActivationRecord(&activationrecord, accounttoken,
													tokensignature)) {
		CFRelease(tokensignature);
		CFRelease(accounttoken);

		if (notifyFunc != NULL) {
			(*notifyFunc)(NOTIFY_ACTIVATION_GEN_FAILED, "Error occurred while creating the activation record");
		}

		return false;
	}

	if (!UtilityFunctions::generateActivationRequest(activationrequest, activationrecord)) {
		CFRelease(tokensignature);
		CFRelease(accounttoken);
		CFRelease(activationrecord);

		if (notifyFunc != NULL) {
			(*notifyFunc)(NOTIFY_ACTIVATION_GEN_FAILED, "Error occurred while creating the activation request");
		}

		return false;
	}

	if (notifyFunc != NULL) {
		(*notifyFunc)(NOTIFY_ACTIVATION_GEN_SUCCESS, "Activation gen succeeded!");
	}

	return true;
}
