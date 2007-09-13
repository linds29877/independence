/*
 *  CustomizeBrowserDelegate.h
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

#import "MainWindow.h"
#import "CustomizeBrowser.h"


@interface CustomizeBrowserDelegate : NSObject
{
	IBOutlet MainWindow *m_mainWindow;
	IBOutlet CustomizeBrowser *m_browser;
	IBOutlet NSButton *m_deleteButton;
	IBOutlet NSButton *m_addButton;
	IBOutlet NSWindow *m_sshWindow;
	IBOutlet NSTextField *m_sshIPText1;
	IBOutlet NSTextField *m_sshIPText2;
	IBOutlet NSTextField *m_sshIPText3;
	IBOutlet NSTextField *m_sshIPText4;
	IBOutlet NSSecureTextField *m_sshPassword;
	NSArray *m_col1Items;
	NSDictionary *m_col2Dictionary;
}

- (bool)acceptDraggedFiles:(NSArray*)files;
- (bool)addFilesToPhone:(NSArray*)files wasCancelled:(bool*)bCancelled;
- (bool)getSSHInfo:(NSString**)ipAddress password:(NSString**)password wasCancelled:(bool*)bCancelled;
- (bool)removeKnownHostsEntry:(NSString*)ipAddress;

- (IBAction)sshDialogCancel:(id)sender;
- (IBAction)sshDialogOk:(id)sender;
- (IBAction)selectionChanged:(id)sender;
- (IBAction)deleteButtonPressed:(id)sender;
- (IBAction)addButtonPressed:(id)sender;

@end
