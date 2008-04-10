/*
 *  AppController.h
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

#import "CustomizeBrowser.h"


class PhoneInteraction;
@class SSHHandler;
@class MainWindow;

@interface AppController : NSObject
{
	IBOutlet MainWindow* mainWindow;
	IBOutlet NSWindow* newPasswordDialog;
	IBOutlet NSWindow* sshRemovalDialog;
	IBOutlet NSTextField* accountNameField;
	IBOutlet NSSecureTextField* passwordField;
	IBOutlet NSSecureTextField* passwordAgainField;
	IBOutlet NSButton* jailbreakButton;
	IBOutlet NSButton* returnToJailButton;
	IBOutlet NSButton* activateButton;
	IBOutlet NSButton* deactivateButton;
	IBOutlet NSButton* installSSHButton;
	IBOutlet NSButton* removeSSHButton;
	IBOutlet NSButton* changePasswordButton;
	IBOutlet NSButton* performSimUnlockButton;
	IBOutlet NSButton* enterRecoveryModeButton;
	IBOutlet NSButton* enterDFUModeButton;
	IBOutlet NSButton* preFirmwareUpgradeButton;
	IBOutlet NSButton* libarmfpRemovalButton;
	IBOutlet NSButton* shRemovalButton;
	IBOutlet NSButton* chmodRemovalButton;
	IBOutlet CustomizeBrowser* customizeBrowser;
	IBOutlet NSTextField* iTunesVersionField;
	IBOutlet NSTextField* productVersionField;
	IBOutlet NSTextField* basebandVersionField;
	IBOutlet NSTextField* firmwareVersionField;
	IBOutlet NSTextField* buildVersionField;
	IBOutlet NSTextField* activationStateField;
	IBOutlet NSTextField* jailbrokenField;
	IBOutlet NSTextField* sshInstalledField;
	IBOutlet NSTextField* serialNumberField;
	IBOutlet NSTextField* enterRecoveryModeDescription;
	IBOutlet SSHHandler* sshHandler;

	PhoneInteraction *m_phoneInteraction;
	bool m_connected;
	bool m_afcConnected;
	bool m_recoveryMode;
	bool m_restoreMode;
	bool m_dfuMode;
	bool m_jailbroken;
	bool m_activated;
	bool m_performingJailbreak;
	bool m_returningToJail;
	bool m_installingSSH;
	bool m_waitingForActivation;
	bool m_waitingForDeactivation;
	bool m_waitingForNewActivation;
	bool m_waitingForNewDeactivation;
	bool m_waitingForRecoveryModeSwitch;
	bool m_bIsDownloading;
	bool m_bDownloadSucceeded;
	NSURLResponse *m_downloadResponse;
	int m_bootCount;
	char *m_sshPath;
	NSString *m_ramdiskPath;
	NSString *m_rdFirmwarePath;
	NSString *m_rdDLPath;
	long long m_expectedLength;
	long long m_bytesReceived;
}

- (void)setConnected:(bool)connected;
- (bool)isConnected;

- (void)setAFCConnected:(bool)connected;
- (bool)isAFCConnected;

- (void)setRecoveryMode:(bool)inRecovery;
- (bool)isInRecoveryMode;

- (void)setRestoreMode:(bool)inRestore;
- (bool)isInRestoreMode;

- (void)setDFUMode:(bool)inDFU;
- (bool)isInDFUMode;

- (void)setJailbroken:(bool)jailbroken;
- (bool)isJailbroken;

- (void)setActivated:(bool)activated;
- (bool)isActivated;

- (void)setPerformingJailbreak:(bool)bJailbreaking;
- (void)setReturningToJail:(bool)bReturning;

- (bool)isOpenSSHInstalled;
- (bool)isDropbearSSHInstalled;
- (bool)isSSHInstalled;

- (bool)isWaitingForActivation;
- (bool)isWaitingForNewActivation;
- (bool)isWaitingForDeactivation;

- (NSString*)phoneFirmwareVersion;

- (bool)doPutPEM:(const char*)pemfile;

- (void)activateStageTwo:(bool)displaySheet;
- (void)activationFailed:(const char*)msg;
- (void)deactivateStageTwo;
- (void)deactivationFailed:(const char*)msg;

- (void)updateInfo;

- (NSString*)generateRamdisk;
- (NSString*)getRamdiskPath;
- (bool)validateRamDisk:(NSString*)rdPath;
- (void)setDownloadResponse:(NSURLResponse*)dlResponse;

- (NSString*)pathToAppSupportRamDisk;

- (IBAction)activate:(id)sender;
- (IBAction)deactivate:(id)sender;
- (IBAction)enterRecoveryMode:(id)sender;
- (IBAction)enterDFUMode:(id)sender;
- (IBAction)preFirmwareUpgrade:(id)sender;
- (IBAction)performJailbreak:(id)sender;
- (IBAction)returnToJail:(id)sender;
- (IBAction)performSimUnlock:(id)sender;
- (IBAction)changePassword:(id)sender;

- (IBAction)installSSH:(id)sender;
- (IBAction)removeSSH:(id)sender;
- (void)sshInstallStageTwo;
- (void)sshInstallStageThree;
- (void)finishInstallingSSH:(bool)bCancelled;

- (IBAction)waitDialogCancel:(id)sender;

- (IBAction)passwordDialogCancel:(id)sender;
- (IBAction)passwordDialogOk:(id)sender;

- (IBAction)sshRemovalDialogCancel:(id)sender;
- (IBAction)sshRemovalDialogOk:(id)sender;

@end
