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
#include <cstdlib>
#include <iostream>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>

#include "PhoneInteraction/UtilityFunctions.h"


using namespace std;


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

bool UtilityFunctions::generateActivationRecord(CFDictionaryRef *activationRecord, const char *pemfile,
												const char *deviceid, const char *imei, const char *iccid)
{
	char token[1024];
	sprintf(token, "{\n\t\"UniqueDeviceID\" = \"%s\";\n\t\"InternationalMobileEquipmentIdentity\" = \"%s\";\n\t\"IntegratedCircuitCardIdentity\" = \"%s\";\n}\n", deviceid, imei, iccid);

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
	
	CFDataRef tokensignature = CFDataCreate(NULL, sig_buf, sig_len);
	
	if (tokensignature == NULL) {
		return false;
	}

	CFDataRef accounttoken = CFDataCreate(NULL, (const UInt8*)token, strlen(token));
	
	if (accounttoken == NULL) {
		CFRelease(tokensignature);
		return false;
	}
	
	const void *keys[] = {
		CFSTR("AccountToken"),
		CFSTR("AccountTokenCertificate"),
		CFSTR("AccountTokenSignature"),
		CFSTR("DeviceCertificate"),
		CFSTR("FairPlayKeyData")
	};
	const void *values[] = {
		accounttoken,
		CFDataCreate(NULL, NULL, 0),
		tokensignature,
		CFDataCreate(NULL, NULL, 0),
		CFDataCreate(NULL, NULL, 0)
	};
	
	*activationRecord = CFDictionaryCreate( NULL, keys, values, 5,
											&kCFCopyStringDictionaryKeyCallBacks,
											&kCFTypeDictionaryValueCallBacks);
	
	if (*activationRecord == NULL) return false;

	return true;
}

bool UtilityFunctions::generateActivationRequest(CFDictionaryRef *activationrequest,
												 const char *pemfile, const char *deviceid,
												 const char *imeiVal, const char *iccidVal)
{
	CFDictionaryRef activationrecord;

	if (!UtilityFunctions::generateActivationRecord(&activationrecord, pemfile, deviceid,
													imeiVal, iccidVal)) {
		return false;
	}

	const void *keys[] = { CFSTR("ActivationRecord"), CFSTR("Request") };
	const void *values[] = { activationrecord, CFSTR("Activate") };
	*activationrequest = CFDictionaryCreate( NULL, keys, values, 2,
											 &kCFCopyStringDictionaryKeyCallBacks,
											 &kCFTypeDictionaryValueCallBacks);											 
	
	if (*activationrequest == NULL) return false;

	return true;
}
