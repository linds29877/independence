#include <CoreFoundation/CoreFoundation.h>
#include "PhoneInteraction/PhoneInteraction.h"

#include <cstdlib>
#include <iostream>
#include <string.h>


using namespace std;

static PhoneInteraction *g_phoneInteraction;


void updateStatus(const char *msg, bool waiting)
{
    cout << msg << endl;
}

void kill300MBProblem()
{
    char *version = g_phoneInteraction->getPhoneProductVersion();
	
    if (strcmp(version, "1.1.1")) {
		cout << "Error: Cannot fix the 300 MB problem because your phone is not using firmware 1.1.1." << endl;
		return;
    }
	
    if (!g_phoneInteraction->isPhoneJailbroken()) {
		cout << "Error: Phone is not jailbroken so I can't fix the 300MB problem for you." << endl;
		return;
    }

	if (g_phoneInteraction->fileExists("/private/var/root/Media.backup")) {

		// ok, we know that iNdependence has been here... don't warn them and clean things up
		if (!g_phoneInteraction->renamePath("/private/var/root/Media", "/private/var/root/.symlink")) {
			cout << "Error: Failed to rename /private/var/root/Media to /private/var/root/.symlink" << endl;
			return;
		}

		if (!g_phoneInteraction->renamePath("/private/var/root/Media.backup", "/private/var/root/Media")) {
			cout << "Success!  However, please run iTunes so that it can recreate the Media directory for you before you do anything else with your phone." << endl;
			return;
		}

	}
	else if (g_phoneInteraction->fileExists("/private/var/root/Media.iNdependence")) {

		// ok, we know that iNdependence has been here... don't warn them and clean things up
		if (!g_phoneInteraction->renamePath("/private/var/root/Media", "/private/var/root/.symlink")) {
			cout << "Error: Failed to rename /private/var/root/Media to /private/var/root/.symlink" << endl;
			return;
		}

		if (!g_phoneInteraction->renamePath("/private/var/root/Media.iNdependence", "/private/var/root/Media")) {
			cout << "Success!  However, please run iTunes so that it can recreate the Media directory for you before you do anything else with your phone." << endl;
			return;
		}

	}
	else if (g_phoneInteraction->fileExists("/private/var/root/Media")) {
		bool proceed = false;

		while (1) {

			// warn them
			cout << "WARNING: It doesn't look like iNdependence has been here.  Are you sure you want to proceed and wipe out your Media directory (yes/no)?" << endl;

			string response;
			cin >> response;

			if (response.empty()) {
				cout << "Invalid response.  Try again." << endl;
			}
			else if ((response[0] == 'y') || (response[0] == 'Y')) {
				proceed = true;
				break;
			}
			else if ((response[0] == 'n') || (response[0] == 'N')) {
				proceed = false;
				break;
			}
			else {
				cout << "Invalid response.  Try again." << endl;
			}

		}

		if (proceed) {

			// I thought about just trying to remove /private/var/root/Media here.  However, if it's symlinked to / then
			// that will wipe out files on the main filesystem, which is not good (I did this to myself and ended up with an
			// empty SpringBoard).  So just rename the path.  They can deal with the wasted disk space if it's a valid
			// directory and not a symlink.

			if (!g_phoneInteraction->renamePath("/private/var/root/Media", "/private/var/root/.symlink")) {
				cout << "Error: Failed to rename /private/var/root/Media to /private/var/root/.symlink" << endl;
				return;
			}

			if (!g_phoneInteraction->createDirectory("/private/var/root/Media")) {
				cout << "Success!  However, please run iTunes so that it can recreate the Media directory for you before you do anything else with your phone." << endl;
				return;
			}

		}
		else {
			return;
		}

	}
	else {
		cout << "Error: Couldn't find /private/var/root/Media.  You should run iTunes to fix this problem." << endl;
		return;
	}

    cout << "Success!  You should no longer have the 300 MB problem when you use iTunes." << endl;
}

void phoneNotification(int type, const char *msg)
{
	
    switch (type) {
		case NOTIFY_CONNECTED:
			cout << "connected!" << endl;
			kill300MBProblem();
			CFRunLoopStop(CFRunLoopGetCurrent());
			break;
		case NOTIFY_DISCONNECTED:
			cout << "disconnected!  Waiting for phone..." << endl;
			break;
		case NOTIFY_INITIALIZATION_FAILED:
			cout << "Initialization failed!  " << msg << endl;
			CFRunLoopStop(CFRunLoopGetCurrent());
			break;
		case NOTIFY_CONNECTION_FAILED:
			cout << "Connection to phone failed!  " << msg << endl;
			CFRunLoopStop(CFRunLoopGetCurrent());
			break;
		case NOTIFY_AFC_CONNECTION_FAILED:
			cout << "AFC connection to phone failed!  " << msg << endl;
			CFRunLoopStop(CFRunLoopGetCurrent());
			break;
		default:
			break;
    }
	
}

int main(int argc, char **argv)
{
    g_phoneInteraction = PhoneInteraction::getInstance(updateStatus, phoneNotification);
    CFRunLoopRun();
    return 0;
}
