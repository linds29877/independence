 *
 * iNdependence v1.2.1
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

Intel Mac, Mac OS X 10.4.10 - Firmware 1.0, 1.0.1, 1.0.2 - iTunes 7.3 to 7.4.1
PPC Mac, Mac OS X 10.4.10 - Firmware 1.0, 1.0.1, 1.0.2 - iTunes 7.3 to 7.4.1

-----

** How to use iNdependence **

The interface is pretty straightforward.  The tabs are arranged as follows:

Activation - For tasks related to activating your phone.
Jailbreak  - For tasks related to jailbreaking your phone (ie. making it so that you can read/write files from/to the entire filesystem on your phone).
SSH        - For tasks related to installing and maintaining SSH on your phone
Customize  - For tasks related to customization of your phone.

* Notes *

Before using iNdependence, it's best to kill off the "iTunes Helper" application so that it doesn't interfere with iNdependence.

To do this, run /Applications/Utilities/Activity Monitor and look for the process named "iTunes Helper".  Select it and press the "Quit Process" button.  Then choose "Force Quit".

* To activate your phone *

1) Download any iPhone firmware zip file from Apple:

(For 1.0) http://appldnld.apple.com.edgesuite.net/content.info.apple.com/iPhone/061-3538.20070629.B7vXa/iPhone1,1_1.0_1A543a_Restore.ipsw
(For 1.0.1) http://appldnld.apple.com.edgesuite.net/content.info.apple.com/iPhone/061-3614.20070731.Nt6Y7/iPhone1,1_1.0.1_1C25_Restore.ipsw
(For 1.0.2) http://content.info.apple.com/iPhone/061-3823.20070821.vormd/iPhone1,1_1.0.2_1C28_Restore.ipsw

It doesn't matter which one, they all work equally well. :)

2) If your web browser didn't automatically decompress it to a folder, then change the file extension from ".ipsw" to ".zip" and unzip it yourself.
3) Launch iNdependence.
4) Select the Activation tab and press the "Activate Phone" button.
5) You'll be prompted to choose the directory where you unzipped the firmware files.

Sit back and wait until it's done.  Yes, that's it.  The activation process will also enable YouTube for you.

Note that if your phone was previously jailbroken, activating it this way will return it to jail again.

* To jailbreak your phone *

1-3) Same as steps 1-3 above (if you already have the firmware files downloaded and unzipped, you can skip that part)
4) Select the Jailbreak tab and press the "Perform Jailbreak" button.
5) Same as step 5 above.

* SSH and beyond *

Installing SSH is a breeze once your phone is jailbroken.  The default accounts on the phone are:

username: root
password: dottie

username: mobile
password: alpine

Remember to change the default passwords after installing SSH.  I'm not responsible if someone hacks your phone.

* Customization *

The customization interface has been completely reworked for version 1.1.  It now uses a Finder-style interface and allows for drag and drop installation of ringtones, wallpapers, and applications.  It also has preview images for wallpapers and application icons.

Please note that in order to install/remove applications, you'll need to have SSH installed on the phone as you'll be asked for the phone's IP address and root password.

To find the phone's IP address, launch the Settings application, navigate to "Wi-Fi" and press the blue arrow beside the Wi-Fi network your phone is connected to.

Also note that ringtones must be MPEG-4 audio (.m4a) files as the file chooser will only allow you to choose .m4a files. Wallpapers must be 2 PNG (.png) files (the main image and the thumbnail image).  The wallpaper image sizes I use are 320x480 for the main image and 75x75 for the thumbnail image.  These seem to work well for me.

One last thing: to have wallpaper files work correctly on the iPhone, you should make sure that the name of the thumbnail PNG file is the same as the main PNG file, but with a .thumbnail added in the name.  For example:

Main wallpaper file: sliquepic.png
Thumbnail file     : sliquepic.thumbnail.png

Otherwise you may not be able to use them (or see them in the file list).

Have fun!
	The Operator

-----

Credit to:

  - iZsh for the code based on iASign and for the new (fast) jailbreak method
  - geohot, ixtli, nightwatch, warren, ziel for the code based on iPhoneInterface
  - nightwatch again for jailbreak exploit
  - ixtli again for helping fix jailbreak on PPC
  - Matt Johnston for DropbearSSH
  - nightwatch, netkas, and nervegas for the binkit binaries
  - MHW for the code based on iphone-fixpng

Cheers to the iPhoneDev team!
