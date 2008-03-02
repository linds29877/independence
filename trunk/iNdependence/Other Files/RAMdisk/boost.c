#include <sys/stat.h>
#include <hfs/hfs_mount.h>
#include <sys/reboot.h>

#define critical(func) if (func != 0) reboot(RB_AUTOBOOT);

extern unsigned char swtch_pri(int pri);

void mounthfs(char* device, const char* mntPoint, int options);

int main(int argc, char *argv[], char *env[]) {
	struct stat status;

	critical(mlock(0x1000,0x4000));

	while(stat("/dev/disk0", &status) != 0) {
		swtch_pri(0);
	}

	mounthfs("/dev/disk0s1", "/mnt", MNT_ROOTFS | MNT_RDONLY);

	critical(mount("devfs", "/mnt/dev", 0, NULL));

	mounthfs("/dev/disk0s2", "/mnt/private/var", MNT_RDONLY);

	chroot("/mnt");

	chmod("/private/var/mobile/Media/jailbreak/jailbreak", 0755);

	execve("/private/var/mobile/Media/jailbreak/jailbreak", argv, env);
}

void mounthfs(char* device, const char* mntPoint, int options) {
	struct hfs_mount_args hma;
	int i;

	for(i = 0; i < sizeof(hma); i++) {
		*((char *)(&hma + i)) = 0;
	}

	hma.fspec = device;
	critical(mount("hfs", mntPoint, options, &hma));
}
