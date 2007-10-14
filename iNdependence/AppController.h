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
	IBOutlet MainWindow *mainWindow;
	IBOutlet NSWindow* newPasswordDialog;
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
	IBOutlet NSButton* installSimUnlockButton;
	IBOutlet NSButton* removeSimUnlockButton;
	IBOutlet NSButton* enterDFUModeButton;
	IBOutlet NSButton* pre111UpgradeButton;
	IBOutlet NSButton* post111UpgradeButton;
	IBOutlet CustomizeBrowser* customizeBrowser;
	IBOutlet NSWindow* keyGenerationOutput;
	IBOutlet NSTextView* logOutput;
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
	int m_bootCount;
	char *m_sshPath;
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

- (bool)isSSHInstalled;
- (bool)isanySIMInstalled;

- (bool)isWaitingForActivation;
- (bool)isWaitingForDeactivation;

- (bool)isUsing10xFirmware;
- (NSString*)phoneFirmwareVersion;

- (bool)doPutPEM:(const char*)pemfile;
- (void)activateStageTwo;
- (void)activateStageThree;
- (void)activationFailed:(const char*)msg;
- (void)deactivateStageTwo;
- (void)deactivateStageThree;
- (void)deactivationFailed:(const char*)msg;

- (IBAction)activate:(id)sender;
- (IBAction)deactivate:(id)sender;
- (IBAction)enterDFUMode:(id)sender;
- (IBAction)pre111Upgrade:(id)sender;
- (IBAction)post111Upgrade:(id)sender;
- (IBAction)performJailbreak:(id)sender;
- (IBAction)returnToJail:(id)sender;
- (IBAction)installSimUnlock:(id)sender;
- (IBAction)removeSimUnlock:(id)sender;
- (IBAction)changePassword:(id)sender;
- (IBAction)installSSH:(id)sender;
- (IBAction)removeSSH:(id)sender;
- (void)finishInstallingSSH:(bool)bCancelled;

- (IBAction)waitDialogCancel:(id)sender;

- (IBAction)keyGenerationOutputDismiss:(id)sender;

- (IBAction)passwordDialogCancel:(id)sender;
- (IBAction)passwordDialogOk:(id)sender;

@end
