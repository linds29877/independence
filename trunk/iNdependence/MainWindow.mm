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

	if (img != nil) {
		waitDialog = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, 496, 286)
												styleMask:NSTitledWindowMask
												  backing:NSBackingStoreBuffered
													defer:NO];
		waitDialogImage = [[NSImageView alloc] initWithFrame:NSMakeRect(20, 69, 456, 150)];
		[waitDialogImage setImage:img];
		[waitDialogImage setImageAlignment:NSImageAlignCenter];
		[waitDialogImage setImageScaling:NSScaleProportionally];
		[waitDialogImage setImageFrameStyle:NSImageFrameNone];
		[[waitDialog contentView] addSubview:waitDialogImage];
	}
	else {
		waitDialog = [[NSPanel alloc] initWithContentRect:NSMakeRect(0, 0, 496, 150)
												styleMask:NSTitledWindowMask
												  backing:NSBackingStoreBuffered
													defer:NO];
		waitDialogImage = nil;
	}

	if (title != nil) {
		[waitDialog setTitle:title];
	}

	if (msg != nil) {

		if (img != nil) {
			waitDialogMessage = [[NSTextField alloc] initWithFrame:NSMakeRect(17, 231, 461, 35)];
		}
		else {
			waitDialogMessage = [[NSTextField alloc] initWithFrame:NSMakeRect(17, 81, 461, 35)];
		}

		[waitDialogMessage setStringValue:msg];
		[waitDialogMessage setEditable:NO];
		[waitDialogMessage setSelectable:NO];
		[waitDialogMessage setBezeled:NO];
		[waitDialogMessage setBackgroundColor:[NSColor clearColor]];
		[waitDialogMessage setDrawsBackground:NO];
		[waitDialogMessage setAlignment:NSCenterTextAlignment];
		[[waitDialog contentView] addSubview:waitDialogMessage];
	}
	else {
		waitDialogMessage = nil;
	}

	if (cancel) {
		waitDialogCancel = [[NSButton alloc] initWithFrame:NSMakeRect(400, 12, 82, 32)];
		[waitDialogCancel setButtonType:NSMomentaryLightButton];
		[waitDialogCancel setBezelStyle:NSRoundedBezelStyle];
		[waitDialogCancel setTitle:@"Cancel"];
		[waitDialogCancel setTarget:appController];
		[waitDialogCancel setAction:@selector(waitDialogCancel:)];
		[[waitDialog contentView] addSubview:waitDialogCancel];
	}
	else {
		waitDialogCancel = nil;
	}

	waitDialogSpinner = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(231, 20, 32, 32)];
	[waitDialogSpinner setIndeterminate:YES];
	[waitDialogSpinner setControlSize:NSRegularControlSize];
	[waitDialogSpinner setStyle:NSProgressIndicatorSpinningStyle];
	[waitDialogSpinner startAnimation:self];
	[[waitDialog contentView] addSubview:waitDialogSpinner];

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

	[waitDialogSpinner release];
	waitDialogSpinner = nil;

	if (waitDialogCancel != nil) {
		[waitDialogCancel setTarget:nil];
		[waitDialogCancel setAction:nil];
		[waitDialogCancel release];
		waitDialogCancel = nil;
	}

	if (waitDialogMessage != nil) {
		[waitDialogMessage release];
		waitDialogMessage = nil;
	}

	if (waitDialogImage != nil) {
		[waitDialogImage release];
		waitDialogImage = nil;
	}

	[waitDialog release];
	waitDialog = nil;
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
