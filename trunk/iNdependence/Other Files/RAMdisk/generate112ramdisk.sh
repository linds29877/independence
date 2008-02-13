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
FW112FILE="iPhone1,1_1.1.2_3B48b_Restore.ipsw"
ITUNES112FW="$ITUNESDIR/$FW112FILE"
DL112FW="http://appldnld.apple.com.edgesuite.net/content.info.apple.com/iPhone/061-4037.20071107.5Bghn/$FW112FILE"
WORKDIR=

# 0p - For debugging...
#/bin/echo "HOME = $HOME"
#/bin/echo "ITUNESDIR = $ITUNESDIR"
#/bin/echo "FW112FILE = $FW112FILE"
#/bin/echo "ITUNES112FW = $ITUNES112FW"
#/bin/echo "DL112FW = $DL112FW"

if [ ! -e "$ITUNES112FW" ]; then

	if [ -e "$ITUNESDIR" ]; then
		WORKDIR=$ITUNESDIR
	else
		WORKDIR="/tmp"
	fi

	pushd "$WORKDIR"

	if [ ! -e "$FW112FILE" ]; then
		/bin/echo ""
		/bin/echo "Downloading 1.1.2 iPhone firmware to" "$WORKDIR/$FW112FILE"
		/usr/bin/curl -C - -O "$DL112FW" --progress-bar
	fi

	popd
else
	WORKDIR=$ITUNESDIR
fi

/bin/echo ""
/bin/echo "Extracting firmware..."
/usr/bin/ditto -kx "$WORKDIR/$FW112FILE" "$WORKDIR/firmware_112/"

/bin/echo ""
/bin/echo "Removing the first 0x800 bytes..."
/bin/dd if="$WORKDIR/firmware_112/022-3726-1.dmg" of="$WORKDIR/ramdisk.dmg" bs=512 skip=4 conv=sync

/bin/echo ""
/bin/echo "Decrypting ramdisk..."
/usr/bin/openssl enc -d -in "$WORKDIR/ramdisk.dmg" -out "$WORKDIR/ramdisk_decrypted.dmg" -aes-128-cbc -K 188458A6D15034DFE386F23B61D43774 -iv 0
/bin/rm "$WORKDIR/ramdisk.dmg"
/bin/dd if="$WORKDIR/ramdisk_decrypted.dmg" of="$WORKDIR/ramdisk_decrypted_clean.dmg" bs=512 count=36632 conv=sync
/bin/rm "$WORKDIR/ramdisk_decrypted.dmg"
/bin/rm -rf "$WORKDIR/firmware_112"

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
/bin/cat "$WORKDIR/zero_file/zero_file" "$WORKDIR/ramdisk_decrypted_clean.dmg" > "$3"

/bin/rm -rf "$WORKDIR/zero_file"
/bin/rm "$WORKDIR/ramdisk_decrypted_clean.dmg"

exit 0
