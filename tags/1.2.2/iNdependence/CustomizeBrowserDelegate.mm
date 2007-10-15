/*
 *  CustomizeBrowserDelegate.mm
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

#import "CustomizeBrowserDelegate.h"
#include "PhoneInteraction/PhoneInteraction.h"
#include "PhoneInteraction/SSHHelper.h"
#include "PhoneInteraction/PNGHelper.h"


static char* g_systemApps[17] = { "Calculator.app", "DemoApp.app", "FieldTest.app", "Maps.app",
								  "MobileCal.app", "MobileMail.app", "MobileMusicPlayer.app",
								  "MobileNotes.app", "MobilePhone.app", "MobileSafari.app",
								  "MobileSlideShow.app", "MobileSMS.app", "MobileTimer.app",
								  "Preferences.app", "Stocks.app", "Weather.app", "YouTube.app" };
static int g_numSystemApps = 17;


@implementation CustomizeBrowserDelegate

- (void)dealloc
{
	[m_col1Items release];
	[m_col2Dictionary release];
	[super dealloc];
}

- (void)awakeFromNib
{
	m_col1Items = [NSArray arrayWithObjects:@"Ringtones", @"Wallpapers", @"Applications", nil];
	[m_col1Items retain];
	m_col2Dictionary = [NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:
		[NSArray arrayWithObjects:@"System", @"User", nil],
		[NSArray arrayWithObjects:@"System", @"User", nil],
		[NSArray arrayWithObjects:@"System", @"User", nil], nil] forKeys:m_col1Items];
	[m_col2Dictionary retain];
}

- (NSImage*)loadAndConvertPNG:(const char*)path
{
	NSImage *retimg = nil;
	unsigned char *buf;
	int size;
				
	if (PhoneInteraction::getInstance()->getFileData((void**)&buf, &size, path, 0, 0) == true) {
		unsigned char *newbuf;
		int bufsize;
					
		// need to convert Apple's PNG files to a normal format
		if (PNGHelper::convertPNGToUseful(buf, size, &newbuf, &bufsize) == true) {
			retimg = [[[NSImage alloc] initWithData:[NSData dataWithBytes:newbuf length:bufsize]] autorelease];
			free(newbuf);
		}
		else {
			// it's likely a normal PNG file already
			retimg = [[[NSImage alloc] initWithData:[NSData dataWithBytes:buf length:size]] autorelease];
		}

		free(buf);
	}

	return retimg;
}

- (const char*)getPhoneDirectory:(NSString*)col1sel col2sel:(NSString*)col2sel
{
	
	if ([col1sel isEqualToString:@"Ringtones"]) {

		if ([col2sel isEqualToString:@"System"]) {
			return "/Library/Ringtones";
		}
		else if ([col2sel isEqualToString:@"User"]) {
			return "/var/root/Library/Ringtones";
		}

	}
	else if ([col1sel isEqualToString:@"Wallpapers"]) {
		
		if ([col2sel isEqualToString:@"System"]) {
			return "/Library/Wallpaper";
		}
		else if ([col2sel isEqualToString:@"User"]) {
			return "/var/root/Library/Wallpaper";
		}

	}
	else if ([col1sel isEqualToString:@"Applications"]) {
		return "/Applications";
	}

	return NULL;
}

- (NSArray*)getValidFileList:(NSString*)col1sel col2sel:(NSString*)col2sel
{
	const char *dir = [self getPhoneDirectory:col1sel col2sel:col2sel];

	if (dir == NULL) return nil;

	NSMutableArray *outputArray = nil;

	if ([col1sel isEqualToString:@"Ringtones"]) {
		char **filenames;
		int numFiles;
		
		if (PhoneInteraction::getInstance()->directoryFileList(dir, &filenames, &numFiles)) {
			outputArray = [[[NSMutableArray alloc] initWithCapacity:numFiles] autorelease];
			
			for (int i = 0; i < numFiles; i++) {
				char *index = strstr(filenames[i], ".m4a");
				
				if (index != NULL) {
					[outputArray addObject:[NSString stringWithUTF8String:filenames[i]]];
				}
				
			}
			
			for (int i = 0; i < numFiles; i++) {
				free(filenames[i]);
			}
			
			free(filenames);
		}
		
	}
	else if ([col1sel isEqualToString:@"Wallpapers"]) {
		char **filenames;
		int numFiles;
		
		if (PhoneInteraction::getInstance()->directoryFileList(dir, &filenames, &numFiles)) {
			outputArray = [[[NSMutableArray alloc] initWithCapacity:numFiles] autorelease];
			
			for (int i = 0; i < numFiles; i++) {
				char *index = strstr(filenames[i], ".png");
				
				if (index != NULL) {
					
					if (!strstr(filenames[i], ".poster.png") && !strstr(filenames[i], ".thumbnail.png")) {
						[outputArray addObject:[NSString stringWithUTF8String:filenames[i]]];
					}
					
				}
				
			}
			
			for (int i = 0; i < numFiles; i++) {
				free(filenames[i]);
			}
			
			free(filenames);
		}
		
	}
	else if ([col1sel isEqualToString:@"Applications"]) {
		bool bSystem = false;
		
		if ([col2sel isEqualToString:@"System"]) {
			bSystem = true;
		}
		
		char **filenames;
		int numFiles;
		
		if (PhoneInteraction::getInstance()->directoryFileList(dir, &filenames, &numFiles)) {
			outputArray = [[[NSMutableArray alloc] initWithCapacity:numFiles] autorelease];
			
			for (int i = 0; i < numFiles; i++) {
				char *index = strstr(filenames[i], ".app");
				
				if (index != NULL) {
					
					if (bSystem) {
						
						for (int j = 0; j < g_numSystemApps; j++) {
							
							if (!strcmp(filenames[i], g_systemApps[j])) {
								[outputArray addObject:[NSString stringWithUTF8String:filenames[i]]];
								break;
							}
							
						}
						
					}
					else {
						bool bIsSystemApp = false;
						
						for (int j = 0; j < g_numSystemApps; j++) {
							
							if (!strcmp(filenames[i], g_systemApps[j])) {
								bIsSystemApp = true;
								break;
							}
							
						}
						
						if (!bIsSystemApp) {
							[outputArray addObject:[NSString stringWithUTF8String:filenames[i]]];
						}
						
					}
					
				}
				
			}
			
			for (int i = 0; i < numFiles; i++) {
				free(filenames[i]);
			}
			
			free(filenames);
		}
		
	}
	
	return outputArray;
}

- (void)browser:(NSBrowser*)sender willDisplayCell:(id)cell atRow:(int)row column:(int)column
{
	[cell setEnabled:[sender isEnabled]];

	if (column == 0) {
		NSString *col1item = (NSString*)[m_col1Items objectAtIndex:row];

		[cell setLeaf:NO];
		[cell setTitle:col1item];

		if ([col1item isEqualToString:@"Ringtones"]) {
			[cell setImage:[NSImage imageNamed:@"RingtoneFolder"]];
		}
		else if ([col1item isEqualToString:@"Wallpapers"]) {
			[cell setImage:[NSImage imageNamed:@"WallpaperFolder"]];
		}
		else if ([col1item isEqualToString:@"Applications"]) {
			[cell setImage:[[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(kApplicationsFolderIcon)]];
		}
		else {
			[cell setImage:[[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(kGenericFolderIcon)]];
		}

	}
	else if (column == 1) {
		NSString *col1sel = (NSString*)[m_col1Items objectAtIndex:[sender selectedRowInColumn:0]];
		NSString *col2item = (NSString*)[(NSArray*)[m_col2Dictionary objectForKey:col1sel] objectAtIndex:row];

		[cell setLeaf:NO];
		[cell setTitle:col2item];

		if ([col2item isEqualToString:@"System"]) {
			[cell setImage:[[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(kSystemFolderIcon)]];
		}
		else if ([col2item isEqualToString:@"User"]) {
			[cell setImage:[NSImage imageNamed:@"UserFolder"]];
		}
		else {
			[cell setImage:[[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(kGenericFolderIcon)]];
		}

	}
	else if (column == 2) {
		[cell setLeaf:YES];
		NSString *col1sel = (NSString*)[m_col1Items objectAtIndex:[sender selectedRowInColumn:0]];
		NSString *col2sel = (NSString*)[(NSArray*)[m_col2Dictionary objectForKey:col1sel] objectAtIndex:[sender selectedRowInColumn:1]];
		const char *path = [self getPhoneDirectory:col1sel col2sel:col2sel];

		if ([col1sel isEqualToString:@"Ringtones"] || [col1sel isEqualToString:@"Applications"]) {
			NSArray *fileList = [self getValidFileList:col1sel col2sel:col2sel];
			NSString *filepath = [NSString stringWithUTF8String:path];
			NSString *filename = (NSString*)[fileList objectAtIndex:row];
			NSString *fullpath = [NSString stringWithFormat:@"%@/%@", filepath, filename];

			[cell setRepresentedObject:[NSArray arrayWithObjects:fullpath, nil]];
			[cell setTitle:filename];

			if ([col1sel isEqualToString:@"Ringtones"]) {
				[cell setImage:[[NSWorkspace sharedWorkspace] iconForFileType:[filename pathExtension]]];
			}
			else if ([col1sel isEqualToString:@"Applications"]) {
				NSString *iconpath = [NSString stringWithFormat:@"%@/%@", fullpath, @"icon.png"];
				NSImage *img = [self loadAndConvertPNG:[iconpath UTF8String]];
	
				if ( (img != nil) && [img isValid] ) {
					[img setScalesWhenResized:YES];
					[img setSize:NSMakeSize(32,32)];
					[cell setImage:img];
				}
				else {
					[cell setImage:[[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(kGenericApplicationIcon)]];
				}

			}

		}
		else if ([col1sel isEqualToString:@"Wallpapers"]) {
			char **filenames;
			int numFiles;
				
			if (PhoneInteraction::getInstance()->directoryFileList(path, &filenames, &numFiles)) {
				NSString *filepath = [NSString stringWithUTF8String:path];
				int count = -1;

				for (int i = 0; i < numFiles; i++) {
					char *index = strstr(filenames[i], ".png");
						
					if (index != NULL) {
							
						if (!strstr(filenames[i], ".poster.png") && !strstr(filenames[i], ".thumbnail.png")) {
							count++;

							if (count == row) {
								int baselen = index - filenames[i];
								NSString *fileString = [NSString stringWithUTF8String:filenames[i]];
								NSString *thumbnail = nil;
								NSString *regular = [NSString stringWithFormat:@"%@/%@", filepath, fileString];
								NSMutableArray *fileList = [NSMutableArray arrayWithCapacity:2];

								[fileList addObject:regular];

								for (int j = 0; j < numFiles; j++) {

									if (strstr(filenames[j], ".thumbnail.png")) {

										if (!strncmp(filenames[i], filenames[j], baselen)) {
											thumbnail = [NSString stringWithFormat:@"%@/%@", filepath, [NSString stringWithUTF8String:filenames[j]]];
											[fileList addObject:thumbnail];
											break;
										}

									}

								}

								if (thumbnail == nil) {
									thumbnail = regular;
								}

								NSImage *img = [self loadAndConvertPNG:[thumbnail UTF8String]];
								
								if ( (img != nil) && [img isValid] ) {
									[img setScalesWhenResized:YES];
									[img setSize:NSMakeSize(32,32)];
									[cell setImage:img];
								}
								else {
									[cell setImage:[[NSWorkspace sharedWorkspace] iconForFileType:@".png"]];
								}

								[cell setRepresentedObject:fileList];
								[cell setTitle:fileString];
								break;
							}

						}
							
					}

				}
					
				for (int i = 0; i < numFiles; i++) {
					free(filenames[i]);
				}
					
				free(filenames);
			}
					
		}

	}

}

- (int)browser:(NSBrowser*)sender numberOfRowsInColumn:(int)column
{

	if (column == 0) {
		return [m_col1Items count];
	}
	else if (column == 1) {
		NSString *col1sel = (NSString*)[m_col1Items objectAtIndex:[sender selectedRowInColumn:0]];
		return [(NSArray*)[m_col2Dictionary objectForKey:col1sel] count];
	}
	else if (column == 2) {
		NSString *col1sel = (NSString*)[m_col1Items objectAtIndex:[sender selectedRowInColumn:0]];
		NSString *col2sel = (NSString*)[(NSArray*)[m_col2Dictionary objectForKey:col1sel] objectAtIndex:[sender selectedRowInColumn:1]];
		return [[self getValidFileList:col1sel col2sel:col2sel] count];
	}

	return 0;
}

- (BOOL)browser:(NSBrowser*)sender isColumnValid:(int)column
{
	NSArray *cells = [[sender matrixInColumn:column] cells];
	int numCells = [cells count];

	if (column == 0) {

		if (numCells != [m_col1Items count]) return NO;

		NSString *str1, *str2;

		for (int i = 0; i < numCells; i++) {
			str1 = [[cells objectAtIndex:i] stringValue];
			str2 = [m_col1Items objectAtIndex:i];

			if (![str1 isEqualToString:str2]) {
				return NO;
			}

		}

	}
	else if (column == 1) {
		NSString *col1sel = (NSString*)[m_col1Items objectAtIndex:[sender selectedRowInColumn:0]];
		NSArray *col2Items = (NSArray*)[m_col2Dictionary objectForKey:col1sel];

		if (numCells != [col2Items count]) return NO;

		NSString *str1, *str2;

		for (int i = 0; i < numCells; i++) {
			str1 = [[cells objectAtIndex:i] stringValue];
			str2 = [col2Items objectAtIndex:i];
			
			if (![str1 isEqualToString:str2]) {
				return NO;
			}
			
		}

	}
	else if (column == 2) {
		NSString *col1sel = (NSString*)[m_col1Items objectAtIndex:[sender selectedRowInColumn:0]];
		NSString *col2sel = (NSString*)[(NSArray*)[m_col2Dictionary objectForKey:col1sel] objectAtIndex:[sender selectedRowInColumn:1]];
		NSArray *fileList = [self getValidFileList:col1sel col2sel:col2sel];

		if (numCells != [fileList count]) return NO;

		NSString *str1, *str2;

		for (int i = 0; i < numCells; i++) {
			str1 = [[cells objectAtIndex:i] stringValue];
			str2 = [fileList objectAtIndex:i];

			if (![str1 isEqualToString:str2]) {
				return NO;
			}

		}

	}

	return YES;
}

- (bool)acceptDraggedFiles:(NSArray*)files
{
	NSString *col1sel = (NSString*)[m_col1Items objectAtIndex:[m_browser selectedRowInColumn:0]];

	if ([col1sel isEqualToString:@"Ringtones"]) {
		NSString *file;

		for (int i = 0; i < [files count]; i++) {
			file = (NSString*)[files objectAtIndex:i];

			if ( [[file pathExtension] caseInsensitiveCompare:@"m4a"] != NSOrderedSame ) {
				return false;
			}

		}

		return true;
	}
	else if ([col1sel isEqualToString:@"Wallpapers"]) {

		if ([files count] < 2) return false;

		NSString *file;
		
		for (int i = 0; i < [files count]; i++) {
			file = (NSString*)[files objectAtIndex:i];

			if ( [[file pathExtension] caseInsensitiveCompare:@"png"] != NSOrderedSame ) {
				return false;
			}
			
		}
		
		return true;
	}
	else if ([col1sel isEqualToString:@"Applications"]) {
		NSString *file;
		
		for (int i = 0; i < [files count]; i++) {
			file = (NSString*)[files objectAtIndex:i];

			if ( [[file pathExtension] caseInsensitiveCompare:@"app"] != NSOrderedSame ) {
				return false;
			}
			
		}
		
		return true;
	}

	return false;
}

- (bool)putRingtone:(bool)bInSystemDir wasCancelled:(bool*)cancelled
{
	NSOpenPanel *fileOpener = [NSOpenPanel openPanel];
	[fileOpener setTitle:@"Choose the ringtone file"];
	[fileOpener setCanChooseDirectories:NO];
	[fileOpener setAllowsMultipleSelection:NO];
	
	NSArray *fileTypes = [NSArray arrayWithObject:@"m4a"];
	
	if ([fileOpener runModalForTypes:fileTypes] != NSOKButton) {

		if (cancelled != NULL) {
			*cancelled = true;
		}

		return false;
	}

	NSArray *files = [NSArray arrayWithObjects:[fileOpener filename], nil];

	if (cancelled != NULL) {
		*cancelled = false;
	}

	return [self addFilesToPhone:files wasCancelled:cancelled];
}

- (bool)putWallpaper:(bool)bInSystemDir wasCancelled:(bool*)cancelled
{
	NSOpenPanel *fileOpener = [NSOpenPanel openPanel];
	[fileOpener setTitle:@"Choose the wallpaper file"];
	[fileOpener setCanChooseDirectories:NO];
	[fileOpener setAllowsMultipleSelection:NO];
	
	NSArray *fileTypes = [NSArray arrayWithObject:@"png"];
	
	if ([fileOpener runModalForTypes:fileTypes] != NSOKButton) {

		if (cancelled != NULL) {
			*cancelled = true;
		}

		return false;
	}

	NSString *wallpaperFile = [NSString stringWithString:[fileOpener filename]];
	
	[fileOpener setTitle:@"Choose the wallpaper thumbnail file"];
	[fileOpener setCanChooseDirectories:NO];
	[fileOpener setAllowsMultipleSelection:NO];
	
	fileTypes = [NSArray arrayWithObject:@"png"];
	
	if ([fileOpener runModalForTypes:fileTypes] != NSOKButton) {

		if (cancelled != NULL) {
			*cancelled = true;
		}

		return false;
	}

	NSArray *files = [NSArray arrayWithObjects:wallpaperFile, [fileOpener filename], nil];

	if (cancelled != NULL) {
		*cancelled = false;
	}

	return [self addFilesToPhone:files wasCancelled:cancelled];
}

- (bool)putApplication:(bool*)cancelled
{
	NSOpenPanel *fileOpener = [NSOpenPanel openPanel];
	[fileOpener setTitle:@"Choose the application"];
	[fileOpener setCanChooseDirectories:NO];
	[fileOpener setAllowsMultipleSelection:NO];
	
	NSArray *fileTypes = [NSArray arrayWithObject:@"app"];
	
	if ([fileOpener runModalForTypes:fileTypes] != NSOKButton) {

		if (cancelled != NULL) {
			*cancelled = true;
		}

		return false;
	}
	
	NSArray *files = [NSArray arrayWithObjects:[fileOpener filename], nil];

	if (cancelled != NULL) {
		*cancelled = false;
	}

	return [self addFilesToPhone:files wasCancelled:cancelled];
}

- (bool)addFilesToPhone:(NSArray*)files wasCancelled:(bool*)bCancelled
{
	int row = [m_browser selectedRowInColumn:0];
	NSString *title = (NSString*)[m_col1Items objectAtIndex:row];
	
	if ([title isEqualToString:@"Ringtones"]) {
		row = [m_browser selectedRowInColumn:1];
		title = (NSString*)[[m_col2Dictionary objectForKey:title] objectAtIndex:row];
		bool bInSystemDir = false;
		
		if ([title isEqualToString:@"System"]) {
			bInSystemDir = true;
		}

		NSString *filename;

		for (int i = 0; i < [files count]; i++) {
			filename = [files objectAtIndex:i];

			if ( [[filename pathExtension] caseInsensitiveCompare:@"m4a"] == NSOrderedSame ) {

				if (PhoneInteraction::getInstance()->ringtoneExists([[filename lastPathComponent] UTF8String], bInSystemDir)) {
					int retval = NSRunAlertPanel(@"Ringtone Exists",
												[NSString stringWithFormat:@"Ringtone file %@ already exists.  Do you wish to keep the existing file or replace it?", [filename lastPathComponent]],
												@"Keep", @"Replace", nil);

					if (retval == 1) {
						continue;
					}

					if (!PhoneInteraction::getInstance()->removeRingtone([[filename lastPathComponent] UTF8String], bInSystemDir)) {
						[m_mainWindow displayAlert:@"Failed" message:@"Error removing old ringtone from phone"];
						return false;
					}

				}

				if (!PhoneInteraction::getInstance()->putRingtoneOnPhone([filename UTF8String], bInSystemDir)) {
					return false;
				}

			}

		}

	}
	else if ([title isEqualToString:@"Wallpapers"]) {

		if ([files count] < 2) {
			return false;
		}

		NSString *nsFilename = [files objectAtIndex:0], *nsFilename2 = [files objectAtIndex:1];
		
		if ( ([nsFilename hasSuffix:@".png"] == NO) || ([nsFilename2 hasSuffix:@".png"] == NO) ) {
			return false;
		}
		
		row = [m_browser selectedRowInColumn:1];
		title = (NSString*)[[m_col2Dictionary objectForKey:title] objectAtIndex:row];
		bool bInSystemDir = false;
		
		if ([title isEqualToString:@"System"]) {
			bInSystemDir = true;
		}

		if (PhoneInteraction::getInstance()->wallpaperExists([[nsFilename lastPathComponent] UTF8String], bInSystemDir)) {
			int retval = NSRunAlertPanel(@"Wallpaper Exists",
										 [NSString stringWithFormat:@"Wallpaper file %@ already exists.  Do you wish to keep the existing file or replace it?", [nsFilename lastPathComponent]],
										 @"Keep", @"Replace", nil);

			if (retval == NSAlertDefaultReturn) {
				return true;
			}

			if (!PhoneInteraction::getInstance()->removeWallpaper([[nsFilename lastPathComponent] UTF8String], bInSystemDir)) {
				[m_mainWindow displayAlert:@"Failed" message:@"Error removing old wallpaper from phone"];
				return false;
			}

		}

		const char *filename = [nsFilename UTF8String], *filename2 = [nsFilename UTF8String];
		
		if (!PhoneInteraction::getInstance()->putWallpaperOnPhone(filename, filename2, bInSystemDir)) {
			return false;
		}

	}
	else if ([title isEqualToString:@"Applications"]) {
		NSString *filename, *ipAddress, *password, *appName;
		
		if ([m_sshHandler getSSHInfo:&ipAddress password:&password wasCancelled:bCancelled] == false) {
			return false;
		}

		for (int i = 0; i < [files count]; i++) {
			filename = [files objectAtIndex:i];

			if ( [[filename pathExtension] caseInsensitiveCompare:@"app"] == NSOrderedSame ) {
				
				if (PhoneInteraction::getInstance()->applicationExists([[filename lastPathComponent] UTF8String])) {
					int retval = NSRunAlertPanel(@"Application Exists",
												 [NSString stringWithFormat:@"Application %@ already exists.  Do you wish to keep the existing application or replace it?", [filename lastPathComponent]],
												 @"Keep", @"Replace", nil);
					
					if (retval == 1) {
						continue;
					}

					if (!PhoneInteraction::getInstance()->removeApplication([[filename lastPathComponent] UTF8String])) {
						[m_mainWindow displayAlert:@"Failed" message:@"Error removing old application from phone"];
						return false;
					}
					
				}
				
				if (!PhoneInteraction::getInstance()->putApplicationOnPhone([filename UTF8String])) {
					return false;
				}

				appName = [NSString stringWithFormat:@"%@/%@", @"/Applications", [filename lastPathComponent]];

				bool done = false;
				int retval;

				while (!done) {
					[m_mainWindow startDisplayWaitingSheet:@"Setting Permissions" message:@"Setting application permissions..." image:nil
											  cancelButton:false runModal:false];
					retval = SSHHelper::copyPermissions([filename UTF8String], [appName UTF8String], [ipAddress UTF8String],
														[password UTF8String]);
					[m_mainWindow endDisplayWaitingSheet];

					if (retval != SSH_HELPER_SUCCESS) {

						switch (retval)
						{
							case SSH_HELPER_ERROR_NO_RESPONSE:
								PhoneInteraction::getInstance()->removeApplication([[filename lastPathComponent] UTF8String]);
								[m_mainWindow displayAlert:@"Failed" message:@"Couldn't connect to SSH server.  Ensure IP address is correct, phone is connected to a network, and SSH is installed correctly."];
								done = true;
								break;
							case SSH_HELPER_ERROR_BAD_PASSWORD:
								PhoneInteraction::getInstance()->removeApplication([[filename lastPathComponent] UTF8String]);
								[m_mainWindow displayAlert:@"Failed" message:@"root password is incorrect."];
								done = true;
								break;
							case SSH_HELPER_VERIFICATION_FAILED:
								int retval = NSRunAlertPanel(@"Failed", @"Host verification failed.  Would you like iNdependence to try and fix this for you by editing ~/.ssh/known_hosts?", @"No", @"Yes", nil);

								if (retval == NSAlertAlternateReturn) {

									if (![m_sshHandler removeKnownHostsEntry:ipAddress]) {
										PhoneInteraction::getInstance()->removeApplication([[filename lastPathComponent] UTF8String]);
										[m_mainWindow displayAlert:@"Failed" message:@"Couldn't remove entry from ~/.ssh/known_hosts.  Please edit that file by hand and remove the line containing your phone's IP address."];
										done = true;
									}

								}
								else {
									done = true;
								}

								break;
							default:
								PhoneInteraction::getInstance()->removeApplication([[filename lastPathComponent] UTF8String]);
								[m_mainWindow displayAlert:@"Failed" message:@"Error setting permissions for application."];
								done = true;
								break;
						}

					}
					else {
						done = true;
					}

				}

				[m_browser validateVisibleColumns];
				return false;
			}

		}

	}

	[m_browser validateVisibleColumns];
	return true;
}

- (IBAction)selectionChanged:(id)sender
{
	int column = [sender selectedColumn];
	int row = [sender selectedRowInColumn:column];

	if ( (column >= 1) && (row >= 0) ) {
		[m_addButton setEnabled:YES];
	}
	else {
		[m_addButton setEnabled:NO];
	}

	if ( (column == 2) && (row >= 0) ) {
		[m_deleteButton setEnabled:YES];
	}
	else {
		[m_deleteButton setEnabled:NO];
	}

}

- (IBAction)deleteButtonPressed:(id)sender
{
	int row = [m_browser selectedRowInColumn:0];
	NSString *title = (NSString*)[m_col1Items objectAtIndex:row];
	bool bIsApplication = false;
	NSString *ipAddress, *password;

	if ([title isEqualToString:@"Applications"]) {
		bool bCancelled = false;

		if ([m_sshHandler getSSHInfo:&ipAddress password:&password wasCancelled:&bCancelled] == false) {
			return;
		}

		if (bCancelled) {
			return;
		}

		bIsApplication = true;
	}

	NSBrowserCell *cell = (NSBrowserCell*)[m_browser selectedCell];
	NSArray *files = (NSArray*)[cell representedObject];

	for (int i = 0; i < [files count]; i++) {

		if (!PhoneInteraction::getInstance()->removePath([(NSString*)[files objectAtIndex:i] UTF8String])) {
			[m_mainWindow displayAlert:@"Failed" message:@"Error deleting selection"];
			return;
		}

	}

	if (bIsApplication) {
		[m_mainWindow startDisplayWaitingSheet:@"Restarting Springboard" message:@"Restarting Springboard..." image:nil
								  cancelButton:false runModal:false];

		int retval = SSHHelper::restartSpringboard([ipAddress UTF8String], [password UTF8String]);

		[m_mainWindow endDisplayWaitingSheet];

		if (retval != SSH_HELPER_SUCCESS) {

			switch (retval)
			{
				case SSH_HELPER_ERROR_NO_RESPONSE:
					[m_mainWindow displayAlert:@"Error" message:@"Couldn't connect to SSH server to restart SpringBoard.  Ensure IP address is correct, phone is connected to a network, and SSH is installed correctly."];
					break;
				case SSH_HELPER_ERROR_BAD_PASSWORD:
					[m_mainWindow displayAlert:@"Error" message:@"Couldn't restart SpringBoard.  root password is incorrect."];
					break;
				default:
					[m_mainWindow displayAlert:@"Error" message:@"Error restarting SpringBoard."];
					break;
			}

		}
		
	}

	[m_browser validateVisibleColumns];
}

- (IBAction)addButtonPressed:(id)sender
{
	int row = [m_browser selectedRowInColumn:0];
	NSString *title = (NSString*)[m_col1Items objectAtIndex:row];

	if ([title isEqualToString:@"Ringtones"]) {
		row = [m_browser selectedRowInColumn:1];
		title = (NSString*)[[m_col2Dictionary objectForKey:title] objectAtIndex:row];
		bool bInSystemDir = false, bCancelled = false;

		if ([title isEqualToString:@"System"]) {
			bInSystemDir = true;
		}

		bool retval = [self putRingtone:bInSystemDir wasCancelled:&bCancelled];

		if (bCancelled) return;

		if (!retval) {
			return;
		}

	}
	else if ([title isEqualToString:@"Wallpapers"]) {
		row = [m_browser selectedRowInColumn:1];
		title = (NSString*)[[m_col2Dictionary objectForKey:title] objectAtIndex:row];
		bool bInSystemDir = false, bCancelled = false;
		
		if ([title isEqualToString:@"System"]) {
			bInSystemDir = true;
		}

		bool retval = [self putWallpaper:bInSystemDir wasCancelled:&bCancelled];

		if (bCancelled) return;

		if (!retval) {
			return;
		}
		
	}
	else if ([title isEqualToString:@"Applications"]) {
		bool bCancelled = false;		
		bool retval = [self putApplication:&bCancelled];

		if (bCancelled) return;
		
		if (!retval) {
			return;
		}
		
	}

	[m_browser validateVisibleColumns];
}

@end
