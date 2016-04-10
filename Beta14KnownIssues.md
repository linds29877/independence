## Details ##

The issue tracker has become too clogged with bugs and I don't have time to deal with it.  So could everyone report issues with iNdependence 1.4 beta on this page instead.

## Current Known Issues ##

  * Google maps current location feature will not work if you SIM unlock using iNdependence 1.4 beta.  This is because when iNdependence SIM unlocks your phone, it installs baseband 04.02.13\_G, and this (older) baseband doesn't support that feature.  The reason I do things this way is because it works for _all bootloader versions_.

> In an upcoming version of iNdependence I hope to have a version of gunlock which autodetects the bootloader version and chooses the best SIM unlock method possible.

  * Some people are reporting problems where iNdependence can't download the firmware file.

> WORKAROUND: Manually download the 1.1.2 firmware file from

> http://appldnld.apple.com.edgesuite.net/content.info.apple.com/iPhone/061-4037.20071107.5Bghn/iPhone1,1_1.1.2_3B48b_Restore.ipsw

> and copy it to HOME/Library/iTunes/iPhone Software Updates/ (where HOME is your user folder in Finder)

> Obviously, Apple's servers may have some downtime/technical issues now and then, so if it doesn't work for you, just wait a bit and try again later.

  * If Wi-Fi/Bluetooth/Phone aren't working after performing a SIM unlock, then it's likely that gunlock failed at some point and left your baseband in a bad state (I've had this happen).  iNdependence will detect this failure and give an error message, but your phone may still be in that bad state.

> WORKAROUND: Try SIM unlocking again as gunlock usually succeeds on the 2nd attempt.

  * If you have previously used iPhoneSimFree (IPSF) to SIM unlock your phone, then iNdependence will still work for you.  However, you may need to run Signal.app each time your phone boots in order to get service.  There are mixed reports on this: some people need Signal.app, some don't.  I'm unsure why at this point as I've never used IPSF myself.

  * iNdependence can freeze up when it's attempting to jailbreak, activate, and SIM unlock.  If the screen on your phone hasn't switched to the Apple logo within 30 seconds, then it's likely frozen.  You'll need to force quit and restart iNdependence (Alt+Option+Esc, select iNdependence, press "Force Quit").  This is a long-standing bug, and it's due to a faulty API in the iTunes library it uses (MobileDevice).

> WORKAROUND: Force quit and restart iNdependence, then try again.

  * If you were previously SIM unlocked, you will need to do a SIM unlock again after upgrading to 1.1.3 or 1.1.4.  TurboSIM doesn't work under 1.1.3/1.1.4, and even the IPSF SIM unlock doesn't work without Signal.app.

  * If a phone has been previously jailbroken with ZiPhone, iNdependence will say that it's still jailed.  The reason for this is because ZiPhone does not create the AFC2 service on the phone.  The AFC2 service allows computer-based applications to write to all areas of the phone (not just the Media partition).  So really, ZiPhone only jailbreaks for applications which run on the phone itself (not applications which run from the computer).

> WORKAROUND: Jailbreak again with iNdependence

  * iNdependence currently does not put Installer.app on your phone.  You'll need to find a standalone copy and then use the Customize tab in iNdependence to install it.

> WORKAROUND: See below for instructions on how to install Installer.app

## Getting Installer.app onto Your Phone ##

(From john.angus - slightly modified by me)

How I got installer.app working:

  1. Get the latest version from here: http://repository.apptapp.com/packages/System/Installer.zip
  1. Install the Installer.app file using iNdependence
  1. SSH into your phone and issue the following commands:
    * `chmod 777 /Applications`
    * `chmod 777 /Applications/Installer.app`
    * `chmod 777 /Applications/Installer.app/Installer`

## Problems With 3rd Party Applications on 1.1.3 ##

john.angus reported that many 3rd party applications don't work all that well on 1.1.3 because they are run as user "mobile" instead of user "root".  Meaning, they won't have permission to write files to certain locations on the phone, and do other things which "root" can do.  This is not a problem with iNdependence, it's a bug in those 3rd party applications (assuming they are being run as root).  You'll have to wait until the developers update their applications.

## Downloading the firmware file is taking forever! ##

Yes, I realize that this can be slow.  I'd really love to just bundle it all up with iNdependence so that you don't have to download it.  Unfortunately, I don't want to risk having Apple's legal team shut this project down because it's distributing things covered under their licensing agreement.  So that's why you have to download the firmware file.

Be patient... you'll only need to download it once. :)