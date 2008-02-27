/*
 *  SSHHandler.h
 *  iNdependence
 *
 *  Created by The Operator on 21/09/07.
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

#import "MainWindow.h"


@interface SSHHandler : NSObject
{
	IBOutlet MainWindow *m_mainWindow;
	IBOutlet NSWindow *m_sshWindow;
	IBOutlet NSTextField *m_sshIPText1;
	IBOutlet NSTextField *m_sshIPText2;
	IBOutlet NSTextField *m_sshIPText3;
	IBOutlet NSTextField *m_sshIPText4;
	IBOutlet NSSecureTextField *m_sshPassword;	
}

- (bool)removeKnownHostsEntry:(NSString*)ipAddress;
- (bool)getSSHInfo:(NSString**)ipAddress password:(NSString**)password wasCancelled:(bool*)bCancelled;
- (IBAction)sshDialogCancel:(id)sender;
- (IBAction)sshDialogOk:(id)sender;

@end
