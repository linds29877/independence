#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/syslog.h>
#include <sys/reboot.h>
#include <hfs/hfs_mount.h>

#include "utilities.h"

#define out(args...) syslog(LOG_EMERG,args)
#define assume0(name) if (rc!=0) out(name "=%d %m",rc)

void mounthfs(char* device, const char* mntPoint, int options) {
	struct hfs_mount_args hma;
	int rc;
	
	bzero(&hma,sizeof(hma));
	hma.fspec = device;
	rc = mount("hfs", mntPoint, options, &hma);
	assume0("mount");
}

int main(int argc, char *argv[], char *env[]) {
	char* version;

	cmd_system((char*[]){"/sbin/fsck_hfs", "-fy", "/dev/rdisk0s1", (char*) 0});
	mounthfs("/dev/disk0s1", "/", MNT_ROOTFS | MNT_UPDATE);

	cmd_system((char*[]){"/sbin/fsck_hfs", "-fy", "/dev/rdisk0s2", (char*) 0});
	mounthfs("/dev/disk0s2", "/private/var", MNT_UPDATE);

	// environment is basically set up at this point

	// jailbreak
	fileCopySimple("/private/var/mobile/Media/jailbreak/fstab", "/private/etc/fstab");
	fileCopySimple("/private/var/mobile/Media/jailbreak/Services.plist", "/System/Library/Lockdown/Services.plist");

	if(isIphone()) {
		// iPhone specific stuff
		if(fileExists("/private/var/mobile/Media/jailbreak/activate")) {
			if(strcmp(activationState(), "Unactivated") == 0) {
				version = firmwareVersion();

				chmod("/private/var/mobile/Media/jailbreak/optivate", 0755);
				if(strcmp(activationState(), "1.0") == 0) {
					cmd_system((char*[]){"/private/var/mobile/Media/jailbreak/optivate", "100", "/usr/libexec/lockdownd", (char*) 0});
				} else if(strcmp(activationState(), "1.0.1") == 0) {
					cmd_system((char*[]){"/private/var/mobile/Media/jailbreak/optivate", "101", "/usr/libexec/lockdownd", (char*) 0});
				} else if(strcmp(activationState(), "1.0.2") == 0) {
					cmd_system((char*[]){"/private/var/mobile/Media/jailbreak/optivate", "102", "/usr/libexec/lockdownd", (char*) 0});
				} else if(strcmp(activationState(), "1.1.1") == 0) {
					cmd_system((char*[]){"/private/var/mobile/Media/jailbreak/optivate", "111", "/usr/libexec/lockdownd", (char*) 0});
				} else if(strcmp(activationState(), "1.1.2") == 0) {
					cmd_system((char*[]){"/private/var/mobile/Media/jailbreak/optivate", "112", "/usr/libexec/lockdownd", (char*) 0});
				} else if(strcmp(activationState(), "1.1.3") == 0) {
					cmd_system((char*[]){"/private/var/mobile/Media/jailbreak/optivate", "113", "/usr/libexec/lockdownd", (char*) 0});
				} else if(strcmp(activationState(), "1.1.4") == 0) {
					cmd_system((char*[]){"/private/var/mobile/Media/jailbreak/optivate", "114", "/usr/libexec/lockdownd", (char*) 0});
				}

				mkdir("/private/var/root/Library", 0755);
				mkdir("/private/var/root/Library/Lockdown", 0755);

				fileCopySimple("/private/var/mobile/Media/jailbreak/data_ark.plist", "/private/var/root/Library/Lockdown/data_ark.plist");
				fileCopySimple("/private/var/mobile/Media/jailbreak/device_private_key.pem", "/private/var/root/Library/Lockdown/device_private_key.pem");
				fileCopySimple("/private/var/mobile/Media/jailbreak/device_public_key.pem", "/private/var/root/Library/Lockdown/device_public_key.pem");
			}
		}

		if(fileExists("/private/var/mobile/Media/jailbreak/simunlock")) {
			cmd_system((char*[]){"/private/var/mobile/Media/jailbreak/gunlock_0p", "/private/var/mobile/Media/jailbreak/secpack", "/private/var/mobile/Media/jailbreak/ICE04.02.13_G.fls", (char*) 0});
		}
	}

	// clean-up
	deleteDir("/private/var/mobile/Media/jailbreak");

	sync();

	mounthfs("/dev/disk0s1", "/", MNT_RDONLY | MNT_UPDATE);
	mounthfs("/dev/disk0s2", "/private/var", MNT_RDONLY | MNT_UPDATE);

	sync();

	reboot(RB_AUTOBOOT);
}

