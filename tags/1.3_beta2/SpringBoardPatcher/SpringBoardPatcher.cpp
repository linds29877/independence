#include <CoreFoundation/CoreFoundation.h>
#include "PhoneInteraction/PhoneInteraction.h"

#include <cstdlib>
#include <iostream>
#include <string.h>


using namespace std;

static PhoneInteraction *g_phoneInteraction;
static bool g_bUndo;


void updateStatus(const char *msg, bool waiting)
{
    cout << msg << endl;
}

void patchSpringBoard(bool bUndo)
{
    char *version = g_phoneInteraction->getPhoneProductVersion();
	
    if (strcmp(version, "1.1.1")) {
		cout << "Error: Cannot patch SpringBoard because phone is not using firmware 1.1.1." << endl;
		return;
    }
	
    if (!g_phoneInteraction->isPhoneJailbroken()) {
		cout << "Error: Cannot patch SpringBoard because phone is not jailbroken." << endl;
		return;
    }
	
    if (!g_phoneInteraction->enableThirdPartyApplications(g_bUndo)) {
		cout << "Error: Patch failed." << endl;
		return;
    }
	
    cout << "SpringBoard patching succeeded!" << endl << "Please reboot your phone by pressing and holding Home + Sleep for 3 seconds, powering it off, then powering it on again by pressing Sleep." << endl;
}

void phoneNotification(int type, const char *msg)
{
	
    switch (type) {
		case NOTIFY_CONNECTED:
			cout << "connected!" << endl;
			patchSpringBoard(g_bUndo);
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
	g_bUndo = false;

	if (argc > 1) {
		char *arg = argv[1];

		if (!strncmp(arg, "-undo", 5)) {
			g_bUndo = true;
		}
		else if (!strncmp(arg, "-?", 2) || !strncmp(arg, "-h", 2)) {
			cout << "Usage: " << argv[0] << " [-undo]" << endl;
			exit(0);
		}

	}

    g_phoneInteraction = PhoneInteraction::getInstance(updateStatus, phoneNotification);
    CFRunLoopRun();
    return 0;
}
