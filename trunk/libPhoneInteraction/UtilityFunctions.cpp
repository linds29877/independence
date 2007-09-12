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
#include "base64.h"


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

	char *dev_cert_b64 = "LS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSURQekNDQXFpZ0F3SUJBZ0lLQTUrbnZmT2paZlFjQkRBTkJna3Foa2lHOXcwQkFRc0ZBREJhTVFzd0NRWUQKVlFRR0V3SlZVekVUTUJFR0ExVUVDaE1LUVhCd2JHVWdTVzVqTGpFVk1CTUdBMVVFQ3hNTVFYQndiR1VnYVZCbwpiMjVsTVI4d0hRWURWUVFERXhaQmNIQnNaU0JwVUdodmJtVWdSR1YyYVdObElFTkJNQjRYRFRBM01EZ3hNVEF6Ck1Ua3dNbG9YRFRFd01EZ3hNVEF6TVRrd01sb3dnWWN4TVRBdkJnTlZCQU1US0RZMk5qRTNabVptT0RNeU1Ua3kKWmpBeU5XUXlabUV4TVdFNU1UaGpPVFk0TVdaaVpEQTRZMk14Q3pBSkJnTlZCQVlUQWxWVE1Rc3dDUVlEVlFRSQpFd0pEUVRFU01CQUdBMVVFQnhNSlEzVndaWEowYVc1dk1STXdFUVlEVlFRS0V3cEJjSEJzWlNCSmJtTXVNUTh3CkRRWURWUVFMRXdacFVHaHZibVV3Z1o4d0RRWUpLb1pJaHZjTkFRRUJCUUFEZ1kwQU1JR0pBb0dCQU1MdkRIU0sKb2F0ZFI0aC94WVZWZ1M2S3JZN0gzUHEzK3kwQ21nbkdMOHBuNWVqaFdURmIrYlp5bktIbUZjTVBMcWVCUUFaWQoxMWVHWFhqWVVnSkExVG1DY1FmKytRS1RzQ2NnUHJ4S2N6S3IwaUxPa1liQkE4K29FQ2R0ZjA3OXB6YjEwaVQwCk1pTlJ3Y0tPU3pwMnRFWXlaWFBBZUJtdG0zL3dnb3JVMjdpTEFnTUJBQUdqZ2Qwd2dkb3dnWUlHQTFVZEl3UjcKTUhtQUZMTCtJU05FaHBWcWVkV0JKbzV6RU5pblRJNTBvVjZrWERCYU1Rc3dDUVlEVlFRR0V3SlZVekVUTUJFRwpBMVVFQ2hNS1FYQndiR1VnU1c1akxqRVZNQk1HQTFVRUN4TU1RWEJ3YkdVZ2FWQm9iMjVsTVI4d0hRWURWUVFECkV4WkJjSEJzWlNCcFVHaHZibVVnUkdWMmFXTmxJRU5CZ2dFQk1CMEdBMVVkRGdRV0JCU1dDTkw3VU5JU3BMZ1gKZkQ3ejZIallZeTZ0elRBTUJnTlZIUk1CQWY4RUFqQUFNQTRHQTFVZER3RUIvd1FFQXdJRm9EQVdCZ05WSFNVQgpBZjhFRERBS0JnZ3JCZ0VGQlFjREFUQU5CZ2txaGtpRzl3MEJBUXNGQUFPQmdRQWhKTEVKN2I2WVZoTGFhQlVTCm50NTFZQUtBVVZEb1B0bVRrTHpNSmJuMXlSNVVSYnpuMjRiQW9vZlNrWFBDem14cHU2R2NJVkdqU3hRTmFOUXQKS2ZKcEkxa2dhMHBhNjhlb1NDc1FodVh3OHVPSmVzUVAwQjdScEs3Rk1QVkFlVnQxVWpVSk83QldyUkJDNTZKVgp3blpobjlSK3NNUGF5TlJFelBHT29hRjBVUT09Ci0tLS0tRU5EIENFUlRJRklDQVRFLS0tLS0K";
	int decodelen = Base64decode_len(dev_cert_b64);
	char *dev_cert = (char*)malloc(decodelen);

	Base64decode(dev_cert, dev_cert_b64);
	CFDataRef devicecertificate = CFDataCreate(NULL, (const UInt8*)dev_cert, decodelen);
	free(dev_cert);

	if (devicecertificate == NULL) {
		CFRelease(tokensignature);
		CFRelease(accounttoken);
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
		devicecertificate,
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
