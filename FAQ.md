Note from 0p: This information is mostly circa 1.1.1/1.1.2 firmware.

### What Versions of Mac OS X/iTunes/iPhone Firmware Is iNdependence Compatible With? ###

It's been tested and is known to work on Intel and PPC Macintosh systems running Mac OS 10.4.10, 10.4.11, and 10.5.1, 10.5.2 and with iTunes versions 7.3 - 7.6.1

It's also been tested and is known to work with iPhone firmware 1.0, 1.0.1, 1.0.2, 1.1.1, 1.1.2, 1.1.3, and 1.1.4.

### Do I need any other applications besides iNdependence to jailbreak/activate my phone? ###

No, iNdependence does everything itself (doesn't rely on any external applications or special activities on your phone).  The only exception to this is for SIM unlocking where the gunlock application is used.  As well, it uses OpenSSH to communicate with the phone for some operations.

### I previously used anySIM 1.0/iUnlock to SIM unlock my 1.0.x phone.  Now I want to upgrade to 1.1.x and SIM unlock again.  What should I do? ###

Before you can upgrade to 1.1.x, you need to revirginize your baseband.  anySIM 1.0 and iUnlock had a problem whereby they would cause certain important parts of your phone's permanent memory to be overwritten.  This causes major problems when you upgrade to 1.1.x and a new baseband is installed.

So you need to "virginize" those areas of memory again.  Instructions and programs for doing this can be found on the iPhone Elite website.

http://code.google.com/p/iphone-elite/wiki/RevirginizingTool

### The Pre Firmware Upgrade is failing?  What gives? ###

First thing, check to ensure that you can SSH into your phone by opening /Applications/Utilities/Terminal and typing:

ssh root@<your phone's IP address>

and then entering your password if you are prompted to do so.

If that doesn't work, then iNdependence will not be able to do the Pre Firmware Upgrade.  The biggest cause of this problem is a bad Wi-Fi connection between your computer and your phone.

If you can SSH into your phone from your computer, then try typing a few commands on your phone (cd .., cd /, etc) and see what happens.  If you are disconnected at any point, or it locks up, then either your phone is going to sleep at some point, you have a bad Wi-Fi connection between your computer and your phone, or the shell on your phone isn't working properly (the default iPhone shell is notoriously flakey).

To make sure your phone doesn't go to sleep at any point, change Settings->General->Auto-Lock to never.

If you have a flakey Wi-Fi connection, then I can't really help you with that.  Make sure your phone is fairly close to your computer.

In order to replace the shell on your phone, you'll need to use an SFTP program to upload a new shell program to /bin/sh.  Instructions for doing this are beyond the scope of this FAQ.

### I upgraded to 1.1.2 and now my phone/SMS program is crashing when I use it.  Why? ###

You are likely using your phone in a country for which it's unable to find the phone number formatting data.  Have a read through this page for details:

http://code.google.com/p/iphone-elite/wiki/fix_crashing_phone_sms_apps

The easiest option is to use the iWorld application to fix the problem, but I can't seem to find a standalone version of it anywhere.  The only other option is to patch the necessary files on your phone yourself.