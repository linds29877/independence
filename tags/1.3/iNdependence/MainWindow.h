/*
 *  MainWindow.h
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

@class AppController;

@interface MainWindow : NSWindow
{
	IBOutlet NSTextField *statusLabel;
	IBOutlet NSTextField *statusBar;
	IBOutlet NSProgressIndicator *statusSpinner;
	IBOutlet NSWindow *aboutBox;
	IBOutlet AppController *appController;

	NSWindow *waitDialog;
	NSTextField *waitDialogMessage;
	NSImageView *waitDialogImage;
	NSButton *waitDialogCancel;
	NSProgressIndicator *waitDialogSpinner;

	bool m_statusSpinning;
	bool m_modal;
	NSAlert *m_sheet;
}

- (void)displayAlert:(NSString*)title message:(NSString*)msg;

- (void)setStatus:(NSString*)text spinning:(bool)spin;
- (void)updateStatus;

- (void)startDisplayWaitingSheet:(NSString*)title message:(NSString*)msg image:(NSImage*)img
					cancelButton:(bool)cancel runModal:(bool)modal;
- (void)endDisplayWaitingSheet;

- (IBAction)showAboutPanel:(id)sender;

@end
