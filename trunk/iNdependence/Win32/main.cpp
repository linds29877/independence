/*
 * main.cpp
 * iNdependence
 *
 * Created by The Operator on 10/09/09
 * Copyright 2007 The Operator. All rights reserved.
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
 */

#if defined(WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <openssl/applink.c>
    #include <CoreFoundation.h>
#else
     #include <CoreFoundation/CoreFoundation.h>
#endif

#include <cstdlib>
#include <iostream>
#include "PhoneInteraction/PhoneInteraction.h"
#include "PhoneInteraction/UtilityFunctions.h"
#include "PhoneInteraction/CFCompatibility.h"


using namespace std;

static char *g_command;
static PhoneInteraction *g_phoneInteraction;
static bool g_returningToJail;
static bool g_performingJailbreak;

#ifdef WIN32
static bool g_run;
#endif

void stopRunLoop()
{
#ifdef WIN32
    g_run = false;
#elif __APPLE__
    CFRunLoopStop(CFRunLoopGetCurrent());
#endif
}

void executeCommand(const char *command)
{

    if (!strcmp(command, "jailbreak")) {

    	if (g_phoneInteraction->isPhoneJailbroken()) {
     	    cout << "Error: phone is already jailbroken" << endl;
     	    stopRunLoop();
     	    return;
    	}

     	string firmwarePath;

        cout << "Enter path to firmware files: ";
	    cin >> firmwarePath;

	    g_phoneInteraction->performJailbreak(firmwarePath.c_str(),
                                             "../Other Files/jailbreak files/fstab_mod",
					                         "../Other Files/jailbreak files/Services_mod.plist");
    }
    else if (!strcmp(command, "jailreturn")) {

        if (!g_phoneInteraction->isPhoneJailbroken()) {
	        cout << "Error: phone is not jailbroken" << endl;
	        stopRunLoop();
	        return;
        }

	    if (!g_phoneInteraction->putServicesFileOnPhone("../Other Files/jailbreak files/Services.plist")) {
	        cout << "Error: couldn't write Services.plist to phone" << endl;
	        stopRunLoop();
	        return;
        }

	    if (!g_phoneInteraction->putFstabFileOnPhone("../Other Files/jailbreak files/fstab")) {
	        cout << "Error: couldn't write fstab to phone" << endl;
	        stopRunLoop();
	        return;
        }

	    g_returningToJail = true;
	    cout << "Please press and hold the Home + Sleep buttons for 3 seconds, then power off your phone, then press Sleep again to restart it." << endl;
    }
    else if (!strcmp(command, "activate")) {

        if (g_phoneInteraction->isPhoneActivated()) {
	        cout << "Error: phone is already activated" << endl;
	        stopRunLoop();
	        return;
        }

	    if (!g_phoneInteraction->putPEMOnPhone("../Other Files/PEMs/iPhoneActivation.pem")) {
            cout << "Error: couldn't write new PEM file to phone" << endl;
            stopRunLoop();
            return;
        }

	    if (!g_phoneInteraction->activate(NULL, "../Other Files/PEMs/iPhoneActivation_private.pem")) {
            cout << "Error: couldn't activate phone" << endl;
            stopRunLoop();
            return;
        }

    }
    else if (!strcmp(command, "deactivate")) {

        if (!g_phoneInteraction->isPhoneActivated()) {
	        cout << "Error: phone is note activated" << endl;
	        stopRunLoop();
	        return;
        }

	    if (!g_phoneInteraction->putPEMOnPhone("../Other Files/PEMs/iPhoneActivation_original.pem")) {
            cout << "Error: couldn't write original PEM file to phone" << endl;
            stopRunLoop();
            return;
        }

	    if (!g_phoneInteraction->deactivate()) {
            cout << "Error: couldn't deactivate phone" << endl;
            stopRunLoop();
            return;
        }

   }

}

void updateStatus(const char *msg, bool waiting)
{
    cout << msg;

    if (waiting) {
	    cout << "..." << endl;
    }
    else {
        cout << endl;
    }

}

void phoneInteractionNotification(int type, const char *msg)
{

    switch (type) {
    case NOTIFY_CONNECTED:
	    cout << "connected!" << endl;

	    if (g_returningToJail) {
	        g_returningToJail = false;
	        cout << "Successfully returned to jail!" << endl;
	        stopRunLoop();
        }
	    else if (!g_performingJailbreak) {
	        executeCommand(g_command);
	    }

	    break;
    case NOTIFY_DISCONNECTED:
        cout << "disconnected!  Waiting for phone..." << endl;
	    break;
    case NOTIFY_DEACTIVATION_SUCCESS:
	    cout << "deactivation succeeded!" << endl;
	    stopRunLoop();
	    break;
    case NOTIFY_DEACTIVATION_FAILED:
        cout << "deactivation failed!  " << msg << endl;
	    break;
    case NOTIFY_PUTPEM_SUCCESS:
	    cout << "putpem succeeded!" << endl;
	    stopRunLoop();
	    break;
    case NOTIFY_PUTPEM_FAILED:
	    cout << "putpem failed!  " << msg << endl;
	    stopRunLoop();
	    break;
    case NOTIFY_ACTIVATION_SUCCESS:
	    cout << "activation succeeded!" << endl;
	    stopRunLoop();
	    break;
    case NOTIFY_ACTIVATION_FAILED:
	    cout << "activation failed!  "  << msg << endl;
	    stopRunLoop();
	    break;
    case NOTIFY_JAILBREAK_SUCCESS:
	    g_performingJailbreak = false;
	    cout << "jailbreak succeeded!" << endl;
	    stopRunLoop();
	    break;
    case NOTIFY_JAILBREAK_FAILED:
	    g_performingJailbreak = false;
	    cout << "jailbreak failed!  " << msg << endl;
	    stopRunLoop();
	    break;
    case NOTIFY_JAILBREAK_RECOVERY_WAIT:
	    cout << "Please press and hold Home + Sleep on your phone for 25 seconds..." << endl;
	    break;
    case NOTIFY_JAILBREAK_FAIL_USER_COULDNT_HOLD:
        cout << msg << endl;
	    stopRunLoop();
	    break;
    case NOTIFY_JAILBREAK_RECOVERY_CONNECTED:
	    g_performingJailbreak = true;
        cout << "Waiting for jailbreak..." << endl;
	    break;
    case NOTIFY_JAILBREAK_RECOVERY_DISCONNECTED:
	    cout << ".rcd." << endl;
	    break;
    case NOTIFY_JAILBREAK_RESTORE_CONNECTED:
	    cout << ".rsc." << endl;
	    break;
    case NOTIFY_JAILBREAK_RESTORE_DISCONNECTED:
	    cout << ".rsd." << endl;
	    break;
    case NOTIFY_INITIALIZATION_FAILED:
    case NOTIFY_WIN32_INITIALIZATION_FAILED:
    case NOTIFY_CONNECTION_FAILED:
    case NOTIFY_PUTSERVICES_FAILED:
    case NOTIFY_PUTFSTAB_FAILED:
	    cout << "Failure: " << msg << endl;
	    stopRunLoop();
	    break;
    case NOTIFY_PUTSERVICES_SUCCESS:
    case NOTIFY_PUTFSTAB_SUCCESS:
    case NOTIFY_JAILBREAK_CANCEL:
    case NOTIFY_CONNECTION_SUCCESS:
    case NOTIFY_INITIALIZATION_SUCCESS:
    default:
	    break;
    }

}

int main(int argc, char **argv)
{

    if (argc < 2) {
	    cout << "Usage: " << argv[0] << " <command>" << endl << endl;
	    cout << "where <command> is one of:" << endl;
	    cout << "     jailbreak     - performs a jailbreak on the phone" << endl;
	    cout << "     jailreturn    - returns to jail" << endl; 
	    cout << "     activate      - activates the phone" << endl;
	    cout << "     deactivate    - deactivates the phone" << endl;
	    return -1;
    }

    g_command = argv[1];
    g_returningToJail = false;
    g_performingJailbreak = false;
    g_phoneInteraction = PhoneInteraction::getInstance(updateStatus,
						       phoneInteractionNotification);

#ifdef WIN32
    g_run = true;
    while (g_run) {
	    Sleep(50);
    }
#elif __APPLE__
    CFRunLoopRun();
#endif

    return 0;
}