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

		NSSize size = [img size];
		float widthScale = 1.0f, heightScale = 1.0f;

		if (size.width > 456.0f) {
			widthScale = 456.0f / size.width;
		}

		if (size.height > 150.0f) {
			heightScale = 150.0f / size.height;
		}

		if ( (widthScale != 1.0f) || (heightScale != 1.0f) ) {

			if (widthScale <= heightScale) {
				size.width = widthScale * size.width;
				size.height = widthScale * size.height;
			}
			else {
				size.width = heightScale * size.width;
				size.height = heightScale * size.height;
			}

		}

		NSPoint loc = NSMakePoint(20, 69);

		if (size.width < 456.0f) {
			loc.x += (456.0f - size.width) / 2.0f;
		}

		if (size.height < 150.0f) {
			loc.y += (150.0f - size.height) / 2.0f;
		}

		waitDialogImage = [[NSImageView alloc] initWithFrame:NSMakeRect(loc.x, loc.y, size.width, size.height)];
		[waitDialogImage setImage:img];
		[waitDialogImage setImageAlignment:NSImageAlignCenter];
		[waitDialogImage setImageScaling:NSScaleProportionally];
		[waitDialogImage setImageFrameStyle:NSImageFramePhoto];
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
	else if ([appController isInDFUMode]) {
		statusStr = [statusStr stringByAppendingString:@"Connected to iPhone in DFU mode"];
	}
	else if ([appController isConnected]) {
		statusStr = [statusStr stringByAppendingFormat:@"Connected to iPhone %C Firmware ", 0x2022];
		statusStr = [statusStr stringByAppendingString:[appController phoneFirmwareVersion]];

		if ([appController isActivated]) {
			statusStr = [statusStr stringByAppendingFormat:@" %C activated", 0x2022];
		}
		else {
			statusStr = [statusStr stringByAppendingFormat:@" %C not activated", 0x2022];
		}

		if ([appController isAFCConnected]) {
			statusStr = [statusStr stringByAppendingFormat:@" %C AFC connection", 0x2022];
		}
		else {
			statusStr = [statusStr stringByAppendingFormat:@" %C no AFC connection", 0x2022];
		}

		if ([appController isJailbroken]) {
			statusStr = [statusStr stringByAppendingFormat:@" %C jailbroken", 0x2022];
		}
		else {
			statusStr = [statusStr stringByAppendingFormat:@" %C jailed", 0x2022];
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
