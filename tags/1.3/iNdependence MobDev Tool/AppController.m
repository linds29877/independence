/*
 *  AppController.m
 *  iNdependence MobDev Tool
 *
 *  Created by The Operator on 08/11/07.
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

#import "AppController.h"
#include "install_name_tool.h"


@implementation AppController

- (BOOL)determineiTunesVersion
{
	NSString *iTunesPath = [[NSWorkspace sharedWorkspace] fullPathForApplication:@"iTunes"];
	
	if (iTunesPath == nil) {
		return NO;
	}

	NSString *versionFilePath = [iTunesPath stringByAppendingPathComponent:@"Contents/version.plist"];
	
	if (versionFilePath == nil) {
		return NO;
	}

	NSDictionary *iTunesVersionDict = [NSDictionary dictionaryWithContentsOfFile:versionFilePath];

	if (iTunesVersionDict == nil) {
		return NO;
	}
	
	iTunesVersion = (NSString*)[iTunesVersionDict valueForKey:@"CFBundleVersion"];
	
	if (iTunesVersion == nil) {
		return NO;
	}
	
	return YES;
}

- (BOOL)validateMobileDeviceVersion:(NSString*)mobDevPath
{

	if (mobDevPath == nil) return NO;

	// sanity check to ensure that the version of MobileDevice is correct
	NSString *mobDevVersionFile = [[mobDevPath stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"Resources/version.plist"];
	NSDictionary *mobDevVersionDict = [NSDictionary dictionaryWithContentsOfFile:mobDevVersionFile];
	
	if (mobDevVersionDict == nil) {
		return NO;
	}
	else {
		NSString *mobDevVersion = (NSString*)[mobDevVersionDict valueForKey:@"CFBundleShortVersionString"];
		
		if (mobDevVersion == nil) {
			return NO;
		}
		else if (![mobDevVersion isEqualToString:@"1.01"]) {
			return NO;
		}
		
	}

	return YES;
}

- (void)awakeFromNib
{
	[NSApp setDelegate:self];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[self doStuff];
}

- (void)doStuff
{
	NSString *mobileDevicePath = nil, *iNdependencePath = nil;
	NSFileManager *fileManager = [NSFileManager defaultManager];

	if (![self determineiTunesVersion]) {
		NSRunAlertPanel(@"Error", @"Error determining installed iTunes version.", @"OK", nil, nil, nil);
	}
	else if ([iTunesVersion hasPrefix:@"7.4"]) {
		mobileDevicePath = @"/System/Library/PrivateFrameworks/MobileDevice.framework/Versions/A/MobileDevice";

		if ([fileManager fileExistsAtPath:mobileDevicePath]) {

			// sanity check to ensure that the version of MobileDevice is correct
			if (![self validateMobileDeviceVersion:mobileDevicePath]) {
				mobileDevicePath = nil;
			}

			if (mobileDevicePath != nil) {
				NSString *msg = [NSString stringWithFormat:@"Detected installed iTunes version is %@\nThis version can be used with iNdependence.\n\nPlease choose the version of iNdependence.app to copy the MobileDevice library into.", iTunesVersion];
				NSRunAlertPanel(@"Alert", msg, @"OK", nil, nil, nil);
			}
			else {
				NSString *msg = [NSString stringWithFormat:@"Detected installed iTunes version is %@\nThis version can be used with iNdependence, however, an incorrect version of the MobileDevice framework was found with it.\n\nPlease choose the location of the version of the MobileDevice framework which was installed with iTunes 7.4.2 to copy into iNdependence.", iTunesVersion];
				NSRunAlertPanel(@"Alert", msg, @"OK", nil, nil, nil);
			}

		}
		else {
			mobileDevicePath = nil;
			NSString *msg = [NSString stringWithFormat:@"Detected installed iTunes version is %@\nThis version can be used with iNdependence.  However, the MobileDevice library was not found.\n\nPlease choose the location of the version of the MobileDevice framework which was installed with iTunes 7.4.2 to copy into iNdependence.", iTunesVersion];
			NSRunAlertPanel(@"Alert", msg, @"OK", nil, nil, nil);
		}

	}
	else {
		NSString *msg = [NSString stringWithFormat:@"Detected iTunes version %@\nThis version cannot be used with iNdependence.\n\nPlease choose the location of the version of the MobileDevice framework which was installed with iTunes 7.4.2 to copy into iNdependence.", iTunesVersion];
		NSRunAlertPanel(@"Alert", msg, @"OK", nil, nil, nil);
	}

	if (mobileDevicePath == nil) {
		NSOpenPanel *fileOpener = [NSOpenPanel openPanel];
		[fileOpener setTitle:@"Choose the MobileDevice framework Folder"];
		[fileOpener setCanChooseDirectories:YES];
		[fileOpener setCanChooseFiles:NO];
		[fileOpener setAllowsMultipleSelection:NO];

		while (mobileDevicePath == nil) {

			if ([fileOpener runModalForTypes:nil] != NSOKButton) {
				[NSApp terminate:self];
			}

			NSString *filename = [fileOpener filename];

			if (filename == nil) {
				[NSApp terminate:self];
			}

			mobileDevicePath = [filename stringByAppendingPathComponent:@"Versions/A/MobileDevice"];

			if (![fileManager fileExistsAtPath:mobileDevicePath]) {
				mobileDevicePath = nil;
				NSRunAlertPanel(@"Error", @"Couldn't find MobileDevice library at specified path.\n\nPlease choose again.", @"OK", nil, nil, nil);
			}
			else if (![self validateMobileDeviceVersion:mobileDevicePath]) {
				mobileDevicePath = nil;
				NSRunAlertPanel(@"Error", @"Invalid version of MobileDevice chosen.  You need to select the version that was installed with iTunes 7.4.2.\n\nPlease choose again.", @"OK", nil, nil, nil);
			}

		}

	}

	NSOpenPanel *fileOpener = [NSOpenPanel openPanel];
	[fileOpener setTitle:@"Choose iNdependence"];
	[fileOpener setCanChooseDirectories:NO];
	[fileOpener setCanChooseFiles:YES];
	[fileOpener setAllowsMultipleSelection:NO];

	NSArray *typeArray = [NSArray arrayWithObject:@"app"];

	while (iNdependencePath == nil) {
		
		if ([fileOpener runModalForTypes:typeArray] != NSOKButton) {
			[NSApp terminate:self];
		}

		iNdependencePath = [fileOpener filename];
		
		if (iNdependencePath == nil) {
			[NSApp terminate:self];
		}

		NSString *tmpString = [iNdependencePath stringByAppendingPathComponent:@"Contents/MacOS/iNdependence"];

		if (![fileManager fileExistsAtPath:tmpString]) {
			iNdependencePath = nil;
			NSRunAlertPanel(@"Error", @"Invalid path chosen.  Please choose again.", @"OK", nil, nil, nil);
		}

	}

	NSString *destination = [iNdependencePath stringByAppendingPathComponent:@"Contents/MacOS/libMobDev.dylib"];

	if ([fileManager fileExistsAtPath:destination]) {

		if (NSRunAlertPanel(@"Alert", @"The MobileDevice library already exists in the iNdependence application bundle.  Overwrite it?", @"OK", @"Cancel", nil, nil) != NSAlertDefaultReturn) {
			[NSApp terminate:self];
		}

		if (![fileManager removeFileAtPath:destination handler:nil]) {
			NSRunAlertPanel(@"Error", @"Unable to delete the copy of the MobileDevice library in the iNdependence application bundle.", @"OK", nil, nil, nil);
			[NSApp terminate:self];
		}

	}

	if (![fileManager copyPath:mobileDevicePath toPath:destination handler:nil]) {
		NSRunAlertPanel(@"Error", @"Error copying MobileDevice library into the iNdependence application bundle.", @"OK", nil, nil, nil);
		[NSApp terminate:self];
	}

	if (changeInstallName("@executable_path/libMobDev.dylib", [destination UTF8String]) == -1) {
		NSRunAlertPanel(@"Error", @"Error changing the install name for the MobileDevice library.", @"OK", nil, nil, nil);
		[NSApp terminate:self];
	}

	NSRunAlertPanel(@"Success", @"Successfully copied the MobileDevice library into iNdependence!", @"OK", nil, nil, nil);
	[NSApp terminate:self];
}

@end
