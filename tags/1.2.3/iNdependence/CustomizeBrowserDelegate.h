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
#import "SSHHandler.h"


@interface CustomizeBrowserDelegate : NSObject
{
	IBOutlet MainWindow *m_mainWindow;
	IBOutlet CustomizeBrowser *m_browser;
	IBOutlet NSButton *m_deleteButton;
	IBOutlet NSButton *m_addButton;
	IBOutlet SSHHandler *m_sshHandler;
	NSArray *m_col1Items;
	NSDictionary *m_col2Dictionary;
}

- (bool)acceptDraggedFiles:(NSArray*)files;
- (bool)addFilesToPhone:(NSArray*)files wasCancelled:(bool*)bCancelled;

- (char*)getRingtoneFileExtension;

- (IBAction)selectionChanged:(id)sender;
- (IBAction)deleteButtonPressed:(id)sender;
- (IBAction)addButtonPressed:(id)sender;

@end
