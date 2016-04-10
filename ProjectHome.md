![http://independence.googlecode.com/svn/trunk/iNdependence/images/iNdependenceBanner.jpg](http://independence.googlecode.com/svn/trunk/iNdependence/images/iNdependenceBanner.jpg)

## About ##

iNdependence is a Cocoa-based application for Mac OS X which provides an easy-to-use interface for jailbreak, activation, SSH installation, and ringtone/wallpaper/application installation on your iPhone.  It's open source under the GNU v2 license.

The source code for interacting with the iPhone has been broken out into a library (libPhoneInteraction) so that it can be easily reused by other applications.

## PLEASE READ THE HELP FILES THOROUGHLY BEFORE POSTING AN ISSUE ##

Documentation is now available from the Help menu within the application itself.  Please read all of it before you post an issue, email me, etc.  I can guarantee that 99% of the problems you will encounter are answered there.  The rest are likely well documented already in the Beta14KnownIssues page, the [FAQ](http://code.google.com/p/independence/wiki/FAQ), or in the [issues](http://code.google.com/p/independence/issues/list) section.  Please also check for duplicate issues there before posting a new one.  I just don't have the time to go through the multitude of issues there (many of which are duplicates or simply usage questions and not real issues).

## News ##

Apr. 8/08 - And yes, I've had my head in the sand for far too long.  Seems that project [Pwnage](http://iphone-dev.org/) has finally come to fruition.  Congrats guys!  Looks like I won't need to maintain jailbreaking/SIM unlocking in iNdependence anymore. :)

Apr. 8/08 - Yes, it's been far too long and I've been far too busy...  Regardless, iNdependence 1.4 (final) is finally out.  Not too many changes in this release, but it's compatible with iTunes 7.6.2, has Danish localization, and it should jailbreak the iPod Touch again.  One major note is that I've revamped the MobileDevice framework hookup code to do proper (binary) searches for the private functions.  This should alleviate all problems with figuring out when iTunes updates break things or not and generally make things easier going forward.  Check the [CHANGELOG](http://code.google.com/p/independence/source/browse/tags/1.4/iNdependence/Other%20Files/CHANGELOG.txt) for more details.

> NOTE: If you have installed the iPhone SDK, then you won't be able to use iNdependence to jailbreak or SIM unlock.  It will give you a message telling you how to fix this problem when it starts up.

Mar. 7/08 - So the [iPhone SDK](http://developer.apple.com/iphone/) is out.  Had a chance to play around with it a bit and it looks great (for the future).  However, it's useless for developing for current phones (1.1.4 or lower) since it's all based on iPhone software version 2.0.  Still might try a few things to see if I can't get it to link against the 1.1.4 frameworks...

Feb. 28/08 - I've started a list of websites which are distributing binary versions of iNdependence as part of their SIM unlocking solutions, but not making the source code publicly available.  If you know of any others, submit them to the list on the [GPLViolators](http://code.google.com/p/independence/wiki/GPLViolators) wiki page.

Feb. 28/08 - Some people have been asking about downgrading the bootloader so that they can use the latest versions of the baseband SIM unlocked.  I do not feel comfortable downgrading the bootloader from iNdependence because, if things go wrong, you will end up with a permanently bricked phone.  I'd rather have people missing out on the Google Maps Locate Me feature than risk having that happen to people.

Feb. 14/08 - Lots of great feedback and helpful information is rolling into the Beta14KnownIssues wiki page.


## Notes ##

  * This program is not tested with the iPod Touch (though there are reports that it has worked)


## A Call To iPhone Application Developers ##

I understand that Installer.app/AppTapp/AppSnapp makes installing/uninstalling applications and their related dependencies easier.  However, I urge developers to also make standalone versions of their apps downloadable from somewhere.  Many people simply want to install one or two applications by hand without having to install a package management system to do that (myself included in that group).  So please consider this type of user as well when you are releasing your applications.


## Donations ##

Some people have asked about donating in appreciation for this software.  Although it is not required, if you would like to donate something, you can send it via PayPal to operator@dialectro.com

Rest assured that I am not affiliated with the core iPhone dev team, and so I have not received any of the donation money sent to them.  However, my work is built upon theirs and so I will send a portion of the donations I receive to them unless you specify otherwise in your donation (ie. you've already donated to them and want this to be separate).

Your support and generosity is appreciated.

<sub>--0p</sub>

_Thanks to Christian Terrazas for the banner_