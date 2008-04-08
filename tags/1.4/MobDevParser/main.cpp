#include <stdio.h>
/*
 *  main.cpp
 *  MobDevParser
 *
 *  Created by The Operator on 02/04/08
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
#include "MobDevInternals.h"


bool fileExists(const char *file)
{
	FILE *fp;

	if ( (fp = fopen(file, "r")) == NULL ) {
		return false;
	}

	fclose(fp);
	return true;
}

void printUsage(const char *progname)
{
	printf("Usage: %s -arch [ppc or intel] MobDevFile\n", progname);
}

int main (int argc, const char **argv)
{

	if (argc < 2) {
		printUsage(argv[0]);
		return -1;
	}

	int arch = ARCH_NATIVE;
	int index = 1;

	if (argc > 2) {

		if (argc < 4) {
			printUsage(argv[0]);
			return -1;
		}
		
		if (!strcmp(argv[1], "-arch")) {

			if (!strcmp(argv[2], "ppc")) {
				arch = ARCH_PPC;
			}
			else if (!strcmp(argv[2], "intel")) {
				arch = ARCH_INTEL;
			}
			else {
				printUsage(argv[0]);
				return -1;
			}

			index = 3;
		}
		else {
			printUsage(argv[0]);
			return -1;
		}

	}

	if (!fileExists(argv[index])) {
		printf("Can't open file %s.\n", argv[index]);
		return -1;
	}

	MobDevInternals mdi(arch, argv[index]); 

	if (mdi.IsInitialized()) {

		switch (arch) {
			case ARCH_NATIVE:
				printf("Private function offsets for native architecture:\n");
				break;
			case ARCH_PPC:
				printf("Private function offsets for PPC architecture:\n");
				break;
			case ARCH_INTEL:
				printf("Private function offsets for Intel architecture:\n");
				break;
			default:
				break;
		}

		mdi.PrintPrivateFunctionOffsets();
	}
	else {
		printf("Error gettting private function info.\n");
		return -1;
	}

	return 0;
}
