/*
 *  AppController.mm
 *  iNdependence
 *
 *  Created by The Operator on 23/08/07.
 *  Copyright 2007 The Operator. All rights reserved.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#import "AppController.h"
#import "MainWindow.h"
#import "SSHHandler.h"
#include "PhoneInteraction/UtilityFunctions.h"
#include "PhoneInteraction/PhoneInteraction.h"
#include "PhoneInteraction/SSHHelper.h"

#define PRE_FIRMWARE_UPGRADE_FILE "/private/var/root/Media/disk"


enum
{
	MENU_ITEM_ACTIVATE = 12,
	MENU_ITEM_DEACTIVATE = 13,
	MENU_ITEM_RETURN_TO_JAIL = 14,
	MENU_ITEM_JAILBREAK = 15,
	MENU_ITEM_INSTALL_SIM_UNLOCK = 16,
	MENU_ITEM_INSTALL_SSH = 17,
	MENU_ITEM_CHANGE_PASSWORD = 18,
	MENU_ITEM_REMOVE_SIM_UNLOCK = 19,
	MENU_ITEM_ENTER_DFU_MODE = 20,
	MENU_ITEM_REMOVE_SSH = 21,
	MENU_ITEM_PRE_FIRMWARE_UPGRADE = 22,
};

extern MainWindow *g_mainWindow;
static AppController *g_appController;
static PhoneInteraction *g_phoneInteraction;
static bool g_ignoreJailbreakSuccess;

static void updateStatus(const char *msg, bool waiting)
{
	
	if (g_mainWindow) {
		[g_mainWindow setStatus:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding] spinning:waiting];
	}
	
}

static void phoneInteractionNotification(int type, const char *msg)
{
	
	if (g_mainWindow) {
		
		switch (type) {
			case NOTIFY_CONNECTED:
				[g_appController setConnected:true];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_DISCONNECTED:
				[g_appController setConnected:false];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_AFC_CONNECTED:
				[g_appController setAFCConnected:true];
				break;
			case NOTIFY_AFC_DISCONNECTED:
				[g_appController setAFCConnected:false];
				break;
			case NOTIFY_INITIALIZATION_FAILED:
				[g_mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				[NSApp terminate:g_appController];
				break;
			case NOTIFY_CONNECTION_FAILED:
				[g_mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_AFC_CONNECTION_FAILED:
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_ACTIVATION_SUCCESS:
				[g_appController setActivated:g_phoneInteraction->isPhoneActivated()];
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_DEACTIVATION_SUCCESS:
				[g_appController setActivated:g_phoneInteraction->isPhoneActivated()];
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_JAILBREAK_SUCCESS:

				if (!g_ignoreJailbreakSuccess) {
					[g_mainWindow endDisplayWaitingSheet];
				}

				[g_appController setPerformingJailbreak:false];
				[g_appController setJailbroken:g_phoneInteraction->isPhoneJailbroken()];
				[g_mainWindow updateStatus];

				if ([g_appController isWaitingForActivation]) {
					[g_appController activateStageTwo:true];
				}
				else if ([g_appController isWaitingForDeactivation]) {
					[g_appController deactivateStageTwo];
				}
				else if (g_ignoreJailbreakSuccess) {
					g_ignoreJailbreakSuccess = false;
				}
				else {
					[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				}

				break;
			case NOTIFY_JAILRETURN_SUCCESS:
				[g_mainWindow endDisplayWaitingSheet];
				[g_appController setReturningToJail:false];
				[g_appController setJailbroken:g_phoneInteraction->isPhoneJailbroken()];
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_DFU_SUCCESS:
				[g_mainWindow endDisplayWaitingSheet];
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Success" message:@"Your phone is now in DFU mode and is ready for you to downgrade."];
				break;
			case NOTIFY_JAILBREAK_FAILED:
				[g_mainWindow endDisplayWaitingSheet];
				[g_appController setPerformingJailbreak:false];
				[g_appController setJailbroken:g_phoneInteraction->isPhoneJailbroken()];
				[g_mainWindow updateStatus];

				if ([g_appController isWaitingForActivation]) {
					[g_appController activationFailed:msg];
				}
				else if ([g_appController isWaitingForDeactivation]) {
					[g_appController deactivationFailed:msg];
				}
				else {
					[g_mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				}

				break;
			case NOTIFY_JAILRETURN_FAILED:
				[g_mainWindow endDisplayWaitingSheet];
				[g_appController setReturningToJail:false];
				[g_appController setJailbroken:g_phoneInteraction->isPhoneJailbroken()];
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_DFU_FAILED:
				[g_mainWindow endDisplayWaitingSheet];
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_ACTIVATION_FAILED:
			case NOTIFY_DEACTIVATION_FAILED:
				[g_appController setActivated:g_phoneInteraction->isPhoneActivated()];
			case NOTIFY_PUTSERVICES_FAILED:
			case NOTIFY_PUTFSTAB_FAILED:
			case NOTIFY_PUTPEM_FAILED:
			case NOTIFY_GET_ACTIVATION_FAILED:
			case NOTIFY_PUTFILE_FAILED:
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_GET_ACTIVATION_SUCCESS:
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_NEW_JAILBREAK_STAGE_ONE_WAIT:
				[g_mainWindow endDisplayWaitingSheet];
				[g_mainWindow startDisplayWaitingSheet:nil
											   message:@"Please press and hold the Sleep/Wake button for 3 seconds, then power off your phone, then press Sleep/Wake again to restart it."
												 image:[NSImage imageNamed:@"sleep_button"] cancelButton:false runModal:false];
				break;
			case NOTIFY_NEW_JAILBREAK_STAGE_TWO_WAIT:
				[g_mainWindow endDisplayWaitingSheet];

				if ([g_appController isWaitingForActivation]) {
					[g_appController activateStageTwo:false];
				}

				[g_mainWindow startDisplayWaitingSheet:nil
											   message:@"Please reboot your phone again using the same steps..."
												 image:[NSImage imageNamed:@"sleep_button"] cancelButton:false runModal:false];
				break;
			case NOTIFY_JAILBREAK_RECOVERY_WAIT:
				[g_mainWindow startDisplayWaitingSheet:nil message:@"Waiting for jail break..." image:[NSImage imageNamed:@"jailbreak"] cancelButton:false runModal:false];
				break;
			case NOTIFY_JAILRETURN_RECOVERY_WAIT:
				[g_mainWindow startDisplayWaitingSheet:nil message:@"Waiting for return to jail..." image:[NSImage imageNamed:@"jailbreak"] cancelButton:false runModal:false];
				break;
			case NOTIFY_DFU_RECOVERY_WAIT:
				[g_mainWindow startDisplayWaitingSheet:nil message:@"Waiting to enter DFU mode..." image:nil cancelButton:false runModal:false];
				break;
			case NOTIFY_RECOVERY_CONNECTED:
				[g_appController setRecoveryMode:true];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_RECOVERY_DISCONNECTED:
				[g_appController setRecoveryMode:false];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_RESTORE_CONNECTED:
				[g_appController setRestoreMode:true];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_RESTORE_DISCONNECTED:
				[g_appController setRestoreMode:false];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_DFU_CONNECTED:
				[g_appController setDFUMode:true];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_DFU_DISCONNECTED:
				[g_appController setDFUMode:false];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_JAILBREAK_CANCEL:
				[g_mainWindow endDisplayWaitingSheet];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_CONNECTION_SUCCESS:
			case NOTIFY_AFC_CONNECTION_SUCCESS:
			case NOTIFY_INITIALIZATION_SUCCESS:
			case NOTIFY_PUTFSTAB_SUCCESS:
			case NOTIFY_PUTSERVICES_SUCCESS:
			case NOTIFY_PUTPEM_SUCCESS:
			default:
				break;
		}
		
	}
	
}

@implementation AppController

- (void)dealloc
{
	
	if (m_phoneInteraction != NULL) {
		delete m_phoneInteraction;
	}

	if (m_sshPath != NULL) {
		free(m_sshPath);
	}

	[super dealloc];
}

- (void)awakeFromNib
{
	g_appController = self;

	if (!g_mainWindow) {
		g_mainWindow = mainWindow;
	}

	g_ignoreJailbreakSuccess = false;
	m_connected = false;
	m_afcConnected = false;
	m_recoveryMode = false;
	m_restoreMode = false;
	m_dfuMode = false;
	m_jailbroken = false;
	m_activated = false;
	m_performingJailbreak = false;
	m_returningToJail = false;
	m_installingSSH = false;
	m_waitingForActivation = false;
	m_waitingForDeactivation = false;
	m_waitingForNewActivation = false;
	m_waitingForNewDeactivation = false;
	m_bootCount = 0;
	m_sshPath = NULL;
	[customizeBrowser setEnabled:NO];
	m_phoneInteraction = PhoneInteraction::getInstance(updateStatus, phoneInteractionNotification);
	g_phoneInteraction = m_phoneInteraction;
}

- (void)setConnected:(bool)connected
{
	m_connected = connected;
	
	if (m_connected) {
		[self setAFCConnected:m_phoneInteraction->isConnectedToAFC()];
		[self setActivated:m_phoneInteraction->isPhoneActivated()];
		[self setJailbroken:m_phoneInteraction->isPhoneJailbroken()];

		if ([self isAFCConnected]) {

			if ([self isActivated]) {
				[activateButton setEnabled:NO];
				[deactivateButton setEnabled:YES];
			}
			else {
				[activateButton setEnabled:YES];
				[deactivateButton setEnabled:NO];
			}

		}
		else {
			[activateButton setEnabled:NO];
			[deactivateButton setEnabled:NO];
		}

		[enterDFUModeButton setEnabled:YES];

		if (m_installingSSH) {
			m_bootCount++;
			[mainWindow endDisplayWaitingSheet];

			if (m_bootCount == 1) {
				[self sshInstallStageTwo];
			}
			else {
				[self finishInstallingSSH:false];
			}

		}
		else if (m_waitingForNewActivation) {
			m_waitingForNewActivation = false;
			[mainWindow endDisplayWaitingSheet];
			[mainWindow displayAlert:@"Success" message:@"Successfully activated phone."];
		}
		else if (m_waitingForNewDeactivation) {
			m_waitingForNewDeactivation = false;
			[mainWindow endDisplayWaitingSheet];
			[mainWindow displayAlert:@"Success" message:@"Successfully deactivated phone."];
		}

	}
	else {
		[self setAFCConnected:false];
		[self setActivated:false];
		[self setJailbroken:false];

		[activateButton setEnabled:NO];
		[deactivateButton setEnabled:NO];
		[enterDFUModeButton setEnabled:NO];
	}
	
	[mainWindow updateStatus];
}

- (bool)isConnected
{
	return m_connected;
}

- (void)setAFCConnected:(bool)connected
{
	m_afcConnected = connected;

	if (m_afcConnected) {

		if ([self isActivated]) {
			[activateButton setEnabled:NO];
			[deactivateButton setEnabled:YES];
		}
		else {
			[activateButton setEnabled:YES];
			[deactivateButton setEnabled:NO];
		}

		if ([self isJailbroken]) {
			[jailbreakButton setEnabled:NO];
			[returnToJailButton setEnabled:YES];
		}
		else {
			[jailbreakButton setEnabled:YES];
			[returnToJailButton setEnabled:NO];
		}
		
	}
	else {
		[activateButton setEnabled:NO];
		[deactivateButton setEnabled:NO];
		[jailbreakButton setEnabled:NO];
		[returnToJailButton setEnabled:NO];
	}

	[mainWindow updateStatus];
}

- (bool)isAFCConnected
{
	return m_afcConnected;
}

- (void)setRecoveryMode:(bool)inRecovery
{
	m_recoveryMode = inRecovery;
	[mainWindow updateStatus];
}

- (bool)isInRecoveryMode
{
	return m_recoveryMode;
}

- (void)setRestoreMode:(bool)inRestore
{
	m_restoreMode = inRestore;
	[mainWindow updateStatus];
}

- (bool)isInRestoreMode
{
	return m_restoreMode;
}

- (void)setDFUMode:(bool)inDFU
{
	m_dfuMode = inDFU;
	[mainWindow updateStatus];
}

- (bool)isInDFUMode
{
	return m_dfuMode;
}

- (void)setJailbroken:(bool)jailbroken
{
	m_jailbroken = jailbroken;
	
	if (m_jailbroken) {
		[returnToJailButton setEnabled:YES];
		[customizeBrowser setEnabled:YES];
		[changePasswordButton setEnabled:YES];
		[jailbreakButton setEnabled:NO];

		if ([self isSSHInstalled]) {
			[installSSHButton setEnabled:NO];
			[removeSSHButton setEnabled:YES];

			if (!m_phoneInteraction->fileExists(PRE_FIRMWARE_UPGRADE_FILE)) {
				[preFirmwareUpgradeButton setEnabled:YES];
			}
			else {
				[preFirmwareUpgradeButton setEnabled:NO];
			}

			if ([self isanySIMInstalled]) {
				[installSimUnlockButton setEnabled:NO];
				[removeSimUnlockButton setEnabled:YES];
			}
			else {
				[installSimUnlockButton setEnabled:YES];
				[removeSimUnlockButton setEnabled:NO];
			}

		}
		else {
			[installSSHButton setEnabled:YES];
			[removeSSHButton setEnabled:NO];
			[installSimUnlockButton setEnabled:NO];
			[removeSimUnlockButton setEnabled:NO];
		}

	}
	else {
		[returnToJailButton setEnabled:NO];
		[installSSHButton setEnabled:NO];
		[removeSSHButton setEnabled:NO];
		[installSimUnlockButton setEnabled:NO];
		[removeSimUnlockButton setEnabled:NO];
		[changePasswordButton setEnabled:NO];
		[customizeBrowser setEnabled:NO];
		[preFirmwareUpgradeButton setEnabled:NO];

		if ([self isConnected] && [self isAFCConnected]) {
			[jailbreakButton setEnabled:YES];
		}
		else {
			[jailbreakButton setEnabled:NO];
		}

	}
	
	[mainWindow updateStatus];
}

- (bool)isJailbroken
{
	return m_jailbroken;
}

- (void)setActivated:(bool)activated
{
	m_activated = activated;

	if (m_activated) {

		if ([self isJailbroken]) {
			[activateButton setEnabled:NO];
			[deactivateButton setEnabled:YES];
		}

	}
	else {

		if ([self isJailbroken]) {
			[activateButton setEnabled:YES];
			[deactivateButton setEnabled:NO];
		}

	}

	[mainWindow updateStatus];
}

- (bool)isActivated
{
	return m_activated;
}

- (bool)isOpenSSHInstalled
{
	return m_phoneInteraction->fileExists("/usr/bin/sshd");
}

- (bool)isDropbearSSHInstalled
{
	return m_phoneInteraction->fileExists("/usr/bin/dropbear");
}

- (bool)isSSHInstalled
{
	return ([self isOpenSSHInstalled] || [self isDropbearSSHInstalled]);
}

- (bool)isanySIMInstalled
{
	return m_phoneInteraction->applicationExists("anySIM.app");
}

- (NSString*)phoneFirmwareVersion
{
	return [NSString stringWithCString:m_phoneInteraction->getPhoneProductVersion() encoding:NSUTF8StringEncoding];
}

- (bool)isUsing10xFirmware
{
	char *value = m_phoneInteraction->getPhoneProductVersion();

	if (!strncmp(value, "1.0", 3)) {
		return true;
	}

	return false;
}

- (void)setPerformingJailbreak:(bool)bJailbreaking
{
	m_performingJailbreak = bJailbreaking;
}

- (void)setReturningToJail:(bool)bReturning
{
	m_returningToJail = bReturning;
}

- (bool)isWaitingForActivation
{
	return m_waitingForActivation;
}

- (bool)isWaitingForNewActivation
{
	return m_waitingForNewActivation;
}

- (bool)isWaitingForDeactivation
{
	return m_waitingForDeactivation;
}

- (IBAction)performJailbreak:(id)sender
{

	if ([self isUsing10xFirmware]) {
		NSString *firmwarePath = nil;

		// first things first -- get the path to the unzipped firmware files
		NSOpenPanel *firmwareOpener = [NSOpenPanel openPanel];
		[firmwareOpener setTitle:@"Select where you unzipped the firmware files"];
		[firmwareOpener setCanChooseDirectories:YES];
		[firmwareOpener setCanChooseFiles:NO];
		[firmwareOpener setAllowsMultipleSelection:NO];

		while (1) {

			if ([firmwareOpener runModalForTypes:nil] != NSOKButton) {
				return;
			}

			firmwarePath = [firmwareOpener filename];

			if ([[NSFileManager defaultManager] fileExistsAtPath:[firmwarePath stringByAppendingString:@"/Restore.plist"]]) {
				break;
			}

			[mainWindow displayAlert:@"Error" message:@"Specified path does not contain firmware files.  Try again."];
			return;
		}

		NSString *servicesFile = [[NSBundle mainBundle] pathForResource:@"Services_mod" ofType:@"plist"];
	
		if (servicesFile == nil) {
			[mainWindow displayAlert:@"Error" message:@"Error finding modified Services.plist file."];
			return;
		}

		NSString *fstabFile = [[NSBundle mainBundle] pathForResource:@"fstab_mod" ofType:@""];
		
		if (fstabFile == nil) {
			[mainWindow displayAlert:@"Error" message:@"Error finding modified fstab file."];
			return;
		}

		m_performingJailbreak = true;
		m_phoneInteraction->performJailbreak([firmwarePath UTF8String], [fstabFile UTF8String],
											 [servicesFile UTF8String]);
	}
	else {
		NSString *servicesFile = [[NSBundle mainBundle] pathForResource:@"Services111_mod" ofType:@"plist"];
		
		if (servicesFile == nil) {
			[mainWindow displayAlert:@"Error" message:@"Error finding modified Services.plist file."];
			return;
		}
		
		m_performingJailbreak = true;
		m_phoneInteraction->performNewJailbreak([servicesFile UTF8String]);
	}

}

- (IBAction)returnToJail:(id)sender
{
	[mainWindow setStatus:@"Returning to jail..." spinning:true];

	NSString *servicesFile = nil;

	if ([self isUsing10xFirmware]) {
		servicesFile = [[NSBundle mainBundle] pathForResource:@"Services" ofType:@"plist"];
	}
	else {
		servicesFile = [[NSBundle mainBundle] pathForResource:@"Services111" ofType:@"plist"];
	}

	if (servicesFile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding Services.plist file."];
		[mainWindow updateStatus];
		return;
	}

	NSString *fstabFile = [[NSBundle mainBundle] pathForResource:@"fstab" ofType:@""];

	if (fstabFile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding fstab file."];
		[mainWindow updateStatus];
		return;
	}

	m_returningToJail = true;
	m_phoneInteraction->returnToJail([servicesFile UTF8String], [fstabFile UTF8String]);
}

- (IBAction)enterDFUMode:(id)sender
{
	NSString *firmwarePath;
	
	// first things first -- get the path to the unzipped firmware files
	NSOpenPanel *firmwareOpener = [NSOpenPanel openPanel];
	[firmwareOpener setTitle:@"Select where you unzipped the firmware files"];
	[firmwareOpener setCanChooseDirectories:YES];
	[firmwareOpener setCanChooseFiles:NO];
	[firmwareOpener setAllowsMultipleSelection:NO];
	
	while (1) {
		
		if ([firmwareOpener runModalForTypes:nil] != NSOKButton) {
			return;
		}
		
		firmwarePath = [firmwareOpener filename];
		
		if ([[NSFileManager defaultManager] fileExistsAtPath:[firmwarePath stringByAppendingString:@"/Restore.plist"]]) {
			break;
		}
		
		[mainWindow displayAlert:@"Error" message:@"Specified path does not contain firmware files.  Try again."];
		return;
	}
	
	m_phoneInteraction->enterDFUMode([firmwarePath UTF8String]);
}

- (IBAction)preFirmwareUpgrade:(id)sender
{

	if (m_phoneInteraction->fileExists(PRE_FIRMWARE_UPGRADE_FILE)) {
		[mainWindow displayAlert:@"Already done" message:@"It appears that you have already performed the pre-firmware operation.  If this is not the case, then remove the /private/var/root/Media/disk file from your phone using SSH/SFTP and try again."];
		return;
	}

	bool bCancelled = false;
	NSString *ipAddress, *password;
	
	if ([sshHandler getSSHInfo:&ipAddress password:&password wasCancelled:&bCancelled] == false) {
		return;
	}
	
	if (bCancelled) {
		return;
	}

	NSString *mknodFile = [[NSBundle mainBundle] pathForResource:@"mknod" ofType:@""];
	
	if (mknodFile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding mknod in bundle."];
		return;
	}
	
	m_phoneInteraction->removePath("/mknod");

	if (!m_phoneInteraction->putFile([mknodFile UTF8String], "/mknod")) {
		[mainWindow displayAlert:@"Error" message:@"Error writing /mknod to phone."];
		return;
	}

	bool done = false, taskSuccessful = false;
	int retval;
	
	while (!done) {
		[mainWindow startDisplayWaitingSheet:@"Performing Pre-Firmware Upgrade" message:@"Performing pre-firmware operations..." image:nil
								cancelButton:false runModal:false];
		retval = SSHHelper::mknodDisk([ipAddress UTF8String], [password UTF8String]);
		[mainWindow endDisplayWaitingSheet];

		if (retval != SSH_HELPER_SUCCESS) {

			switch (retval)
			{
				case SSH_HELPER_ERROR_NO_RESPONSE:
					[mainWindow displayAlert:@"Failed" message:@"Couldn't connect to SSH server.  Ensure IP address is correct, phone is connected to a network, and SSH is installed correctly."];
					done = true;
					break;
				case SSH_HELPER_ERROR_BAD_PASSWORD:
					[mainWindow displayAlert:@"Failed" message:@"root password is incorrect."];
					done = true;
					break;
				case SSH_HELPER_VERIFICATION_FAILED:
					NSString *msg = [NSString stringWithFormat:@"Host verification failed.\n\nWould you like iNdependence to try and fix this for you by editing %@/.ssh/known_hosts?", NSHomeDirectory()];
					int retval = NSRunAlertPanel(@"Failed", msg, @"Yes", @"No", nil);

					if (retval == NSAlertDefaultReturn) {

						if (![sshHandler removeKnownHostsEntry:ipAddress]) {
							msg = [NSString stringWithFormat:@"Couldn't remove entry from %@/.ssh/known_hosts.  Please edit that file by hand and remove the line containing your phone's IP address.", NSHomeDirectory()];
							[mainWindow displayAlert:@"Failed" message:msg];
							done = true;
						}
						
					}
					else {
						done = true;
					}

					break;
				default:
					[mainWindow displayAlert:@"Failed" message:@"Error performing pre-firmware operations."];
					done = true;
					break;
			}

		}
		else {
			done = true;
			taskSuccessful = true;
		}

	}

	m_phoneInteraction->removePath("/mknod");

	if (taskSuccessful) {
		[preFirmwareUpgradeButton setEnabled:false];
		[mainWindow displayAlert:@"Success" message:@"Your phone is now ready to be upgraded.\n\nPlease quit iNdependence, then use iTunes to do this now.\n\nEnsure that you choose 'Update' and not 'Restore' in iTunes."];
	}

}

- (IBAction)installSimUnlock:(id)sender
{
	[mainWindow displayAlert:@"Alert" message:@"Please note that anySIM is not able to SIM unlock phones which came with the 1.1.2 firmware out of the box."];

	char *value = m_phoneInteraction->getPhoneBasebandVersion();
	NSString *simUnlockApp = nil;
	bool bUsingAnysim12With112 = false;

	if (!strcmp(value, "04.02.13_G")) {
		value = m_phoneInteraction->getPhoneProductVersion();

		if (!strcmp(value, "1.0.2") || !strcmp(value, "1.1.1") || !strcmp(value, "1.1.2")) {
			simUnlockApp = [[NSBundle mainBundle] pathForResource:@"anySIM_12" ofType:@"app"];

			if (!strcmp(value, "1.1.2")) {
				bUsingAnysim12With112 = true;
			}

		}
		else {
			NSString *msg = [NSString stringWithFormat:@"anySIM 1.2 is required for SIM unlocking.  However, your firmware version (%s) isn't compatible with anySIM 1.2.\n\nPlease upgrade your phone to firmware version 1.0.2 or higher.", value];
			[mainWindow displayAlert:@"Error" message:msg];
			return;
		}

	}
	else {
		simUnlockApp = [[NSBundle mainBundle] pathForResource:@"anySIM_11" ofType:@"app"];
	}

	if (simUnlockApp == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding SIM unlock application in bundle."];
		return;
	}

	bool bCancelled = false;
	NSString *ipAddress, *password;

	if ([sshHandler getSSHInfo:&ipAddress password:&password wasCancelled:&bCancelled] == false) {
		return;
	}

	if (bCancelled) {
		return;
	}

	if (!m_phoneInteraction->putApplicationOnPhone([simUnlockApp UTF8String], "anySIM.app")) {
		[mainWindow displayAlert:@"Error" message:@"Couldn't put application on phone"];
		return;
	}

	char *appPath = "/Applications/anySIM.app";
	bool done = false, permsSet = false;
	int retval;
	
	while (!done) {
		[mainWindow startDisplayWaitingSheet:@"Setting Permissions" message:@"Setting application permissions..." image:nil
								cancelButton:false runModal:false];
		retval = SSHHelper::copyPermissions([simUnlockApp UTF8String], appPath, [ipAddress UTF8String],
											[password UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		
		if (retval != SSH_HELPER_SUCCESS) {
			
			switch (retval)
			{
				case SSH_HELPER_ERROR_NO_RESPONSE:
					PhoneInteraction::getInstance()->removeApplication([[simUnlockApp lastPathComponent] UTF8String]);
					[mainWindow displayAlert:@"Failed" message:@"Couldn't connect to SSH server.  Ensure IP address is correct, phone is connected to a network, and SSH is installed correctly."];
					done = true;
					break;
				case SSH_HELPER_ERROR_BAD_PASSWORD:
					PhoneInteraction::getInstance()->removeApplication([[simUnlockApp lastPathComponent] UTF8String]);
					[mainWindow displayAlert:@"Failed" message:@"root password is incorrect."];
					done = true;
					break;
				case SSH_HELPER_VERIFICATION_FAILED:
					NSString *msg = [NSString stringWithFormat:@"Host verification failed.\n\nWould you like iNdependence to try and fix this for you by editing %@/.ssh/known_hosts?", NSHomeDirectory()];
					int retval = NSRunAlertPanel(@"Failed", msg, @"Yes", @"No", nil);
					
					if (retval == NSAlertDefaultReturn) {
						
						if (![sshHandler removeKnownHostsEntry:ipAddress]) {
							PhoneInteraction::getInstance()->removeApplication([[simUnlockApp lastPathComponent] UTF8String]);
							msg = [NSString stringWithFormat:@"Couldn't remove entry from %@/.ssh/known_hosts.  Please edit that file by hand and remove the line containing your phone's IP address.", NSHomeDirectory()];
							[mainWindow displayAlert:@"Failed" message:msg];
							done = true;
						}
						
					}
					else {
						done = true;
					}

					break;
				default:
					PhoneInteraction::getInstance()->removeApplication([[simUnlockApp lastPathComponent] UTF8String]);
					[mainWindow displayAlert:@"Failed" message:@"Error setting permissions for application."];
					done = true;
					break;
			}
			
		}
		else {
			permsSet = true;
			done = true;
		}
		
	}

	if ([self isanySIMInstalled]) {
		[installSimUnlockButton setEnabled:NO];
		[removeSimUnlockButton setEnabled:YES];
	}
	else {
		[installSimUnlockButton setEnabled:YES];
		[removeSimUnlockButton setEnabled:NO];
	}

	if (permsSet) {

		if (bUsingAnysim12With112) {
			[mainWindow displayAlert:@"Success" message:@"The anySIM application should now be installed on your phone.\n\nPlease note that you are using anySIM 1.2 in combination with firmware 1.1.2, so you must put your phone in Airplane mode before you run anySIM.  Please put your phone in Airplane mode now, then run anySIM and follow the instructions to complete the SIM unlock process.\n\nAfter you are done, you can turn Airplane mode off again and remove anySIM from your phone."];
		}
		else {
			[mainWindow displayAlert:@"Success" message:@"The anySIM application should now be installed on your phone.  Simply run it, follow the instructions, and it will complete the SIM unlock process.\n\nAfter you are done, you can remove it from your phone."];
		}

	}

}

- (void)removeSimUnlock:(id)sender
{
	bool bCancelled = false;
	NSString *ipAddress, *password;
	
	if ([sshHandler getSSHInfo:&ipAddress password:&password wasCancelled:&bCancelled] == false) {
		return;
	}
	
	if (bCancelled) {
		return;
	}

	if (m_phoneInteraction->applicationExists("anySIM.app")) {

		if (!m_phoneInteraction->removeApplication("anySIM.app")) {
			[mainWindow displayAlert:@"Failed" message:@"Couldn't remove existing version of anySIM from phone."];
			return;
		}

	}

	if ([self isanySIMInstalled]) {
		[installSimUnlockButton setEnabled:NO];
		[removeSimUnlockButton setEnabled:YES];
	}
	else {
		[installSimUnlockButton setEnabled:YES];
		[removeSimUnlockButton setEnabled:NO];
	}
	
	bool done = false, sbRestarted = false;
	int retval;
	
	while (!done) {
		[mainWindow startDisplayWaitingSheet:@"Restarting SpringBoard" message:@"Restarting SpringBoard..." image:nil
								cancelButton:false runModal:false];
		retval = SSHHelper::restartSpringboard([ipAddress UTF8String], [password UTF8String]);
		[mainWindow endDisplayWaitingSheet];

		if (retval != SSH_HELPER_SUCCESS) {
			
			switch (retval)
			{
				case SSH_HELPER_ERROR_NO_RESPONSE:
					[mainWindow displayAlert:@"Failed" message:@"Couldn't connect to SSH server.  Ensure IP address is correct, phone is connected to a network, and SSH is installed correctly."];
					done = true;
					break;
				case SSH_HELPER_ERROR_BAD_PASSWORD:
					[mainWindow displayAlert:@"Failed" message:@"root password is incorrect."];
					done = true;
					break;
				case SSH_HELPER_VERIFICATION_FAILED:
					NSString *msg = [NSString stringWithFormat:@"Host verification failed.\n\nWould you like iNdependence to try and fix this for you by editing %@/.ssh/known_hosts?", NSHomeDirectory()];
					int retval = NSRunAlertPanel(@"Failed", msg, @"Yes", @"No", nil);
					
					if (retval == NSAlertDefaultReturn) {
						
						if (![sshHandler removeKnownHostsEntry:ipAddress]) {
							msg = [NSString stringWithFormat:@"Couldn't remove entry from %@/.ssh/known_hosts.  Please edit that file by hand and remove the line containing your phone's IP address.", NSHomeDirectory()];
							[mainWindow displayAlert:@"Failed" message:msg];
							done = true;
						}
						
					}
					else {
						done = true;
					}

					break;
				default:
					[mainWindow displayAlert:@"Failed" message:@"Error restarting SpringBoard."];
					done = true;
					break;
			}
			
		}
		else {
			sbRestarted = true;
			done = true;
		}
		
	}

	if (sbRestarted) {
		[mainWindow displayAlert:@"Success" message:@"The anySIM application was successfully removed from your phone."];
	}
	else {
		[mainWindow displayAlert:@"Success" message:@"The anySIM application was successfully removed from your phone.  However, you'll need to reboot your phone to have it disappear from the SpringBoard menu."];
	}

}

- (bool)doPutPEM:(const char*)pemfile
{
	[mainWindow setStatus:@"Putting PEM file on phone..." spinning:true];
	return m_phoneInteraction->putPEMOnPhone(pemfile);
}

- (void)activateStageTwo:(bool)displaySheet
{
	m_waitingForActivation = false;
		
	if (!m_phoneInteraction->factoryActivate()) {
		[mainWindow displayAlert:@"Error" message:@"Error during activation."];
		return;
	}

	if (!m_phoneInteraction->enableYouTube()) {
		[mainWindow displayAlert:@"Error" message:@"Error enabling YouTube."];
		return;
	}

	m_waitingForNewActivation = true;
	g_ignoreJailbreakSuccess = true;
		
	if (displaySheet) {
		[g_mainWindow startDisplayWaitingSheet:nil
									   message:@"Please press and hold the Sleep/Wake button for 3 seconds, then power off your phone, then press Sleep/Wake again to restart it."
										 image:[NSImage imageNamed:@"sleep_button"] cancelButton:false runModal:false];
	}

}

- (void)activationFailed:(const char*)msg
{
	m_waitingForActivation = false;
	[mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
}

- (void)deactivateStageTwo
{
	m_waitingForDeactivation = false;

	if (!m_phoneInteraction->factoryActivate(true)) {
		[mainWindow displayAlert:@"Error" message:@"Error during deactivation."];
		return;
	}

	if (!m_phoneInteraction->enableYouTube(true)) {
		[mainWindow displayAlert:@"Error" message:@"Error disabling YouTube."];
		return;
	}

	m_waitingForNewDeactivation = true;
	[g_mainWindow startDisplayWaitingSheet:nil
								   message:@"Please press and hold the Sleep/Wake button for 3 seconds, then power off your phone, then press Sleep/Wake again to restart it."
									 image:[NSImage imageNamed:@"sleep_button"] cancelButton:false runModal:false];
}

- (void)deactivationFailed:(const char*)msg
{
	m_waitingForDeactivation = false;
	[mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
}

- (IBAction)activate:(id)sender
{
	m_waitingForActivation = true;
	
	if (!m_phoneInteraction->isPhoneJailbroken()) {
		[self performJailbreak:sender];
		return;
	}
	
	[self activateStageTwo:true];
}

- (IBAction)deactivate:(id)sender
{
	m_waitingForDeactivation = true;
	
	if (!m_phoneInteraction->isPhoneJailbroken()) {
		[self performJailbreak:sender];
		return;
	}
	
	[self deactivateStageTwo];
}

- (IBAction)waitDialogCancel:(id)sender
{

	if (m_installingSSH) {
		m_installingSSH = false;
		[mainWindow endDisplayWaitingSheet];
		[self finishInstallingSSH:true];
	}
	
}

- (IBAction)changePassword:(id)sender
{
	[NSApp beginSheet:newPasswordDialog modalForWindow:mainWindow modalDelegate:nil didEndSelector:nil
		  contextInfo:nil];

	const char *accountName = NULL;
	const char *newPassword = NULL;

	while ( !accountName || !newPassword ) {

		if ([NSApp runModalForWindow:newPasswordDialog] == -1) {
			[NSApp endSheet:newPasswordDialog];
			[newPasswordDialog orderOut:self];
			return;
		}

		[NSApp endSheet:newPasswordDialog];
		[newPasswordDialog orderOut:self];

		if ([[accountNameField stringValue] length] == 0) {
			[mainWindow displayAlert:@"Error" message:@"Invalid account name.  Try again."];
			continue;
		}

		if ([[passwordField stringValue] length] == 0) {
			[mainWindow displayAlert:@"Error" message:@"Invalid password.  Try again."];
			continue;
		}

		if (![[passwordField stringValue] isEqualToString:[passwordAgainField stringValue]]) {
			[mainWindow displayAlert:@"Error" message:@"Passwords don't match.  Try again."];
			continue;
		}

		accountName = [[accountNameField stringValue] UTF8String];
		newPassword = [[passwordField stringValue] UTF8String];
	}

	int size = 0;
	char *buf, *offset;

	if (!m_phoneInteraction->getFileData((void**)&buf, &size, "/etc/master.passwd")) {
		[mainWindow displayAlert:@"Error" message:@"Error reading /etc/master.passwd from phone."];
		return;
	}

	int accountLen = strlen(accountName);
	char pattern[accountLen+2];

	strcpy(pattern, accountName);
	pattern[accountLen] = ':';
	pattern[accountLen+1] = 0;

	if ( (offset = strstr(buf, pattern)) == NULL ) {
		free(buf);
		[mainWindow displayAlert:@"Error" message:@"No such account name in master.passwd."];
		return;
	}

	char *encryptedPassword = crypt(newPassword, "XU");
	
	if (encryptedPassword == NULL) {
		free(buf);
		[mainWindow displayAlert:@"Error" message:@"Error encrypting given password."];
		return;
	}

	strncpy(offset + accountLen + 1, encryptedPassword, 13);

	if (!m_phoneInteraction->putData(buf, size, "/etc/master.passwd")) {
		free(buf);
		[mainWindow displayAlert:@"Error" message:@"Error writing to /etc/master.passwd on phone."];
		return;
	}

	free(buf);
	[mainWindow displayAlert:@"Success" message:@"Successfully changed account password."];
}

- (IBAction)passwordDialogCancel:(id)sender
{
	[NSApp stopModalWithCode:-1];
}

- (IBAction)passwordDialogOk:(id)sender
{
	[NSApp stopModalWithCode:0];
}

- (IBAction)installSSH:(id)sender
{
	[mainWindow startDisplayWaitingSheet:nil
								 message:@"Generating SSH keys..."
								   image:nil cancelButton:false runModal:false];

	// first generate the RSA and DSA keys
	NSString *sshKeygenPath = @"/usr/bin/ssh-keygen";
	NSString *tmpDir = NSTemporaryDirectory();
	NSMutableString *sshHostKey = [NSMutableString stringWithString:tmpDir];
	[sshHostKey appendString:@"/ssh_host_key"];
	NSMutableString *sshHostKeyPub = [NSMutableString stringWithString:sshHostKey];
	[sshHostKeyPub appendString:@".pub"];

	// remove old files if they exist
	remove([sshHostKey UTF8String]);
	remove([sshHostKeyPub UTF8String]);

	NSArray *args = [NSArray arrayWithObjects:@"-t", @"rsa1", @"-f", sshHostKey, @"-N", @"", nil];
	NSTask *task = [[NSTask alloc] init];
	[task setLaunchPath:sshKeygenPath];
	[task setArguments:args];
	[task launch];
	[task waitUntilExit];

	if ([task terminationStatus] != 0) {
		[task release];
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error occurred while executing ssh-keygen."];
		return;
	}

	[task release];

	NSMutableString *sshHostRSAKey = [NSMutableString stringWithString:tmpDir];
	[sshHostRSAKey appendString:@"/ssh_host_rsa_key"];
	NSMutableString *sshHostRSAKeyPub = [NSMutableString stringWithString:sshHostRSAKey];
	[sshHostRSAKeyPub appendString:@".pub"];

	// remove old files if they exist
	remove([sshHostRSAKey UTF8String]);
	remove([sshHostRSAKeyPub UTF8String]);

	args = [NSArray arrayWithObjects:@"-t", @"rsa", @"-f", sshHostRSAKey, @"-N", @"", nil];
	task = [[NSTask alloc] init];
	[task setLaunchPath:sshKeygenPath];
	[task setArguments:args];
	[task launch];
	[task waitUntilExit];

	if ([task terminationStatus] != 0) {
		[task release];
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error occurred while executing ssh-keygen."];
		return;
	}

	[task release];

	NSMutableString *sshHostDSAKey = [NSMutableString stringWithString:tmpDir];
	[sshHostDSAKey appendString:@"/ssh_host_dsa_key"];
	NSMutableString *sshHostDSAKeyPub = [NSMutableString stringWithString:sshHostDSAKey];
	[sshHostDSAKeyPub appendString:@".pub"];
	
	// remove old files if they exist
	remove([sshHostDSAKey UTF8String]);
	remove([sshHostDSAKeyPub UTF8String]);

	args = [NSArray arrayWithObjects:@"-t", @"dsa", @"-f", sshHostDSAKey, @"-N", @"", nil];
	task = [[NSTask alloc] init];
	[task setLaunchPath:sshKeygenPath];
	[task setArguments:args];
	[task launch];
	[task waitUntilExit];
	
	if ([task terminationStatus] != 0) {
		[task release];
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error occurred while executing ssh-keygen."];
		return;
	}
	
	[task release];

	if (!m_phoneInteraction->createDirectory("/etc/ssh")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error creating /etc/ssh directory on phone."];
		return;
	}

	if (!m_phoneInteraction->putFile([sshHostKey UTF8String], "/etc/ssh/ssh_host_key")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /etc/ssh/ssh_host_key to phone."];
		return;
	}

	if (!m_phoneInteraction->putFile([sshHostKeyPub UTF8String], "/etc/ssh/ssh_host_key.pub")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /etc/ssh/ssh_host_key.pub to phone."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([sshHostRSAKey UTF8String], "/etc/ssh/ssh_host_rsa_key")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /etc/ssh/ssh_host_rsa_key to phone."];
		return;
	}

	if (!m_phoneInteraction->putFile([sshHostRSAKeyPub UTF8String], "/etc/ssh/ssh_host_rsa_key.pub")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /etc/ssh/ssh_host_rsa_key.pub to phone."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([sshHostDSAKey UTF8String], "/etc/ssh/ssh_host_dsa_key")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /etc/ssh/ssh_host_dsa_key to phone."];
		return;
	}

	if (!m_phoneInteraction->putFile([sshHostDSAKeyPub UTF8String], "/etc/ssh/ssh_host_dsa_key.pub")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /etc/ssh/ssh_host_dsa_key.pub to phone."];
		return;
	}
	
	NSString *sshdConfigFile = [[NSBundle mainBundle] pathForResource:@"sshd_config" ofType:@""];

	if (sshdConfigFile == nil) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error finding sshd_config in bundle."];
		return;
	}

	if (!m_phoneInteraction->putFile([sshdConfigFile UTF8String], "/etc/ssh/sshd_config")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /etc/ssh/sshd_config to phone."];
		return;
	}

	NSString *chmodFile = [[NSBundle mainBundle] pathForResource:@"chmod" ofType:@""];

	if (chmodFile == nil) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error finding chmod in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([chmodFile UTF8String], "/bin/chmod")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /bin/chmod to phone."];
		return;
	}

	NSString *shFile = [[NSBundle mainBundle] pathForResource:@"sh" ofType:@""];

	if (shFile == nil) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error finding sh in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([shFile UTF8String], "/bin/sh")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /bin/sh to phone."];
		return;
	}

	NSString *sftpFile = [[NSBundle mainBundle] pathForResource:@"sftp-server" ofType:@""];

	if (sftpFile == nil) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error finding sftp-server in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([sftpFile UTF8String], "/usr/bin/sftp-server")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/bin/sftp-server to phone."];
		return;
	}

	NSString *scpFile = [[NSBundle mainBundle] pathForResource:@"scp" ofType:@""];
	
	if (scpFile == nil) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error finding scp in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([scpFile UTF8String], "/usr/bin/scp")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/bin/scp to phone."];
		return;
	}

	NSString *libarmfpFile = [[NSBundle mainBundle] pathForResource:@"libarmfp" ofType:@"dylib"];
	
	if (libarmfpFile == nil) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error finding libarmfp.dylib in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([libarmfpFile UTF8String], "/usr/lib/libarmfp.dylib")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/lib/libarmfp.dylib to phone."];
		return;
	}
	
	NSString *sshdFile = [[NSBundle mainBundle] pathForResource:@"sshd" ofType:@""];

	if (sshdFile == nil) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error finding sshd in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([sshdFile UTF8String], "/usr/bin/sshd")) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/bin/sshd to phone."];
		return;
	}
	
	NSMutableString *tmpFilePath = [NSMutableString stringWithString:tmpDir];
	[tmpFilePath appendString:@"/update.backup.iNdependence"];

	// remove old file if it exists
	remove([tmpFilePath UTF8String]);

	if (!m_phoneInteraction->getFile("/usr/sbin/update", [tmpFilePath UTF8String])) {
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error reading /usr/sbin/update from phone."];
		return;
	}

	if (!m_phoneInteraction->putFile([chmodFile UTF8String], "/usr/sbin/update")) {
		remove([tmpFilePath UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/sbin/update to phone."];
		return;
	}

	NSMutableString *tmpFilePath2 = [NSMutableString stringWithString:tmpDir];
	[tmpFilePath2 appendString:@"/com.apple.update.plist.backup.iNdependence"];

	// remove old file if it exists
	remove([tmpFilePath2 UTF8String]);

	if (!m_phoneInteraction->getFile("/System/Library/LaunchDaemons/com.apple.update.plist", [tmpFilePath2 UTF8String])) {
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		remove([tmpFilePath UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error reading /System/Library/LaunchDaemons/com.apple.update.plist from phone."];
		return;
	}

	int fd = open([tmpFilePath2 UTF8String], O_RDONLY, 0);

	if (fd == -1) {
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error opening com.apple.update.plist.backup.iNdependence for reading."];
		return;
	}

	struct stat st;

	if (fstat(fd, &st) == -1) {
		close(fd);
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error obtaining com.apple.update.plist.original file size."];
		return;
	}

	NSMutableString *tmpFilePath3 = [NSMutableString stringWithString:tmpDir];
	[tmpFilePath3 appendString:@"/com.apple.update.plist.iNdependence"];
	int fd2 = open([tmpFilePath3 UTF8String], O_CREAT | O_TRUNC | O_WRONLY,
				   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (fd2 == -1) {
		close(fd);
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error opening com.apple.update.plist.iNdependence for writing."];
		return;
	}

	unsigned char buf[1024];
	int readCount = 0;

	while (readCount < st.st_size) {
		int retval = read(fd, buf, 1024);

		if (retval < 1) {
			break;
		}

		write(fd2, buf, retval);
		readCount += retval;
	}

	close(fd);
	close(fd2);

	if (readCount < st.st_size) {
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error copying com.apple.update.plist."];
		return;
	}

	NSDictionary *dict = [NSDictionary dictionaryWithContentsOfFile:tmpFilePath3];
	NSMutableDictionary *mutDict = [NSMutableDictionary dictionaryWithCapacity:[dict count]];
	[mutDict addEntriesFromDictionary:dict];
	NSMutableArray *mutArgs = [NSMutableArray arrayWithCapacity:5];
	[mutArgs addObject:@"/usr/sbin/update"];
	[mutArgs addObject:@"555"];
	[mutArgs addObject:@"/bin/chmod"];
	[mutArgs addObject:@"/bin/sh"];
	[mutArgs addObject:@"/usr/bin/sshd"];
	[mutArgs addObject:@"/usr/bin/sftp-server"];
	[mutArgs addObject:@"/usr/bin/scp"];
	[mutDict setObject:mutArgs forKey:@"ProgramArguments"];

	if (remove([tmpFilePath3 UTF8String]) == -1) {
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error deleting com.apple.update.plist.iNdependence"];
		return;
	}

	if (![mutDict writeToFile:tmpFilePath3 atomically:YES]) {
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error creating new com.apple.update.plist."];
		return;
	}

	if (!m_phoneInteraction->putFile([tmpFilePath3 UTF8String], "/System/Library/LaunchDaemons/com.apple.update.plist")) {
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /System/Library/LaunchDaemons/com.apple.update.plist to phone."];
		return;
	}

	NSString *sshPlistFile = [[NSBundle mainBundle] pathForResource:@"org.thebends.openssh" ofType:@"plist"];

	if (sshPlistFile == nil) {
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		m_phoneInteraction->putFile([tmpFilePath2 UTF8String], "/System/Library/LaunchDaemons/com.apple.update.plist");
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error finding org.thebends.openssh.plist in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([sshPlistFile UTF8String], "/System/Library/LaunchDaemons/org.thebends.openssh.plist")) {
		m_phoneInteraction->putFile([tmpFilePath UTF8String], "/usr/sbin/update");
		m_phoneInteraction->putFile([tmpFilePath2 UTF8String], "/System/Library/LaunchDaemons/com.apple.update.plist");
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
		[mainWindow endDisplayWaitingSheet];
		[mainWindow displayAlert:@"Error" message:@"Error writing /System/Library/LaunchDaemons/org.thebends.openssh.plist to phone."];
		return;
	}

	m_installingSSH = true;
	m_bootCount = 0;

	[mainWindow endDisplayWaitingSheet];
	[mainWindow startDisplayWaitingSheet:nil
								 message:@"Please press and hold the Sleep/Wake button for 3 seconds, then power off your phone, then press Sleep/Wake again to restart it."
								   image:[NSImage imageNamed:@"sleep_button"] cancelButton:true runModal:false];
}

- (void)sshInstallStageTwo
{
	NSMutableString *backupFilePath = [NSMutableString stringWithString:NSTemporaryDirectory()];
	[backupFilePath appendString:@"/com.apple.update.plist.iNdependence"];

	NSDictionary *dict = [NSDictionary dictionaryWithContentsOfFile:backupFilePath];
	NSMutableDictionary *mutDict = [NSMutableDictionary dictionaryWithCapacity:[dict count]];
	[mutDict addEntriesFromDictionary:dict];
	NSMutableArray *mutArgs = [NSMutableArray arrayWithCapacity:5];
	[mutArgs addObject:@"/usr/sbin/update"];
	[mutArgs addObject:@"600"];
	[mutArgs addObject:@"/etc/ssh/ssh_host_key"];
	[mutArgs addObject:@"/etc/ssh/ssh_host_rsa_key"];
	[mutArgs addObject:@"/etc/ssh/ssh_host_dsa_key"];
	[mutDict setObject:mutArgs forKey:@"ProgramArguments"];
	
	remove([backupFilePath UTF8String]);
	
	if (![mutDict writeToFile:backupFilePath atomically:YES]) {
		[self finishInstallingSSH:true];
		[mainWindow displayAlert:@"Error" message:@"Error creating new com.apple.update.plist."];
		return;
	}

	if (!m_phoneInteraction->putFile([backupFilePath UTF8String], "/System/Library/LaunchDaemons/com.apple.update.plist")) {
		[self finishInstallingSSH:true];
		[mainWindow displayAlert:@"Error" message:@"Error writing /System/Library/LaunchDaemons/com.apple.update.plist to phone."];
		return;
	}

	[mainWindow startDisplayWaitingSheet:nil
								 message:@"Please reboot your phone again using the same steps..."
								   image:[NSImage imageNamed:@"sleep_button"] cancelButton:true runModal:false];
}

- (void)finishInstallingSSH:(bool)bCancelled
{
	m_installingSSH = false;
	m_bootCount = 0;

	NSString *tmpDir = NSTemporaryDirectory();
	NSMutableString *backupFilePath = [NSMutableString stringWithString:tmpDir];
	[backupFilePath appendString:@"/com.apple.update.plist.backup.iNdependence"];
	NSMutableString *backupFilePath2 = [NSMutableString stringWithString:tmpDir];
	[backupFilePath2 appendString:@"/update.backup.iNdependence"];
	NSMutableString *backupFilePath3 = [NSMutableString stringWithString:tmpDir];
	[backupFilePath3 appendString:@"/com.apple.update.plist.iNdependence"];

	if (!m_phoneInteraction->putFile([backupFilePath UTF8String], "/System/Library/LaunchDaemons/com.apple.update.plist")) {
		m_phoneInteraction->putFile([backupFilePath2 UTF8String], "/usr/sbin/update");
		remove([backupFilePath UTF8String]);
		remove([backupFilePath2 UTF8String]);
		remove([backupFilePath3 UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error restoring original /System/Library/LaunchDaemons/com.apple.update.plist on phone.  Please try installing SSH again."];
		return;
	}

	if (!m_phoneInteraction->putFile([backupFilePath2 UTF8String], "/usr/sbin/update")) {
		remove([backupFilePath UTF8String]);
		remove([backupFilePath2 UTF8String]);
		remove([backupFilePath3 UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error restoring original /usr/sbin/update on phone.  Please try installing SSH again."];
		return;
	}

	// clean up
	remove([backupFilePath UTF8String]);
	remove([backupFilePath2 UTF8String]);
	remove([backupFilePath3 UTF8String]);

	if (!bCancelled) {
		[mainWindow displayAlert:@"Success" message:@"Successfully installed SSH, SFTP, and SCP on your phone."];
	}

}

- (IBAction)removeSSH:(id)sender
{
	int retval = NSRunAlertPanel(@"Alert", @"During SSH installation, a file named libarmfp.dylib was installed.  This is needed by SSH, but is also needed by many 3rd party applications.  Removing it could render many 3rd party applications you have installed inoperable.\n\nWould you like to remove libarmfp.dylib during SSH removal?", @"No", @"Yes", nil);
	bool bRemoveLibarmfp = false;

	if (retval == NSAlertAlternateReturn) {
		bRemoveLibarmfp = true;
	}

	if ([self isDropbearSSHInstalled]) {

		if (!m_phoneInteraction->removePath("/usr/bin/dropbear")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /usr/bin/dropbear from phone."];
			return;
		}

		if (!m_phoneInteraction->removePath("/usr/libexec/sftp-server")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /usr/libexec/sftp-server from phone."];
			return;
		}
	
		if (!m_phoneInteraction->removePath("/etc/dropbear/dropbear_rsa_host_key")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/dropbear/dropbear_rsa_host_key from phone."];
			return;
		}

		if (!m_phoneInteraction->removePath("/etc/dropbear/dropbear_dss_host_key")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/dropbear/dropbear_dss_host_key from phone."];
			return;
		}

		if (!m_phoneInteraction->removePath("/etc/dropbear")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/dropbear from phone."];
			return;
		}

		if (!m_phoneInteraction->removePath("/System/Library/LaunchDaemons/au.asn.ucc.matt.dropbear.plist")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /System/Library/LaunchDaemons/au.asn.ucc.matt.dropbear.plist from phone."];
			return;
		}
		
	}

	if ([self isOpenSSHInstalled]) {
		
		if (!m_phoneInteraction->removePath("/usr/bin/sshd")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /usr/bin/sshd from phone."];
			return;
		}
		
		if (!m_phoneInteraction->removePath("/usr/bin/sftp-server")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /usr/bin/sftp-server from phone."];
			return;
		}
		
		if (!m_phoneInteraction->removePath("/etc/ssh/ssh_host_key")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/ssh/ssh_host_key from phone."];
			return;
		}
		
		if (!m_phoneInteraction->removePath("/etc/ssh/ssh_host_key.pub")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/ssh/ssh_host_key.pub from phone."];
			return;
		}
		
		if (!m_phoneInteraction->removePath("/etc/ssh/ssh_host_rsa_key")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/ssh/ssh_host_rsa_key from phone."];
			return;
		}
		
		if (!m_phoneInteraction->removePath("/etc/ssh/ssh_host_rsa_key.pub")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/ssh/ssh_host_rsa_key.pub from phone."];
			return;
		}
		
		if (!m_phoneInteraction->removePath("/etc/ssh/ssh_host_dsa_key")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/ssh/ssh_host_dsa_key from phone."];
			return;
		}
		
		if (!m_phoneInteraction->removePath("/etc/ssh/ssh_host_dsa_key.pub")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/ssh/ssh_host_dsa_key.pub from phone."];
			return;
		}
		
		if (!m_phoneInteraction->removePath("/etc/ssh")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /etc/ssh from phone."];
			return;
		}
		
		if (!m_phoneInteraction->removePath("/System/Library/LaunchDaemons/org.thebends.openssh.plist")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /System/Library/LaunchDaemons/org.thebends.openssh.plist from phone."];
			return;
		}
		
	}

	[installSSHButton setEnabled:YES];
	[removeSSHButton setEnabled:NO];

	if (!m_phoneInteraction->removePath("/usr/bin/scp")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /usr/bin/scp from phone."];
		return;
	}

	if (bRemoveLibarmfp) {

		if (!m_phoneInteraction->removePath("/usr/lib/libarmfp.dylib")) {
			[mainWindow displayAlert:@"Error" message:@"Error removing /usr/lib/libarmfp.dylib from phone."];
			return;
		}

	}

	if (!m_phoneInteraction->removePath("/bin/chmod")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /bin/chmod from phone."];
		return;
	}

	if (!m_phoneInteraction->removePath("/bin/sh")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /bin/sh from phone."];
		return;
	}

	[mainWindow displayAlert:@"Success" message:@"Successfully removed SSH, SFTP, and SCP from your phone.\n\nNote that SSH will continue to run on the phone until you reboot it."];
}

- (BOOL)validateMenuItem:(NSMenuItem*)menuItem
{
	
	switch ([menuItem tag]) {
		case MENU_ITEM_ACTIVATE:

			if (![self isConnected] || [self isActivated]) {
				return NO;
			}

			break;
		case MENU_ITEM_DEACTIVATE:
			
			if (![self isConnected] || ![self isActivated]) {
				return NO;
			}

			break;
		case MENU_ITEM_ENTER_DFU_MODE:

			if (![self isConnected]) {
				return NO;
			}

			break;
		case MENU_ITEM_JAILBREAK:
			
			if (![self isConnected] || [self isJailbroken]) {
				return NO;
			}
			
			break;
		case MENU_ITEM_INSTALL_SSH:

			if (![self isConnected] || ![self isJailbroken] || [self isSSHInstalled]) {
				return NO;
			}

			break;
		case MENU_ITEM_REMOVE_SSH:

			if (![self isConnected] || ![self isJailbroken] || ![self isSSHInstalled]) {
				return NO;
			}
			
			break;
		case MENU_ITEM_RETURN_TO_JAIL:
			
			if (![self isConnected] || ![self isJailbroken]) {
				return NO;
			}
			
			break;
		case MENU_ITEM_CHANGE_PASSWORD:
			
			if (![self isConnected] || ![self isJailbroken]) {
				return NO;
			}

			break;
		case MENU_ITEM_INSTALL_SIM_UNLOCK:

			if (![self isConnected] || ![self isJailbroken] || ![self isSSHInstalled] ||
				[self isanySIMInstalled]) {
				return NO;
			}

			break;
		case MENU_ITEM_REMOVE_SIM_UNLOCK:

			if (![self isConnected] || ![self isJailbroken] || ![self isSSHInstalled] ||
				![self isanySIMInstalled]) {
				return NO;
			}
			
			break;
		case MENU_ITEM_PRE_FIRMWARE_UPGRADE:

			if (![self isConnected] || ![self isJailbroken] || ![self isSSHInstalled] ||
				m_phoneInteraction->fileExists(PRE_FIRMWARE_UPGRADE_FILE)) {
				return NO;
			}

			break;
		default:
			break;
	}
	
	return YES;
}

- (void)updateInfo
{

	if (![self isConnected]) {
		[iTunesVersionField setStringValue:@"-"];
		[productVersionField setStringValue:@"-"];
		[basebandVersionField setStringValue:@"-"];
		[firmwareVersionField setStringValue:@"-"];
		[buildVersionField setStringValue:@"-"];
		[serialNumberField setStringValue:@"-"];
		[activationStateField setStringValue:@"-"];
		[jailbrokenField setStringValue:@"-"];
		[sshInstalledField setStringValue:@"-"];
		return;
	}

	PIVersion iTunesVersion = m_phoneInteraction->getiTunesVersion();
	
	[iTunesVersionField setStringValue:[NSString stringWithFormat:@"%d.%d.%d", iTunesVersion.major, iTunesVersion.minor, iTunesVersion.point]];
	[productVersionField setStringValue:[NSString stringWithCString:m_phoneInteraction->getPhoneProductVersion() encoding:NSUTF8StringEncoding]];
	[basebandVersionField setStringValue:[NSString stringWithCString:m_phoneInteraction->getPhoneBasebandVersion() encoding:NSUTF8StringEncoding]];
	[firmwareVersionField setStringValue:[NSString stringWithCString:m_phoneInteraction->getPhoneFirmwareVersion() encoding:NSUTF8StringEncoding]];
	[buildVersionField setStringValue:[NSString stringWithCString:m_phoneInteraction->getPhoneBuildVersion() encoding:NSUTF8StringEncoding]];
	[serialNumberField setStringValue:[NSString stringWithCString:m_phoneInteraction->getPhoneSerialNumber() encoding:NSUTF8StringEncoding]];
	[activationStateField setStringValue:[NSString stringWithCString:m_phoneInteraction->getPhoneActivationState() encoding:NSUTF8StringEncoding]];

	if ([self isJailbroken]) {
		[jailbrokenField setStringValue:@"Yes"];
	}
	else {
		[jailbrokenField setStringValue:@"No"];
	}

	if ([self isSSHInstalled]) {
		[sshInstalledField setStringValue:@"Yes"];
	}
	else {
		[sshInstalledField setStringValue:@"No"];
	}

}

@end
