CC=arm-apple-darwin-gcc
AS=arm-apple-darwin-as
STRIP=arm-apple-darwin-strip

BOOSTOBJS=boost.o syscalls.o
JBOBJS=jailbreak.o utilities.o

all:	launchd jailbreak

boost.o:	boost.c
		$(CC) $(CFLAGS) -static -c $< -o $@

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o:	%.S
	$(AS) $< -o $@


launchd:	$(BOOSTOBJS)
		$(CC) $(CFLAGS) -nostdlib -o $@ $(BOOSTOBJS)
		$(STRIP) $@

jailbreak:	$(JBOBJS)
		$(CC) $(CFLAGS) -framework CoreFoundation -o $@ $(JBOBJS)
		$(STRIP) $@

ramdisk:	launchd
		-mkdir ./ramdisk_mnt
		dd if=/dev/zero of=ramdisk.dmg bs=1024 count=512
		FREELOOP=`losetup -f`; \
		losetup $$FREELOOP ramdisk.dmg; \
		mkfs.hfsplus -s -v "Jailbreak Bootstrap" $$FREELOOP; \
		mount -t hfsplus $$FREELOOP ./ramdisk_mnt; \
		mkdir ./ramdisk_mnt/mnt; \
		mkdir ./ramdisk_mnt/sbin; \
		cp launchd ./ramdisk_mnt/sbin/launchd; \
		chmod 755 ./ramdisk_mnt/sbin/launchd; \
		chown 0:0 ./ramdisk_mnt/sbin/launchd; \
		umount ./ramdisk_mnt; \
		losetup -d $$FREELOOP
		rmdir ./ramdisk_mnt

clean:
		rm -f *.o
		rm -f launchd
		rm -f ramdisk.dmg
		rm -f jailbreak

dist-clean:	clean
		rm -f *~

