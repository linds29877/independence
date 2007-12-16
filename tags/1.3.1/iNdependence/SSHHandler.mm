/*
 *  SSHHandler.mm
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

#import "SSHHandler.h"


@implementation SSHHandler

- (bool)removeKnownHostsEntry:(NSString*)ipAddress
{
	NSString *homedir = NSHomeDirectory();
	
	if (homedir == nil) {
		return false;
	}
	
	NSMutableString *filepath = [NSMutableString stringWithString:homedir];
	
	if ([filepath hasSuffix:@"/"]) {
		[filepath appendString:@".ssh/known_hosts"];
	}
	else {
		[filepath appendString:@"/.ssh/known_hosts"];
	}
	
	NSData *filedata  = [NSData dataWithContentsOfFile:filepath];
	
	if (filedata == nil) {
		return false;
	}
	
	NSMutableString *filecontents = [[NSMutableString alloc] initWithData:filedata encoding:NSASCIIStringEncoding];
	
	if (filecontents == nil) {
		return false;
	}
	
	NSRange iprange = [filecontents rangeOfString:ipAddress];
	
	if (iprange.location == NSNotFound) {
		[filecontents release];
		return false;
	}
	
	NSRange linerange = [filecontents lineRangeForRange:iprange];
	
	if (linerange.location == NSNotFound) {
		[filecontents release];
		return false;
	}
	
	[filecontents deleteCharactersInRange:linerange];
	
	if ([filecontents writeToFile:filepath atomically:YES encoding:NSASCIIStringEncoding error:nil] == NO) {
		[filecontents release];
		return false;
	}
	
	[filecontents release];
	return true;
}

- (bool)getSSHInfo:(NSString**)ipAddress password:(NSString**)password wasCancelled:(bool*)bCancelled
{
	[NSApp beginSheet:m_sshWindow modalForWindow:m_mainWindow modalDelegate:nil didEndSelector:nil
		  contextInfo:nil];
	
	*ipAddress = nil;
	*password = nil;
	
	while ( !(*ipAddress) || !(*password) ) {
		
		if ([NSApp runModalForWindow:m_sshWindow] == -1) {
			
			if (bCancelled != NULL) {
				*bCancelled = true;
			}
			
			[NSApp endSheet:m_sshWindow];
			[m_sshWindow orderOut:self];
			return false;
		}
		
		NSScanner *scanner = [NSScanner scannerWithString:[m_sshIPText1 stringValue]];
		int value;
		
		if ([scanner scanInt:&value] == NO) {
			[m_mainWindow displayAlert:@"Error" message:@"You entered an invalid IP address.  Try again."];
			continue;
		}
		
		if ( (value < 0) || (value > 255) ) {
			[m_mainWindow displayAlert:@"Error" message:@"You entered an invalid IP address.  Try again."];
			continue;
		}
		
		scanner = [NSScanner scannerWithString:[m_sshIPText2 stringValue]];
		
		if ([scanner scanInt:&value] == NO) {
			[m_mainWindow displayAlert:@"Error" message:@"You entered an invalid IP address.  Try again."];
			continue;
		}
		
		if ( (value < 0) || (value > 255) ) {
			[m_mainWindow displayAlert:@"Error" message:@"You entered an invalid IP address.  Try again."];
			continue;
		}
		
		scanner = [NSScanner scannerWithString:[m_sshIPText3 stringValue]];
		
		if ([scanner scanInt:&value] == NO) {
			[m_mainWindow displayAlert:@"Error" message:@"You entered an invalid IP address.  Try again."];
			continue;
		}
		
		if ( (value < 0) || (value > 255) ) {
			[m_mainWindow displayAlert:@"Error" message:@"You entered an invalid IP address.  Try again."];
			continue;
		}
		
		scanner = [NSScanner scannerWithString:[m_sshIPText4 stringValue]];
		
		if ([scanner scanInt:&value] == NO) {
			[m_mainWindow displayAlert:@"Error" message:@"You entered an invalid IP address.  Try again."];
			continue;
		}
		
		if ( (value < 0) || (value > 255) ) {
			[m_mainWindow displayAlert:@"Error" message:@"You entered an invalid IP address.  Try again."];
			continue;
		}
		
		if ([[m_sshPassword stringValue] length] < 1) {
			[m_mainWindow displayAlert:@"Error" message:@"You entered an invalid password.  Try again."];
			continue;
		}
		
		*ipAddress = [[[NSString alloc] initWithFormat:@"%@.%@.%@.%@", [m_sshIPText1 stringValue],
			[m_sshIPText2 stringValue], [m_sshIPText3 stringValue], [m_sshIPText4 stringValue]] autorelease];
		*password = [m_sshPassword stringValue];
	}

	[NSApp endSheet:m_sshWindow];
	[m_sshWindow orderOut:self];

	if (bCancelled != NULL) {
		*bCancelled = false;
	}

	return true;
}

- (BOOL)control:(NSControl*)control isValidObject:(id)obj
{
	
    if ( (control == m_sshIPText1) || (control == m_sshIPText2) ||
		 (control == m_sshIPText3) || (control == m_sshIPText4) ) {
		NSScanner *scanner = [NSScanner scannerWithString:obj];
		int value;
		
		if ([scanner scanInt:&value] == NO) {
			return NO;
		}
		
		if ( (value < 0) || (value > 255) ) {
			return NO;
		}
		
    }
	else if (control == m_sshPassword) {
		
		if ([obj length] < 1) {
			return NO;
		}
		
	}
	
    return YES;
}

- (IBAction)sshDialogCancel:(id)sender
{
	[NSApp stopModalWithCode:-1];
}

- (IBAction)sshDialogOk:(id)sender
{
	[NSApp stopModalWithCode:0];
}

@end
