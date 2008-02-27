/*
 * optivate - A small tool to patch / unpatch all known versions of lockdownd
 *
 * Version 1.00
 * Created by The Operator
 *
 */
#include <stdio.h>


enum
{
	VERSION_100 = 0,
	VERSION_101,
	VERSION_102,
	VERSION_111,
	VERSION_112,
	VERSION_113,
	VERSION_114
};

unsigned int NUMPATCHES[7] = { 6, 6, 6, 5, 6, 17, 17 };

unsigned int OFFSETS[7][17] =
{
	{
		0x90A4, 0x90A7, 0x90A8, 0x90AB, 0x90B3, 0x9263, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x94C4, 0x94C7, 0x94C8, 0x94CB, 0x94D3, 0x9683, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x94F0, 0x94F3, 0x94F4, 0x94F7, 0x94FF, 0x96AF, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0xB810, 0xB812, 0xB813, 0xB814, 0xB818, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x4B3B, 0xC5C8, 0xC5CA, 0xC5CB, 0xC5CC, 0xC5D4, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x83AF, 0xAFA3, 0xC4CF, 0xCDB4, 0xCDB5, 0xCDC0, 0xCE08, 0xCE58,
		0xCE59, 0xCF24, 0xCF7C, 0xCF7D, 0xD000, 0xD1A8, 0xD1A9, 0xD224,
		0xD274
	},
	{
		0x83AF, 0xAFA3, 0xC4CF, 0xCDB4, 0xCDB5, 0xCDC0, 0xCE08, 0xCE58,
		0xCE59, 0xCF24, 0xCF7C, 0xCF7D, 0xD000, 0xD1A8, 0xD1A9, 0xD224,
		0xD274
	}
};

unsigned char PATCHES[7][17] =
{
	{
		0x00, 0xE3, 0x68, 0xE5, 0xEA, 0xEA, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x00, 0xE3, 0x68, 0xE5, 0xEA, 0xEA, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x00, 0xE3, 0x68, 0xE5, 0xEA, 0xEA, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x00, 0xA0, 0xE1, 0x54, 0x00, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0xEA, 0x00, 0xA0, 0xE1, 0x00, 0xEC, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0xEA, 0xEA, 0xEA, 0x04, 0x29, 0x00, 0xB0, 0x60,
		0x28, 0x94, 0x3C, 0x27, 0xB8, 0x10, 0x25, 0x94,
		0x00
	},
	{
		0xEA, 0xEA, 0xEA, 0x04, 0x29, 0x00, 0xB0, 0x60,
		0x28, 0x94, 0x3C, 0x27, 0xB8, 0x10, 0x25, 0x94,
		0x00
	}
};

unsigned char UNDO_PATCHES[7][17] =
{
	{
		0x01, 0x03, 0x3C, 0x05, 0x0A, 0x0A, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x01, 0x03, 0x3C, 0x05, 0x0A, 0x0A, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x01, 0x03, 0x3C, 0x05, 0x0A, 0x0A, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x04, 0x00, 0x1A, 0x24, 0x01, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x1A, 0x04, 0x00, 0x1A, 0x01, 0x88, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0
	},
	{
		0x0A, 0x0A, 0x1A, 0x80, 0x28, 0x01, 0x2C, 0xDC,
		0x27, 0x3C, 0xF4, 0x26, 0x70, 0x8C, 0x24, 0x4C,
		0x01
	},
	{
		0x0A, 0x0A, 0x1A, 0x80, 0x28, 0x01, 0x2C, 0xDC,
		0x27, 0x3C, 0xF4, 0x26, 0x70, 0x8C, 0x24, 0x4C,
		0x01
	}
};

int patchIt(int undo, int version, const char *path)
{

	if ( (version < VERSION_100) || (version > VERSION_114) ) {
		return 0;
	}

	FILE *fp = NULL;

	if (!(fp = fopen(path, "r+"))) {
		perror("fopen");
		return 0;
	}

	unsigned int i;

	for (i = 0; i < NUMPATCHES[version]; i++) {

		if (fseek(fp, OFFSETS[version][i], 0) == -1) {
			perror("fseek");
			fclose(fp);
			return 0;
		}

		if (undo) {

			if (fwrite(UNDO_PATCHES[version]+i, 1, 1, fp) == 0) {
				perror("fwrite");
				fclose(fp);
				return 0;
			}

		}
		else {

			if (fwrite(PATCHES[version]+i, 1, 1, fp) == 0) {
				perror("fwrite");
				fclose(fp);
				return 0;
			}

		}

	}

	fclose(fp);
	return 1;
}

int fileExists(const char *path)
{
	FILE *fp = NULL;

	if (!(fp = fopen(path, "r"))) {
		perror("fopen");
		return 0;
	}

	fclose(fp);
	return 1;
}

void printUsage(const char *progName)
{
	printf("Usage: %s [100 | 101 | 102 | 111 | 112 | 113 | 114] <path to lockdownd file> <undo>\n", progName);
}

int main(int argc, char **argv)
{
	printf("optivate v1.00\n");
	printf("Created by The Operator\n");

	int undo = 0;

	if (argc < 3) {
		printUsage(argv[0]);
		return -1;
	}
	else if (argc > 3) {

		if (!strncmp(argv[3], "undo", 4)) {
			undo = 1;
		}
		else {
			printUsage(argv[0]);
			return -1;
		}

	}

	int version = -1;

	if (!strncmp(argv[1], "100", 3)) {
		version = VERSION_100;
	}
	else if (!strncmp(argv[1], "101", 3)) {
		version = VERSION_101;
	}
	else if (!strncmp(argv[1], "102", 3)) {
		version = VERSION_102;
	}
	else if (!strncmp(argv[1], "111", 3)) {
		version = VERSION_111;
	}
	else if (!strncmp(argv[1], "112", 3)) {
		version = VERSION_112;
	}
	else if (!strncmp(argv[1], "113", 3)) {
		version = VERSION_113;
	}
	else if (!strncmp(argv[1], "114", 3)) {
		version = VERSION_114;
	}

	if (version == -1) {
		printUsage(argv[0]);
		return -1;
	}

	if (!fileExists(argv[2])) {
		printf("File %s doesn't exist.  Exiting...\n", argv[2]);
		return -1;
	}

	if (!patchIt(undo, version, argv[2])) {
		printf("Patching failed.  Exiting...\n");
		return -1;
	}

	printf("Patching succeeded.  Phone is now factory activated.\n");
	return 0;
}
