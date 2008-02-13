// geohot's 112 OTB unlocker
// This code is GPLed
//
// Spiffed up by The Operator

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <IOKit/IOKitLib.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <errno.h>
#include <mach/mach_time.h>

#define PATCHLOC 0x2359d4	// this is for 4.02.13


struct cmd_pkt {
	unsigned short int w02;
	unsigned short int cmd;
	unsigned short int data_size;
};

struct cmd_pkt_end {
	unsigned short int checksum;
	unsigned short int w03;
};

int g_fp, g_hlen, g_adrcount;
unsigned char *g_data;
struct cmd_pkt g_mycmdpkt;
struct cmd_pkt_end g_mycmdpktend;

// ----- Begin utility functions used by just about everything -----

int cmdWrite()
{
	g_mycmdpkt.w02 = 2;
	g_mycmdpktend.w03 = 3;
	g_mycmdpktend.checksum = 0;

	int i;

	for ( i = 0; i < g_mycmdpkt.data_size; i++ ) {
		g_mycmdpktend.checksum += g_data[i];
	}
	
	g_mycmdpktend.checksum += g_mycmdpkt.cmd + g_mycmdpkt.data_size;
	
	if (write(g_fp, &g_mycmdpkt, 6) < 6) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}
	
	if (write(g_fp, g_data, g_mycmdpkt.data_size) < g_mycmdpkt.data_size) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}
	
	if (write(g_fp, &g_mycmdpktend, 4) < 4) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}
	
	return 0;
}

int getCommand()	// will return when done
{
	int maxlength = 6;
	int retval;
	
	g_hlen = 0;
	
	while (g_hlen < maxlength) {
		retval = read(g_fp, g_data+g_hlen, 6);
		
		if (retval == -1) {
			fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
			return -1;
		}
		
		g_hlen += retval;
	}
	
	maxlength += (g_data[5] * 0x100) + g_data[4] + 4;	// 2 for checksum and 2 for end
	
	while (g_hlen < maxlength) {
		retval = read(g_fp, g_data+g_hlen, 0x10064-g_hlen); 
		
		if (retval == -1) {
			fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
			return -1;
		}
		
		g_hlen += retval;
	}
	
	return 0;
}

void printBuffer()
{
	printf("printBuffer(): g_hlen = %d, g_data =\n", g_hlen);

	int i;

	for ( i = 0; i < g_hlen; i++ ) {
		
		if (i != 0&&i%16==0) {
			printf("\n");
		}
		
		printf("%2.2X ", g_data[i]);
	}
	
	if (g_hlen > 0) {
		printf("\n");
	}
	
}

// ----- End utility functions used by just about everything -----

int bbWrite(unsigned int size, int print)	// put crap in data already
{

	if (print == 0) {
		printf("Writing: 0x%X\n", g_adrcount);
	}

	g_mycmdpkt.cmd = 0x804;
	g_mycmdpkt.data_size = size;

	if (cmdWrite() == -1) {
		fprintf(stderr, "cmdWrite() failed!\n");
		return -1;
	}
	
	if (getCommand() == -1) {
		fprintf(stderr, "getCommand() failed!\n");
		return -1;
	}

	if (print == 0) {
		printBuffer();
	}

	g_adrcount += size;
}


int performAddress(unsigned int addr, int print)
{
	g_adrcount = addr;

	if (print == 0) {
		printf("Address to 0x%X\n", addr);
	}

	g_mycmdpkt.cmd = 0x802;
	g_mycmdpkt.data_size = 4;
	memcpy(g_data, &addr, 4);

	if (cmdWrite() == -1) {
		fprintf(stderr, "cmdWrite() failed!\n");
		return -1;
	}
	
	if (getCommand() == -1) {
		fprintf(stderr, "getCommand() failed!\n");
		return -1;
	}
	
	if (print == 0) {
		printBuffer();
	}

	return 0;
}

int performErase(unsigned int start, unsigned int end, int debug)
{
	printf("Erasing: 0x%X-0x%X\n", start, end);

	g_mycmdpkt.cmd = 0x805;
	g_mycmdpkt.data_size = 8;

	memcpy(g_data, &start, 0x4);
	memcpy(&g_data[4], &end, 0x4);

	if (cmdWrite() == -1) {
		fprintf(stderr, "cmdWrite() failed!\n");
		return -1;
	}
	
	if (getCommand() == -1) {
		fprintf(stderr, "getCommand() failed!\n");
		return -1;
	}
	
	printBuffer();

	printf("Waiting for erase to finish...\n");

	do {
		g_mycmdpkt.cmd = 0x806;
		g_mycmdpkt.data_size = 2;
		g_data[0] = 0;
		g_data[1] = 0;

		if (cmdWrite() == -1) {
			fprintf(stderr, "cmdWrite() failed!\n");
			return -1;
		}
		
		if (getCommand() == -1) {
			fprintf(stderr, "getCommand() failed!\n");
			return -1;
		}
		
		if (debug == 0) printBuffer();

		usleep(100000);
	} while (g_data[6] == 0);

	if (debug != 0) printBuffer();

	if (g_data[9] != 0x31) {
		//printf("Erase failed!\n");
		return -1;
	}

	return 0;
}

int endSecpack()
{
	printf("Ending secpack...\n");

	g_mycmdpkt.cmd = 0x205;
	g_mycmdpkt.data_size = 2;
	g_data[0] = 0;
	g_data[1] = 0;

	if (cmdWrite() == -1) {
		fprintf(stderr, "cmdWrite() failed!\n");
		return -1;
	}
	
	if (getCommand() == -1) {
		fprintf(stderr, "getCommand() failed!\n");
		return -1;
	}
	
	printBuffer();
	return 0;
}

int sendSecpack(char *secpack)
{
	printf("Sending secpack...\n");

	g_mycmdpkt.cmd = 0x204;
	g_mycmdpkt.data_size = 0x800;

	memcpy(g_data, secpack, 0x800);

	if (cmdWrite() == -1) {
		fprintf(stderr, "cmdWrite() failed!\n");
		return -1;
	}
	
	if (getCommand() == -1) {
		fprintf(stderr, "getCommand() failed!\n");
		return -1;
	}

	printBuffer();
	return 0;
}

int cfiStage1()
{
	printf("CFI Stage 1\n");

	g_mycmdpkt.cmd = 0x84;
	g_mycmdpkt.data_size = 2;
	g_data[0] = 0;
	g_data[1] = 0;

	if (cmdWrite() == -1) {
		fprintf(stderr, "cmdWrite() failed!\n");
		return -1;
	}
	
	if (getCommand() == -1) {
		fprintf(stderr, "getCommand() failed!\n");
		return -1;
	}
	
	//printBuffer();
	return 0;
}

int cfiStage2()
{
	printf("CFI Stage 2\n");

	g_mycmdpkt.cmd = 0x85;
	g_mycmdpkt.data_size = 0;

	if (cmdWrite() == -1) {
		fprintf(stderr, "cmdWrite() failed!\n");
		return -1;
	}
	
	if (getCommand() == -1) {
		fprintf(stderr, "getCommand() failed!\n");
		return -1;
	}
	
	//printBuffer();
	return 0;
}

int increaseBaudRate()
{
	printf("Increasing baud rate...\n");
	g_mycmdpkt.cmd = 0x82;
	g_mycmdpkt.data_size = 4;
	g_data[0] = 0x00; g_data[1] = 0x10; g_data[2] = 0x0E; g_data[3] = 0x00;		//115200 bps

	if (cmdWrite() == -1) {
		fprintf(stderr, "cmdWrite() failed!\n");
		return -1;
	}

	if (getCommand() == -1) {
		fprintf(stderr, "getCommand() failed!\n");
		return -1;
	}

	printBuffer();

	struct termios options;

	if (tcgetattr(g_fp, &options) == -1) {    // baud rate upped
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	if (cfsetspeed(&options, 921600) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	if (tcsetattr(g_fp, 0, &options) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	return 0;
}

int getHeader(unsigned int timeout)
{
	fd_set nfp;
	FD_ZERO(&nfp);
	FD_SET(g_fp, &nfp);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout * 1000;

	ssize_t retval;
	g_hlen = 0;

	while ( select(g_fp+1, &nfp, 0, 0, &tv) > 0 ) {
		retval = read(g_fp, g_data+g_hlen, 0x10064-g_hlen);

		if ( retval == -1 ) {
			fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
			return -1;
		}

		g_hlen += retval;
		//printf("Attempting to read[%d]...%x %x\n",g_hlen,g_data[0],g_data[1]);
	}

	return 0;
}

int enterInteractive()
{
	struct termios options;

	if (tcgetattr(g_fp, &options) == -1) {		// baud rate upped
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	if (cfsetspeed(&options, 115200) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	if (tcsetattr(g_fp, 0, &options) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	printf("Waiting for data...\n");

	do {
		g_data[0] = 0x60;
		g_data[1] = 0x0D;

		if (write(g_fp, g_data, 2) == -1) {
			fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
			return -1;
		}

		printf("Attempt...\n");

		if (getHeader(500) == -1) {
			fprintf(stderr, "getHeader() failed!\n");
			return -1;
		}

	}
	while( (g_hlen == 0) || (g_data[0] != 0xb) );

	printf("Got Header: %d %2.2x %2.2x\n", g_hlen, g_data[0], g_data[1]);
	return 0;
}

int openPort(int speed)
{
	int fd = open("/dev/tty.baseband", O_RDWR | 0x20000 | O_NOCTTY);
	
	if (fd == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	if (ioctl(fd, 0x2000740D) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	if (fcntl(fd, 4, 0) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	struct termios term;

	if (tcgetattr(fd, &term) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	unsigned int blahnull = 0;

	// 0p - this ioctl actually fails, but we'll do it anyways
	/*
	if (ioctl(fd, 0x8004540A, &blahnull) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}
	*/
	ioctl(fd, 0x8004540A, &blahnull);

	if (cfsetspeed(&term, speed) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	cfmakeraw(&term);

	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 5;
	
	term.c_iflag = (term.c_iflag & 0xFFFFF0CD) | 5;
	term.c_oflag = term.c_oflag & 0xFFFFFFFE;
	term.c_cflag = (term.c_cflag & 0xFFFC6CFF) | 0x3CB00;
	term.c_lflag = term.c_lflag & 0xFFFFFA77;
	
	term.c_cflag = (term.c_cflag & ~CSIZE) | CS8;
	term.c_cflag &= ~PARENB;
	term.c_lflag &= ~ECHO;
	
	if (tcsetattr(fd, TCSANOW, &term) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	if (ioctl(fd, TIOCSDTR) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	if (ioctl(fd, TIOCCDTR) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	unsigned int handshake = TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR;

	if (ioctl(fd, TIOCMSET, &handshake) == -1) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		return -1;
	}

	return fd;
}

int resetBaseband()
{
	mach_port_t masterPort;

	if ( IOMasterPort(MACH_PORT_NULL, &masterPort) != KERN_SUCCESS ) {
		return -1;
	}
	
	CFMutableDictionaryRef matchingDict = IOServiceMatching("AppleBaseband");  
	
	if (!matchingDict) {
		return -1;
	}
	
	io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, matchingDict);
	
	if (!service) {
		return -1;
	}
	
	io_connect_t conn;
	
	if ( IOServiceOpen(service, mach_task_self(), 0, &conn) != KERN_SUCCESS ) {
		return -1;
	}
	
	if ( IOConnectCallScalarMethod(conn, 0, 0, 0, 0, 0) != KERN_SUCCESS ) {
		return -1;
	}
	
	IOServiceClose(conn);
	IOObjectRelease(service);
	return 0;
}

int fileExists(char *filePath)
{
	FILE *fp = fopen(filePath, "rb");

	if (fp == NULL) {
		return -1;
	}

	fclose(fp);
	return 0;
}

void printUsage(char *progName)
{
	printf("usage: %s <113secpack> <112fls>\n", progName);
}

int main(int argc, char *argv[])
{
	printf("geohot's 1.1.2 / 1.1.3 OTB unlocker -- Spiffed up by The Operator\n");

	if (argc < 3) {
		printUsage(argv[0]);
		return -1;
	}

	if (fileExists(argv[1]) == -1) {
		fprintf(stderr, "File %s doesn't exist.  Exiting...\n", argv[1]);
		return -1;
	}

	if (fileExists(argv[2]) == -1) {
		fprintf(stderr, "File %s doesn't exist.  Exiting...\n", argv[2]);
		return -1;
	}
	
	if (resetBaseband() == -1) {
		fprintf(stderr, "resetBaseband() failure!  Exiting...\n");
		return -1;
	}

	if ( (g_fp = openPort(115200)) == -1 ) {
		fprintf(stderr, "openPort() failure!  Exiting...\n");
		return -1;
	}

	if ( (g_data = (unsigned char*)malloc(70000)) == NULL ) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		close(g_fp);
		return -1;
	}
	
	if (enterInteractive() == -1) {
		fprintf(stderr, "enterInteractive() failure!  Exiting...\n");
		close(g_fp);
		free(g_data);
		return -1;
	}

	printf("Bootloader version: %s\n", &g_data[0xD]);

	// 0p - No need to check, this actually works on the older bootloader too
	/*
	if ( g_data[5] != 4 ) {
		printf("Incorrect bootloader version!  Exiting...\n");
		close(g_fp);
		free(g_data);
		return -1;
	}
	*/

	if (increaseBaudRate() == -1) {
		fprintf(stderr, "increaseBaudRate() failure!  Exiting...\n");
		close(g_fp);
		free(g_data);
		return -1;
	}

	if (cfiStage1() == -1) {
		fprintf(stderr, "cfiStage1() failure!  Exiting...\n");
		close(g_fp);
		free(g_data);
		return -1;
	}

	if (cfiStage2() == -1) {
		fprintf(stderr, "cfiStage1() failure!  Exiting...\n");
		close(g_fp);
		free(g_data);
		return -1;
	}

	char *rsecpack = (char*)malloc(0x800);

	if (rsecpack == NULL) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		close(g_fp);
		free(g_data);
		return -1;
	}

	FILE *secpack = fopen(argv[1], "rb");

	if (secpack == NULL) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		close(g_fp);
		free(g_data);
		free(rsecpack);
		return -1;
	}

	if (fread(rsecpack, 1, 0x800, secpack) < 0x800) {
		fprintf(stderr, "fread() gave short item count!  Exiting...\n");
		fclose(secpack);
		close(g_fp);
		free(g_data);
		free(rsecpack);
		return -1;
	}

	fclose(secpack);

	// Send the 1.1.3 secpack to erase 1.1.2
	if (sendSecpack(rsecpack) == -1) {
		fprintf(stderr, "sendSecpack() failure!  Exiting...\n");
		close(g_fp);
		free(g_data);
		free(rsecpack);
		return -1;
	}

	if (performErase(0xA0020000, 0xA03BFFFE, 1) == -1) {
		printf("Erase failed\n");
		printf("Hang on...we can fix that\n");

		const char efakesec[]= {
			0x00, 0x00, 0x02, 0xA0, 0x00, 0x00, 0x3D, 0x00,
			0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00 };  // full range including main fw...

		// 2nd exploit variant for >= 1.1.3
		memcpy(&rsecpack[0x780], efakesec, 0x10);

		if (sendSecpack(rsecpack) == -1) {
			fprintf(stderr, "sendSecpack() failure!  Exiting...\n");
			close(g_fp);
			free(g_data);
			free(rsecpack);
			return -1;
		}

		if (endSecpack() == -1) {
			fprintf(stderr, "endSecpack() failure!  Exiting...\n");
			close(g_fp);
			free(g_data);
			free(rsecpack);
			return -1;
		}

		if (performErase(0xA03D0000, 0xA03F0000, 1) == -1) { // the only secpack free allowed erase :)
			fprintf(stderr, "Erase failed again, nothing we can do now.  Exiting...\n");
			close(g_fp);
			free(g_data);
			free(rsecpack);
			return -1;
		}

		printf("Okay, lets try that again...\n");

		secpack = fopen(argv[1], "rb");	// reread

		if (secpack == NULL) {
			fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
			close(g_fp);
			free(g_data);
			free(rsecpack);
			return -1;
		}
		
		if (fread(rsecpack, 1, 0x800, secpack) < 0x800) {
			fprintf(stderr, "fread() gave short item count!  Exiting...\n");
			fclose(secpack);
			close(g_fp);
			free(g_data);
			free(rsecpack);
			return -1;
		}

		fclose(secpack);

		if (sendSecpack(rsecpack) == -1) {
			fprintf(stderr, "sendSecpack() failure!  Exiting...\n");
			close(g_fp);
			free(g_data);
			free(rsecpack);
			return -1;
		}

		if (performErase(0xA0020000, 0xA03BFFFE, 1) == -1) {
			printf("Hmm... what did you do?\n");
			fprintf(stderr, "Erase failed again, nothing we can do now.  Exiting...\n");
			close(g_fp);
			free(g_data);
			free(rsecpack);
			return -1;
		}

	}

	// First exploit, the -0x20000 exploit
	// This writes the firmware, in all its unsigned glory
	// I guess Apple figured -0x400 was simple, -0x20000 is *much* harder

	if (performAddress(0xA0000000, 0) == -1) {  // -0x20000, like i said :)
		fprintf(stderr, "performAddress() failure!  Exiting...\n");
		close(g_fp);
		free(g_data);
		free(rsecpack);
		return -1;
	}

	FILE *bb = fopen(argv[2], "rb");

	if (bb == NULL) {
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		close(g_fp);
		free(g_data);
		free(rsecpack);
		return -1;
	}

	if (fseek(bb, 0x9a4, SEEK_SET) == -1) {  // skip bbupdater data and secpack
		fprintf(stderr, "Error %i(%s)\n", errno, strerror(errno));
		close(g_fp);
		free(g_data);
		free(rsecpack);
		return -1;
	}

	int a, rc = 0;

	do {
		a = fread(g_data, 1, 0x800, bb);

		if ( (rc < PATCHLOC) && (PATCHLOC < (rc+a)) ) {  // patch the firmware
			printf("Patching...\n");
			g_data[PATCHLOC-rc+3] = 0xe3;
			g_data[PATCHLOC-rc+2] = 0xa0;
			g_data[PATCHLOC-rc+1] = 0x00;
			g_data[PATCHLOC-rc]   = 0x01; 
		}

		if ( ((rc % 0x10000) == 0) || (a != 0x800) ) {
			printf("Wrote: 0x%x 0x%x\n", a, rc);
		}

		if (a > 0) {

			if (bbWrite(a, 1) == -1) {  // write like hell
				fprintf(stderr, "bbWrite() failure!  Exiting...\n");
				close(g_fp);
				free(g_data);
				free(rsecpack);
				return -1;
			}

		}

		rc += a;
	} while (a > 0);

	// Second exploit, the fake secpack erase range
	// If a valid secpack is present in 0x3C0000, the phone won't boot
	// And since endpack doesn't work, I needed to find another way
	const char fakesec[] = {
		0x00, 0x00, 0x3C, 0xA0, 0x00, 0x00, 0x03, 0x00,
		0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};  // not 0xA03D0000

	memcpy(&rsecpack[0x780], fakesec, 0x10);

	if (sendSecpack(rsecpack) == -1) {
		fprintf(stderr, "sendSecpack() failure!  Exiting...\n");
		close(g_fp);
		free(g_data);
		free(rsecpack);
		return -1;
	}

	if (endSecpack() == -1) {
		fprintf(stderr, "endSecpack() failure!  Exiting...\n");
		close(g_fp);
		free(g_data);
		free(rsecpack);
		return -1;
	}

	// 0p - Another case where we close our eyes and pretend it works :)
	/*
	if (performErase(0xA03D0000, 0xA03F0000, 1) == -1) {  // the only secpack free allowed erase :)
		fprintf(stderr, "performErase failure!  Exiting...\n");
		close(g_fp);
		free(g_data);
		free(rsecpack);
		return -1;
	}
	*/
	performErase(0xA03D0000, 0xA03F0000, 1);

	close(g_fp);
	free(g_data);
	free(rsecpack);

	if (resetBaseband() == -1) {
		fprintf(stderr, "resetBaseband() failure during cleanup.  You still should be OK...\n");
		return -1;
	}

	printf("Enjoy your unlocked iPhone...\n");
	return 0;
}
