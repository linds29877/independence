#!/bin/sh

/usr/bin/clear
/bin/echo "***********************************" 
/bin/echo "* iNdependence RAMdisk Generator  *"
/bin/echo "* Created by The Operator         *"
/bin/echo "***********************************" 

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then
	/bin/echo "usage: $0 <path to extra files> <path to zero file> <path to ramdisk>"
	exit 1
fi

ITUNESDIR="$HOME/Library/iTunes/iPhone Software Updates"
FW113FILE="iPhone1,1_1.1.3_4A93_Restore.ipsw"
ITUNES113FW="$ITUNESDIR/$FW113FILE"
DL113FW="http://appldnld.apple.com.edgesuite.net/content.info.apple.com/iPhone/061-4061.20080115.4Fvn7/$FW113FILE"
WORKDIR=

# 0p - For debugging...
#/bin/echo "HOME = $HOME"
#/bin/echo "ITUNESDIR = $ITUNESDIR"
#/bin/echo "FW113FILE = $FW113FILE"
#/bin/echo "ITUNES113FW = $ITUNES113FW"
#/bin/echo "DL113FW = $DL113FW"

if [ ! -e "$ITUNES113FW" ]; then

	if [ -e "$ITUNESDIR" ]; then
		WORKDIR=$ITUNESDIR
	else
		WORKDIR="/tmp"
	fi

	pushd "$WORKDIR"

	if [ ! -e "$FW113FILE" ]; then
		/bin/echo ""
		/bin/echo "Downloading 1.1.3 iPhone firmware to" "$WORKDIR/$FW113FILE"
		/usr/bin/curl -C - -O "$DL113FW" --progress-bar
	fi

	popd
else
	WORKDIR=$ITUNESDIR
fi

/bin/echo ""
/bin/echo "Extracting firmware..."
/usr/bin/ditto -kx "$WORKDIR/$FW113FILE" "$WORKDIR/firmware_113/"

/bin/echo ""
/bin/echo "Removing the first 0x800 bytes..."
/bin/dd if="$WORKDIR/firmware_113/022-3745-250.dmg" of="$WORKDIR/ramdisk.dmg" bs=512 skip=4 conv=sync

/bin/echo ""
/bin/echo "Decrypting ramdisk..."
/usr/bin/openssl enc -d -in "$WORKDIR/ramdisk.dmg" -out "$WORKDIR/ramdisk_decrypted.dmg" -aes-128-cbc -K 188458A6D15034DFE386F23B61D43774 -iv 0
/bin/rm "$WORKDIR/ramdisk.dmg"
/bin/dd if="$WORKDIR/ramdisk_decrypted.dmg" of="$WORKDIR/ramdisk_decrypted_clean.dmg" bs=512 count=36632 conv=sync
/bin/rm "$WORKDIR/ramdisk_decrypted.dmg"
/bin/rm -rf "$WORKDIR/firmware_113"

/bin/echo ""
/bin/echo "Mounting ramdisk..."
/usr/bin/hdiutil mount "$WORKDIR/ramdisk_decrypted_clean.dmg"

/bin/echo ""
/bin/echo "Modifying ramdisk..."
/bin/rm -rf "/Volumes/ramdisk/System/Library/Frameworks/CoreGraphics.framework"
/usr/bin/ditto -kx "$1" "/Volumes/ramdisk/"

# 0p - Sometimes you can't unmount the ramdisk if you don't do this
/bin/sync
/bin/sleep 1

/bin/echo ""
/bin/echo "Unmounting ramdisk..."
/usr/bin/hdiutil unmount "/Volumes/ramdisk"

/bin/echo ""
/bin/echo "Creating new ramdisk..."
/usr/bin/ditto -kx "$2" "$WORKDIR/zero_file"
/bin/cat "$WORKDIR/zero_file" "$WORKDIR/ramdisk_decrypted_clean.dmg" > "$3"

/bin/rm "$WORKDIR/zero_file"
/bin/rm "$WORKDIR/ramdisk_decrypted_clean.dmg"
