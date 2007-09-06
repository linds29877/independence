 *
 * iNdependence v1.1
 * Copyright 2007 The Operator
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 2 for more details
 *

Disclaimer: I am not responsible for any damage caused to your phone by this tool.  Remember that you can always use iTunes to restore your phone to it's original state.

Platforms and configurations known to work with iNdependence:

Intel Mac, Mac OS X 10.4.10 - Firmware 1.0, 1.0.1, 1.0.2
PPC Mac, Mac OS X 10.4.10 - Firmware 1.0, 1.0.1, 1.0.2

-----

** How to use iNdependence **

The interface is pretty straightforward.  The tabs are arranged as follows:

Jailbreak  - For tasks related to jailbreaking your phone (ie. making it so that you can read/write files from/to the entire filesystem on your phone).
Activation - For tasks related to activating your phone.
SSH        - For tasks related to installing and maintaining SSH on your phone
Customize  - For tasks related to customization of your phone.

The most complicated task is Activation, so I'll include instructions for it here.  If you've used iActivator before, it's exactly the same.

* Note about jailbreak *

Please note that if iTunes launches and communicates with your iPhone at any point during the jailbreak process, it will mess up iNdependence and cause strange errors.  This usually happens after you perform a jailbreak and the phone reboots.

To avoid this, kill off the "iTunes Helper" program using /Applications/Utilities/Activity Monitor (or the command-line if you prefer).  Otherwise, you will need to quit and restart iNdependence before you proceed with the next step.

* To activate your phone *

1) Download the iPhone firmware zip file from Apple which corresponds to the version of the firmware you have on your phone:

(For 1.0) http://appldnld.apple.com.edgesuite.net/content.info.apple.com/iPhone/061-3538.20070629.B7vXa/iPhone1,1_1.0_1A543a_Restore.ipsw
(For 1.0.1) http://appldnld.apple.com.edgesuite.net/content.info.apple.com/iPhone/061-3614.20070731.Nt6Y7/iPhone1,1_1.0.1_1C25_Restore.ipsw
(For 1.0.2) http://content.info.apple.com/iPhone/061-3823.20070821.vormd/iPhone1,1_1.0.2_1C28_Restore.ipsw

2) If your web browser didn't automatically decompress it to a folder, then change the file extension from ".ipsw" to ".zip" and unzip it yourself.
3) Launch iNdependence.
4) Select the Jailbreak tab and press the "Perform Jailbreak" button.  Follow the instructions.  You'll be prompted to select the directory where you unzipped the firmware files.
5) Select the Activation tab and press the "Put PEM File" button.
6) Again in the Activation tab, press the "Generate Activation" button to create an activation file.  Choose a location to save this file on your computer.
7) Now press the "Activate Phone" button and select the activation file you saved in the previous step.

If any of the above steps fail, quit iNdependence, restart it, and continue at the same step again.  Also, it has been noted that the "Activate Phone" step tends to fail quite often.  If this happens, you can either quit iNdependence, restart it, and try again, or you can try returning your phone to jail (in the Jailbreak tab) and then attempt to activate it again (some people have reported better success doing it that way).

Beyond that, installing SSH is a breeze.  The default accounts on the phone are:

username: root
password: dottie

username: mobile
password: alpine

Remember to change the default passwords after installing SSH.  I'm not responsible if someone hacks your phone.

* Customization *

The customization interface has been completely reworked for version 1.1.  It now uses a Finder-style interface and allows for drag and drop installation of ringtones, wallpapers, and applications.  It also has preview images for wallpapers and application icons.

Please note that in order to install/remove applications, you'll need to have SSH installed on the phone as you'll be asked for the phone's IP address and root password.

Also note that ringtones must be MPEG-4 audio (.m4a) files as the file chooser will only allow you to choose .m4a files. Wallpapers must be 2 PNG (.png) files (the main image and the thumbnail image).  The wallpaper image sizes I use are 320x480 for the main image and 75x75 for the thumbnail image.  These seem to work well for me.

Have fun!
	The Operator

-----

Credit to:

  - iZsh for the code based on iASign
  - geohot, ixtli, nightwatch, warren, ziel for the code based on iPhoneInterface
  - nightwatch again for jailbreak exploit
  - ixtli again for helping fix jailbreak on PPC
  - Matt Johnston for DropbearSSH
  - nightwatch, netkas, and nervegas for the binkit binaries
  - MHW for the code based on iphone-fixpng

Cheers to the iPhoneDev team!
