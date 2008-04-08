#include <stdio.h>
#include <fcntl.h>

#include <sys/syslog.h>
#include <sys/stat.h>
#include <hfs/hfs_mount.h>
#include <sys/reboot.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <mach/mach.h>
#include <mach/mach_time.h>

#define critical(func) if (func != 0) { message("ERROR!\n"); while(1) { my_sleep(1); } }

void my_sleep(int seconds);

void mounthfs(char* device, const char* mntPoint, int options);

void cmd_system(char * argv[], char *env[]);

void message(char *msg);

int main(int argc, char *argv[], char *env[]) {
	struct stat status;
	int console;

	mlock(0x1000,0x2000);

	console = open("/dev/console", O_WRONLY);
	dup2(console, 1);
	dup2(console, 2);

	message("iNdependence: Waiting for flash\n");

	while(stat("/dev/disk0", &status) != 0) {
		my_sleep(1);
	}

	message("iNdependence: Mounting root\n");

	mounthfs("/dev/disk0s1", "/mnt", MNT_ROOTFS | MNT_RDONLY);

	message("iNdependence: Mounting devfs\n");

	critical(mount("devfs", "/mnt/dev", 0, NULL));

	message("iNdependence: Switching to root\n");

	critical(chroot("/mnt"));

	message("iNdependence: Checking data volume integrity\n");

	cmd_system((char*[]){"/sbin/fsck_hfs", "-fy", "/dev/rdisk0s2", (char*) 0}, env);

	message("iNdependence: Mounting data volume\n");

	mounthfs("/dev/disk0s2", "/private/var", 0);

	message("iNdependence: Preparing to execute jailbreak payload\n");

	critical(chmod("/private/var/mobile/Media/jailbreak/jailbreak", 0755));

	message("iNdependence: Executing jailbreak payload\n");

	critical(execve("/private/var/mobile/Media/jailbreak/jailbreak", argv, env));
}

void message(char *msg) {
	while((*msg) != '\0') {
		write(1, msg, 1);
		msg++;
	}
}

void my_sleep(int seconds) {
	static mach_timebase_info_data_t sTimebaseInfo = {0, 0};

	if(sTimebaseInfo.denom == 0) {
		mach_timebase_info(&sTimebaseInfo);
	}

	mach_wait_until(mach_absolute_time() + (((unsigned int)seconds) * 6000000)); // 6000000 only accurate for current iPhone CPU
}

void cmd_system(char * argv[], char *env[])
{
	if(vfork() != 0)
	{
		while(wait4(-1, NULL, WNOHANG, NULL) <= 0) {
			my_sleep(1);
		}
	} else {
		execve(argv[0], argv, env);
	}
}

void my_bzero(char* buffer, int size) {
	int i;

	for(i = 0; i < size; i++) {
		buffer[i] = 0;
	}
}

void mounthfs(char* device, const char* mntPoint, int options) {
	struct hfs_mount_args hma;

	my_bzero((char*)&hma, sizeof(hma));
	hma.fspec = device;
	critical(mount("hfs", mntPoint, options, &hma));
}
