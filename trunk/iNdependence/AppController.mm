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
#include "PhoneInteraction/UtilityFunctions.h"
#include "PhoneInteraction/PhoneInteraction.h"


enum
{
	MENU_ITEM_GENERATE = 10,
	MENU_ITEM_PUT_PEM = 11,
	MENU_ITEM_ACTIVATE = 12,
	MENU_ITEM_DEACTIVATE = 13,
	MENU_ITEM_RETURN_TO_JAIL = 14,
	MENU_ITEM_JAILBREAK = 15,
	MENU_ITEM_BACKUP_ACTIVATION = 16,
	MENU_ITEM_INSTALL_SSH = 17,
	MENU_ITEM_CHANGE_PASSWORD = 18,
	MENU_ITEM_REMOVE_SSH = 21,
	MENU_ITEM_RESTORE_PEM = 22
};

extern MainWindow *g_mainWindow;
static AppController *g_appController;

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
				break;
			case NOTIFY_DISCONNECTED:
				[g_appController setConnected:false];
				break;
			case NOTIFY_INITIALIZATION_FAILED:
			case NOTIFY_CONNECTION_FAILED:
				[g_mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_ACTIVATION_SUCCESS:
				[g_appController setActivated:true];
				[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_DEACTIVATION_SUCCESS:
				[g_appController setActivated:false];
				[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_JAILBREAK_SUCCESS:
				[g_mainWindow endDisplayWaitingSheet];
				[g_appController setJailbroken:true];
				[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_ACTIVATION_FAILED:
			case NOTIFY_DEACTIVATION_FAILED:
			case NOTIFY_PUTPEM_FAILED:
			case NOTIFY_PUTSERVICES_FAILED:
			case NOTIFY_PUTFSTAB_FAILED:
			case NOTIFY_JAILBREAK_FAILED:
			case NOTIFY_JAILBREAK_FAIL_USER_COULDNT_HOLD:
				[g_mainWindow endDisplayWaitingSheet];
			case NOTIFY_GET_ACTIVATION_FAILED:
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_PUTPEM_SUCCESS:
				[g_mainWindow endDisplayWaitingSheet];
			case NOTIFY_GET_ACTIVATION_SUCCESS:
				[g_mainWindow updateStatus];
				[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_JAILBREAK_RECOVERY_WAIT:
				[g_mainWindow startDisplayWaitingSheet:nil message:@"Please press and hold Home + Sleep on your iPhone until this dialog window disappears..." image:[NSImage imageNamed:@"home_sleep_buttons"] cancelButton:true runModal:true];
				break;
			case NOTIFY_JAILBREAK_RECOVERY_CONNECTED:
				[g_mainWindow endDisplayWaitingSheet];
				[g_mainWindow startDisplayWaitingSheet:nil message:@"Waiting for jail break..." image:[NSImage imageNamed:@"jailbreak"] cancelButton:false runModal:false];
				[g_appController setRecoveryMode:true];
				break;
			case NOTIFY_JAILBREAK_RECOVERY_DISCONNECTED:
				[g_appController setRecoveryMode:false];
				break;
			case NOTIFY_JAILBREAK_RESTORE_CONNECTED:
				[g_appController setRestoreMode:true];
				break;
			case NOTIFY_JAILBREAK_RESTORE_DISCONNECTED:
				[g_appController setRestoreMode:false];
				break;
			case NOTIFY_JAILBREAK_CANCEL:
				[g_mainWindow endDisplayWaitingSheet];
				[g_mainWindow updateStatus];
				break;
			case NOTIFY_CONNECTION_SUCCESS:
			case NOTIFY_INITIALIZATION_SUCCESS:
			case NOTIFY_PUTFSTAB_SUCCESS:
			case NOTIFY_PUTSERVICES_SUCCESS:
			default:
				break;
		}
		
	}
	
}

static void utilityFunctionNotification(int type, const char *msg)
{
	
	if (g_mainWindow) {
		
		switch (type) {
			case NOTIFY_ACTIVATION_GEN_FAILED:
				[g_mainWindow displayAlert:@"Failure" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_ACTIVATION_GEN_SUCCESS:
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

	m_connected = false;
	m_recoveryMode = false;
	m_restoreMode = false;
	m_jailbroken = false;
	m_activated = false;
	m_performingJailbreak = false;
	m_returningToJail = false;
	m_installingSSH = false;
	m_bootCount = 0;
	m_sshPath = NULL;
	[customizeBrowser setEnabled:NO];
	m_phoneInteraction = PhoneInteraction::getInstance(updateStatus, phoneInteractionNotification);
}

- (void)setConnected:(bool)connected
{
	m_connected = connected;
	
	if (m_connected) {
		[self setActivated:m_phoneInteraction->isPhoneActivated()];
		[self setJailbroken:m_phoneInteraction->isPhoneJailbroken()];
		
		if (m_returningToJail) {
			m_returningToJail = false;
			[mainWindow endDisplayWaitingSheet];
			[mainWindow displayAlert:@"Success" message:@"Successfully returned to jail."];
		}

		if (m_installingSSH) {
			m_bootCount++;
			[mainWindow endDisplayWaitingSheet];

			if (m_bootCount < 2) {
				[mainWindow startDisplayWaitingSheet:nil
											 message:@"Please reboot your phone again using the same steps..."
											   image:[NSImage imageNamed:@"home_sleep_buttons"] cancelButton:true runModal:false];
			}
			else {
				[self finishInstallingSSH:false];
			}

		}
		
	}
	else {
		[self setActivated:false];
		[self setJailbroken:false];
	}
	
	[mainWindow updateStatus];
}

- (bool)isConnected
{
	return m_connected;
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

- (void)setJailbroken:(bool)jailbroken
{
	m_jailbroken = jailbroken;
	
	if (m_jailbroken) {
		[putPEMButton setEnabled:YES];
		[restorePEMButton setEnabled:YES];
		[returnToJailButton setEnabled:YES];
		[changePasswordButton setEnabled:YES];
		[customizeBrowser setEnabled:YES];
		[jailbreakButton setEnabled:NO];
		
		if ([self isActivated]) {
			[backupButton setEnabled:YES];
		}
		else {
			[backupButton setEnabled:NO];
		}

		if ([self isSSHInstalled]) {
			[installSSHButton setEnabled:NO];
			[removeSSHButton setEnabled:YES];
		}
		else {
			[installSSHButton setEnabled:YES];
			[removeSSHButton setEnabled:NO];
		}

	}
	else {
		[putPEMButton setEnabled:NO];
		[restorePEMButton setEnabled:NO];
		[returnToJailButton setEnabled:NO];
		[installSSHButton setEnabled:NO];
		[removeSSHButton setEnabled:NO];
		[changePasswordButton setEnabled:NO];
		[customizeBrowser setEnabled:NO];
		[backupButton setEnabled:NO];
		
		if ([self isConnected]) {
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
		[activateButton setEnabled:NO];
		[deactivateButton setEnabled:YES];
		
		if ([self isJailbroken]) {
			[backupButton setEnabled:YES];
		}
		else {
			[backupButton setEnabled:NO];
		}
		
	}
	else {
		
		if (![self isConnected]) {
			[activateButton setEnabled:NO];
		}
		else {
			[activateButton setEnabled:YES];
		}
		
		[backupButton setEnabled:NO];
		[deactivateButton setEnabled:NO];
	}
	
	[mainWindow updateStatus];
}

- (bool)isActivated
{
	return m_activated;
}

- (bool)isSSHInstalled
{
	return m_phoneInteraction->fileExists("/usr/bin/dropbear");
}

- (IBAction)generateActivation:(id)sender
{
	[mainWindow setStatus:@"Generating activation..." spinning:true];

	int retval;
	const char *imeiVal;
	const char *iccidVal;
	char deviceid[41];

	// get the IMEI and ICCID
	[NSApp beginSheet:imeiDialog modalForWindow:mainWindow modalDelegate:nil
	   didEndSelector:nil contextInfo:nil];

	while (1) {
		retval = [NSApp runModalForWindow:imeiDialog];

		if (retval == -1) {
			[NSApp endSheet:imeiDialog];
			[imeiDialog orderOut:self];
			[mainWindow updateStatus];
			return;
		}

		imeiVal = [[imeiTextField stringValue] UTF8String];

		if (!UtilityFunctions::validateIMEI(imeiVal)) {
			[mainWindow displayAlert:@"Error" message:@"You entered an invalid IMEI value.  Try again."];
			continue;
		}

		iccidVal = [[iccidTextField stringValue] UTF8String];
	
		if (!UtilityFunctions::validateICCID(iccidVal)) {
			[mainWindow displayAlert:@"Error" message:@"You entered an invalid ICCID value.  Try again."];
			continue;
		}

		if (!UtilityFunctions::findDeviceID(deviceid)) {
			[mainWindow displayAlert:@"Error" message:@"Can't find device ID.  Check to ensure that ~/Library/Lockdown/ exists and contains a .plist file."];
			[NSApp endSheet:imeiDialog];
			[imeiDialog orderOut:self];
			[mainWindow updateStatus];
			return;
		}

		break;
	}

	[NSApp endSheet:imeiDialog];
	[imeiDialog orderOut:self];

	// now find the PEM file
	NSString *pemfile = [[NSBundle mainBundle] pathForResource:@"iPhoneActivation_private" ofType:@"pem"];
	
	if (pemfile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding PEM file to sign activation with."];
		[mainWindow updateStatus];
		return;
	}

	CFDictionaryRef activationrequest;

	if (!UtilityFunctions::generateActivationRequest_All(&activationrequest, [pemfile UTF8String],
														 deviceid, imeiVal, iccidVal,
														 utilityFunctionNotification)) {
		[mainWindow displayAlert:@"Error" message:@"Error generating the activation file."];
		[mainWindow updateStatus];
		return;
	}

	NSSavePanel *plistSaver = [NSSavePanel savePanel];
	[plistSaver setTitle:@"Choose a Location for the activation file"];
	[plistSaver setRequiredFileType:@"plist"];

	retval = [plistSaver runModal];

	if (retval == NSFileHandlingPanelOKButton) {
		[(NSDictionary*)activationrequest writeToFile:[plistSaver filename] atomically:YES];
	}

	CFRelease(activationrequest);
	[mainWindow updateStatus];
}

- (IBAction)imeiDialogCancel:(id)sender
{
	[NSApp stopModalWithCode:-1];
}

- (IBAction)imeiDialogOk:(id)sender
{
	[NSApp stopModalWithCode:0];
}

- (IBAction)putPEMOnPhone:(id)sender
{
	[mainWindow setStatus:@"Putting PEM file on phone..." spinning:true];

	NSString *pemfile = [[NSBundle mainBundle] pathForResource:@"iPhoneActivation" ofType:@"pem"];

	if (pemfile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding PEM file in application bundle."];
		[mainWindow updateStatus];
		return;
	}

	m_phoneInteraction->putPEMOnPhone([pemfile UTF8String]);
}

- (IBAction)restorePEM:(id)sender
{
	[mainWindow setStatus:@"Restoring original PEM file on phone..." spinning:true];

	NSString *pemfile = [[NSBundle mainBundle] pathForResource:@"iPhoneActivation_original" ofType:@"pem"];

	if (pemfile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding PEM file in application bundle."];
		[mainWindow updateStatus];
		return;
	}
	
	m_phoneInteraction->putPEMOnPhone([pemfile UTF8String]);
}

- (IBAction)returnToJail:(id)sender
{
	[mainWindow setStatus:@"Returning to jail..." spinning:true];

	NSString *servicesFile = [[NSBundle mainBundle] pathForResource:@"Services" ofType:@"plist"];

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
	
	if (!m_phoneInteraction->putServicesFileOnPhone([servicesFile UTF8String])) {
		[mainWindow displayAlert:@"Error" message:@"Error writing Services.plist file to phone."];
		[mainWindow updateStatus];
		return;
	}

	if (!m_phoneInteraction->putFstabFileOnPhone([fstabFile UTF8String])) {
		[mainWindow displayAlert:@"Error" message:@"Error writing fstab file to phone."];
		[mainWindow updateStatus];
		return;
	}

	m_returningToJail = true;

	[mainWindow startDisplayWaitingSheet:nil
						   message:@"Please press and hold the Home + Sleep buttons for 3 seconds, then power off your phone, then press Sleep again to restart it..."
							 image:[NSImage imageNamed:@"home_sleep_buttons"] cancelButton:true runModal:true];
}

- (IBAction)activate:(id)sender
{
	NSOpenPanel *plistOpener = [NSOpenPanel openPanel];
	[plistOpener setTitle:@"Select an Activation plist File"];
	[plistOpener setCanChooseDirectories:NO];
	[plistOpener setAllowsMultipleSelection:NO];

	NSArray *fileTypes = [NSArray arrayWithObject:@"plist"];
	int retval = [plistOpener runModalForTypes:fileTypes];
	
	if (retval != NSOKButton) {
		return;
	}
	
	m_phoneInteraction->activate([[plistOpener filename] UTF8String]);
}

- (IBAction)deactivate:(id)sender
{
	m_phoneInteraction->deactivate();
}

- (IBAction)performJailbreak:(id)sender
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

- (IBAction)backupActivation:(id)sender
{
	NSSavePanel *plistSaver = [NSSavePanel savePanel];
	[plistSaver setTitle:@"Choose a Location for the activation file"];
	[plistSaver setRequiredFileType:@"plist"];

	if ([plistSaver runModal] == NSFileHandlingPanelOKButton) {
		m_phoneInteraction->getActivationFile([[plistSaver filename] UTF8String]);
	}
	
}

- (IBAction)waitDialogCancel:(id)sender
{
	
	if (m_performingJailbreak) {
		m_performingJailbreak = false;
		m_phoneInteraction->cancelJailbreak();
	}
	else if (m_returningToJail) {
		m_returningToJail = false;
	}
	else if (m_installingSSH) {
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

- (IBAction)keyGenerationOutputDismiss:(id)sender
{
	[keyGenerationOutput orderOut:self];
}

- (IBAction)installSSH:(id)sender
{

	// first generate the dropbear RSA and DSS keys
	NSString *dropbearkeyPath = [[NSBundle mainBundle] pathForResource:@"dropbearkey" ofType:@""];

	if (dropbearkeyPath == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding dropbearkey in bundle."];
		return;
	}

	[logOutput setString:@""];
	[keyGenerationOutput orderFront:self];

	NSString *tmpDir = NSTemporaryDirectory();
	NSMutableString *dropbearRSAFile = [NSMutableString stringWithString:tmpDir];
	[dropbearRSAFile appendString:@"/dropbear_rsa_host_key"];

	// remove old file if it exists
	remove([dropbearRSAFile UTF8String]);

	NSArray *args = [NSArray arrayWithObjects:@"-t", @"rsa", @"-f", dropbearRSAFile, nil];
	NSTask *task = [[NSTask alloc] init];
	NSPipe *pipe = [NSPipe pipe];
	NSFileHandle *readHandle = [pipe fileHandleForReading];
	NSData *inData = nil;

	[task setStandardOutput:pipe];
	[task setStandardError:pipe];
	[task setLaunchPath:dropbearkeyPath];
	[task setArguments:args];
	[task launch];

	NSTextStorage *textStore = [logOutput textStorage];
	
	// output to log window
	while ((inData = [readHandle availableData]) && [inData length]) {
		int len = [inData length];
		char buf[len+1];
		memcpy(buf, [inData bytes], len);

		if (buf[len-1] != 0) {
			buf[len] = 0;
		}

		NSAttributedString *tmpString = [[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:buf]];
		[textStore appendAttributedString:tmpString];
		[logOutput scrollRangeToVisible:NSMakeRange([textStore length]-2, 1)];
		[tmpString release];
	}

	[task waitUntilExit];

	if ([task terminationStatus] != 0) {
		[task release];
		[mainWindow displayAlert:@"Error" message:@"Error occurred while executing dropbearkey."];
		return;
	}

	[task release];

	NSMutableString *dropbearDSSFile = [NSMutableString stringWithString:tmpDir];
	[dropbearDSSFile appendString:@"/dropbear_dss_host_key"];

	// remove old file if it exists
	remove([dropbearDSSFile UTF8String]);

	args = [NSArray arrayWithObjects:@"-t", @"dss", @"-f", dropbearDSSFile, nil];
	task = [[NSTask alloc] init];
	pipe = [NSPipe pipe];
	readHandle = [pipe fileHandleForReading];

	[task setStandardOutput:pipe];
	[task setStandardError:pipe];
	[task setLaunchPath:dropbearkeyPath];
	[task setArguments:args];
	[task launch];

	// output to log window
	while ((inData = [readHandle availableData]) && [inData length]) {
		int len = [inData length];
		char buf[len+1];
		memcpy(buf, [inData bytes], len);
		
		if (buf[len-1] != 0) {
			buf[len] = 0;
		}
		
		NSAttributedString *tmpString = [[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:buf]];
		[textStore appendAttributedString:tmpString];
		[logOutput scrollRangeToVisible:NSMakeRange([textStore length]-2, 1)];
		[tmpString release];
	}

	[task waitUntilExit];

	if ([task terminationStatus] != 0) {
		[task release];
		[mainWindow displayAlert:@"Error" message:@"Error occurred while executing dropbearkey."];
		return;
	}

	[task release];

	if (!m_phoneInteraction->createDirectory("/etc/dropbear")) {
		[mainWindow displayAlert:@"Error" message:@"Error creating /etc/dropbear directory on phone."];
		return;
	}

	if (!m_phoneInteraction->putFile([dropbearRSAFile UTF8String], "/etc/dropbear/dropbear_rsa_host_key")) {
		[mainWindow displayAlert:@"Error" message:@"Error writing /etc/dropbear/dropbear_rsa_host_key to phone."];
		return;
	}

	if (!m_phoneInteraction->putFile([dropbearDSSFile UTF8String], "/etc/dropbear/dropbear_dss_host_key")) {
		[mainWindow displayAlert:@"Error" message:@"Error writing /etc/dropbear/dropbear_dss_host_key to phone."];
		return;
	}

	NSString *chmodFile = [[NSBundle mainBundle] pathForResource:@"chmod" ofType:@""];

	if (chmodFile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding chmod in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([chmodFile UTF8String], "/bin/chmod")) {
		[mainWindow displayAlert:@"Error" message:@"Error writing /bin/chmod to phone."];
		return;
	}

	NSString *shFile = [[NSBundle mainBundle] pathForResource:@"sh" ofType:@""];

	if (shFile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding sh in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([shFile UTF8String], "/bin/sh")) {
		[mainWindow displayAlert:@"Error" message:@"Error writing /bin/sh to phone."];
		return;
	}

	NSString *dropbearFile = [[NSBundle mainBundle] pathForResource:@"dropbear" ofType:@""];
	
	if (dropbearFile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding dropbear in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([dropbearFile UTF8String], "/usr/bin/dropbear")) {
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/bin/dropbear to phone."];
		return;
	}

	NSString *sftpFile = [[NSBundle mainBundle] pathForResource:@"sftp-server" ofType:@""];

	if (sftpFile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding sftp-server in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([sftpFile UTF8String], "/usr/libexec/sftp-server")) {
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/libexec/sftp-server to phone."];
		return;
	}

	NSString *scpFile = [[NSBundle mainBundle] pathForResource:@"scp" ofType:@""];
	
	if (scpFile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding scp in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([scpFile UTF8String], "/usr/bin/scp")) {
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/bin/scp to phone."];
		return;
	}
	
	NSString *libarmfpFile = [[NSBundle mainBundle] pathForResource:@"libarmfp" ofType:@"dylib"];
	
	if (libarmfpFile == nil) {
		[mainWindow displayAlert:@"Error" message:@"Error finding libarmfp.dylib in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([libarmfpFile UTF8String], "/usr/lib/libarmfp.dylib")) {
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/lib/libarmfp.dylib to phone."];
		return;
	}
	
	NSMutableString *tmpFilePath = [NSMutableString stringWithString:tmpDir];
	[tmpFilePath appendString:@"/update.backup.iNdependence"];

	if (!m_phoneInteraction->getFile("/usr/sbin/update", [tmpFilePath UTF8String])) {
		[mainWindow displayAlert:@"Error" message:@"Error reading /usr/sbin/update from phone."];
		return;
	}

	if (!m_phoneInteraction->putFile([chmodFile UTF8String], "/usr/sbin/update")) {
		remove([tmpFilePath UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error writing /usr/sbin/update to phone."];
		return;
	}

	NSMutableString *tmpFilePath2 = [NSMutableString stringWithString:tmpDir];
	[tmpFilePath2 appendString:@"/com.apple.update.plist.backup.iNdependence"];

	if (!m_phoneInteraction->getFile("/System/Library/LaunchDaemons/com.apple.update.plist", [tmpFilePath2 UTF8String])) {
		remove([tmpFilePath UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error reading /System/Library/LaunchDaemons/com.apple.update.plist from phone."];
		return;
	}

	int fd = open([tmpFilePath2 UTF8String], O_RDONLY, 0);

	if (fd == -1) {
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error opening com.apple.update.plist.backup.iNdependence for reading."];
		return;
	}

	struct stat st;

	if (fstat(fd, &st) == -1) {
		close(fd);
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error obtaining com.apple.update.plist.original file size."];
		return;
	}

	NSMutableString *tmpFilePath3 = [NSMutableString stringWithString:tmpDir];
	[tmpFilePath3 appendString:@"/com.apple.update.plist.iNdependence"];
	int fd2 = open([tmpFilePath3 UTF8String], O_CREAT | O_TRUNC | O_WRONLY,
				   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (fd2 == -1) {
		close(fd);
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
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
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
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
	[mutArgs addObject:@"/usr/bin/dropbear"];
	[mutArgs addObject:@"/usr/libexec/sftp-server"];
	[mutArgs addObject:@"/usr/bin/scp"];
	[mutDict setObject:mutArgs forKey:@"ProgramArguments"];

	if (remove([tmpFilePath3 UTF8String]) == -1) {
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error deleting com.apple.update.plist.iNdependence"];
		return;
	}

	if (![mutDict writeToFile:tmpFilePath3 atomically:YES]) {
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error creating new com.apple.update.plist."];
		return;
	}

	if (!m_phoneInteraction->putFile([tmpFilePath3 UTF8String], "/System/Library/LaunchDaemons/com.apple.update.plist")) {
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error writing /System/Library/LaunchDaemons/com.apple.update.plist to phone."];
		return;
	}

	NSString *dropbearPlistFile = [[NSBundle mainBundle] pathForResource:@"au.asn.ucc.matt.dropbear" ofType:@"plist"];

	if (dropbearPlistFile == nil) {
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error finding au.asn.ucc.matt.dropbear.plist in bundle."];
		return;
	}
	
	if (!m_phoneInteraction->putFile([dropbearPlistFile UTF8String], "/System/Library/LaunchDaemons/au.asn.ucc.matt.dropbear.plist")) {
		remove([tmpFilePath UTF8String]);
		remove([tmpFilePath2 UTF8String]);
		remove([tmpFilePath3 UTF8String]);
		[mainWindow displayAlert:@"Error" message:@"Error writing /System/Library/LaunchDaemons/au.asn.ucc.matt.dropbear.plist to phone."];
		return;
	}

	m_installingSSH = true;
	m_bootCount = 0;

	[mainWindow startDisplayWaitingSheet:nil
								 message:@"Please press and hold the Home + Sleep buttons for 3 seconds, then power off your phone, then press Sleep again to restart it."
								   image:[NSImage imageNamed:@"home_sleep_buttons"] cancelButton:true runModal:false];
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
		[mainWindow displayAlert:@"Success" message:@"Successfully installed Dropbear SSH, SFTP, and SCP on your phone."];
	}

}

- (IBAction)removeSSH:(id)sender
{

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

	if (!m_phoneInteraction->removePath("/bin/chmod")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /bin/chmod from phone."];
		return;
	}
	
	if (!m_phoneInteraction->removePath("/bin/sh")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /bin/sh from phone."];
		return;
	}
	
	if (!m_phoneInteraction->removePath("/usr/bin/dropbear")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /usr/bin/dropbear from phone."];
		return;
	}
	
	if (!m_phoneInteraction->removePath("/usr/libexec/sftp-server")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /usr/libexec/sftp-server from phone."];
		return;
	}
	
	if (!m_phoneInteraction->removePath("/usr/bin/scp")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /usr/bin/scp from phone."];
		return;
	}
	
	if (!m_phoneInteraction->removePath("/usr/lib/libarmfp.dylib")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /usr/lib/libarmfp.dylib from phone."];
		return;
	}
	
	if (!m_phoneInteraction->removePath("/System/Library/LaunchDaemons/au.asn.ucc.matt.dropbear.plist")) {
		[mainWindow displayAlert:@"Error" message:@"Error removing /System/Library/LaunchDaemons/au.asn.ucc.matt.dropbear.plist from phone."];
		return;
	}
	
	[mainWindow displayAlert:@"Success" message:@"Successfully removed Dropbear SSH, SFTP, and SCP from your phone."];
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
		case MENU_ITEM_PUT_PEM:
		case MENU_ITEM_RESTORE_PEM:
		case MENU_ITEM_BACKUP_ACTIVATION:
		case MENU_ITEM_CHANGE_PASSWORD:
			
			if (![self isConnected] || ![self isJailbroken]) {
				return NO;
			}

			break;
		case MENU_ITEM_GENERATE:
		default:
			break;
	}
	
	return YES;
}

@end
