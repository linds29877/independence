/*
 *  MainWindow.mm
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
#import "AppController.h"


MainWindow *g_mainWindow;


@implementation MainWindow

- (void)awakeFromNib
{
	[NSApp setDelegate:self];
	[self setDelegate:self];
	g_mainWindow = self;
	m_statusSpinning = false;
	m_modal = false;
}

- (void)displayAlert:(NSString*)title message:(NSString*)msg
{
	NSRunAlertPanel(title, msg, @"OK", nil, nil, nil);
}

- (void)startDisplayWaitingSheet:(NSString*)title message:(NSString*)msg image:(NSImage*)img
					cancelButton:(bool)cancel runModal:(bool)modal
{

	if (title != nil) {
		[waitDialog setTitle:title];
	}

	if (msg != nil) {
		[waitDialogMessage setStringValue:msg];
	}

	NSRect msgFrame = [waitDialogMessage frame];
	NSRect wndFrame = [waitDialog frame];

	if (img != nil) {
		[waitDialogImage setImage:img];
		[waitDialog setFrame:NSMakeRect(wndFrame.origin.x, wndFrame.origin.y, wndFrame.size.width, 286) display:NO];
		[waitDialogMessage setFrame:NSMakeRect(msgFrame.origin.x, 231, msgFrame.size.width, msgFrame.size.height)];
	}
	else {
		[waitDialog setFrame:NSMakeRect(wndFrame.origin.x, wndFrame.origin.y, wndFrame.size.width, 150) display:NO];
		[waitDialogMessage setFrame:NSMakeRect(msgFrame.origin.x, 81, msgFrame.size.width, msgFrame.size.height)];
	}

	[waitDialog update];

	if (cancel) {
		[waitDialogCancel setHidden:NO];
	}
	else {
		[waitDialogCancel setHidden:YES];
	}

	[waitDialogSpinner startAnimation:self];

	[NSApp beginSheet:waitDialog modalForWindow:self modalDelegate:nil
	   didEndSelector:nil contextInfo:nil];

	m_modal = modal;

	if (modal) {
		[NSApp runModalForWindow:waitDialog];
	}

}

- (void)endDisplayWaitingSheet
{
	[waitDialogSpinner stopAnimation:self];
	[NSApp endSheet:waitDialog];
	[waitDialog orderOut:self];

	if (m_modal) {
		[NSApp stopModal];
	}

}

- (void)setStatus:(NSString*)text spinning:(bool)spin
{
	[statusBar setStringValue:text];

	if (m_statusSpinning != spin) {
		m_statusSpinning = spin;

		NSRect contentFrame = [[self contentView] frame];

		if (spin) {
			[statusSpinner startAnimation:self];
			[statusLabel setFrameOrigin:NSMakePoint(41, 2)];
			[statusBar setFrame:NSMakeRect(86, 2, contentFrame.size.width - 86 - 17,
										   14)];
		}
		else {
			[statusSpinner stopAnimation:self];
			[statusLabel setFrameOrigin:NSMakePoint(17, 2)];
			[statusBar setFrame:NSMakeRect(62, 2, contentFrame.size.width - 62 - 17,
										   14)];
		}

		NSRect redrawRect = NSUnionRect([statusSpinner frame], [statusLabel frame]);
		redrawRect = NSUnionRect(redrawRect, [statusBar frame]);
		[[self contentView] setNeedsDisplayInRect:redrawRect];
	}

}

- (void)updateStatus
{
	NSString *statusStr = [NSString string];
	bool waiting = false;

	if ([appController isInRestoreMode]) {
		statusStr = [statusStr stringByAppendingString:@"Connected to iPhone in restore mode"];
	}
	else if ([appController isInRecoveryMode]) {
		statusStr = [statusStr stringByAppendingString:@"Connected to iPhone in recovery mode"];
	}
	else if ([appController isConnected]) {
		statusStr = [statusStr stringByAppendingString:@"Connected to iPhone"];

		if ([appController isActivated]) {
			statusStr = [statusStr stringByAppendingString:@" -- activated"];
		}
		else {
			statusStr = [statusStr stringByAppendingString:@" -- not activated"];
		}

		if ([appController isJailbroken]) {
			statusStr = [statusStr stringByAppendingString:@" -- jailbroken"];
		}

	}
	else {
		statusStr = [statusStr stringByAppendingString:@"Disconnected: waiting for iPhone..."];
		waiting = true;
	}

	[self setStatus:statusStr spinning:waiting];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApp
{
	return YES;
}

- (void)windowDidResize:(NSNotification*)note
{
}

- (IBAction)showAboutPanel:(id)sender
{
	[aboutBox makeKeyAndOrderFront:sender];
}

@end
