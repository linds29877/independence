#import "MainWindow.h"
#include "PhoneInteraction/UtilityFunctions.h"
#include "PhoneInteraction/PhoneInteraction.h"


enum
{
	MENU_ITEM_COPY_CORE_FILESYSTEM = 10,
	MENU_ITEM_COPY_FULL_FILESYSTEM = 11,
	MENU_ITEM_COPY_MEDIA_FILESYSTEM = 12
};


static MainWindow *g_mainWindow;


void updateStatus(const char *msg, bool waiting)
{
	
	if (g_mainWindow) {
		[g_mainWindow setStatus:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
	}
	
}

void phoneInteractionNotification(int type, const char *msg)
{
	
	if (g_mainWindow) {
		
		switch (type) {
			case NOTIFY_CONNECTED:
				[g_mainWindow setConnected:true];
				break;
			case NOTIFY_DISCONNECTED:
				[g_mainWindow setConnected:false];
				break;
			case NOTIFY_INITIALIZATION_FAILED:
			case NOTIFY_CONNECTION_FAILED:
				[g_mainWindow displayAlert:@"Failed" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_FSCOPY_FAILED:
				[g_mainWindow updateStatus];
				[g_mainWindow endDisplayWaitingSheet];
				[g_mainWindow displayAlert:@"Failed" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_FSCOPY_SUCCESS:
				[g_mainWindow updateStatus];
				[g_mainWindow endDisplayWaitingSheet];
				[g_mainWindow displayAlert:@"Success" message:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
				break;
			case NOTIFY_CONNECTION_SUCCESS:
			case NOTIFY_INITIALIZATION_SUCCESS:
			default:
				break;
		}
		
	}
	
}

@implementation MainWindow

- (void)awakeFromNib
{
	[NSApp setDelegate:self];
	g_mainWindow = self;
	m_connected = false;
	m_jailbroken = false;
	m_phoneInteraction = PhoneInteraction::getInstance(updateStatus, phoneInteractionNotification);
}

- (void)displayAlert:(NSString*)title message:(NSString*)msg
{
	NSRunAlertPanel(title, msg, @"OK", nil, nil, nil);
}

- (void)setStatus:(NSString*)text
{
	[statusBar setStringValue:text];
}

- (void)updateStatus
{
	NSString *statusStr = [NSString string];
	
	if ([self isConnected]) {
		statusStr = [statusStr stringByAppendingString:@"Connected to iPhone"];

		if ([self isJailbroken]) {
			statusStr = [statusStr stringByAppendingString:@" -- jailbroken"];
		}
		
	}
	else {
		statusStr = [statusStr stringByAppendingString:@"Disconnected: waiting for iPhone..."];
	}
	
	[self setStatus:statusStr];
}

- (void)setConnected:(bool)connected
{
	m_connected = connected;
	
	if (m_connected) {
		[self setJailbroken:m_phoneInteraction->isPhoneJailbroken()];
	}
	else {
		[self setJailbroken:false];
	}
	
	[self updateStatus];
}

- (bool)isConnected
{
	return m_connected;
}

- (void)setJailbroken:(bool)jailbroken
{
	m_jailbroken = jailbroken;
	
	if (m_jailbroken) {
		[copyCoreButton setEnabled:YES];
		[copyFullButton setEnabled:YES];
		[copyMediaButton setEnabled:YES];
	}
	else {
		[copyCoreButton setEnabled:NO];
		[copyFullButton setEnabled:NO];
		[copyMediaButton setEnabled:NO];
	}
	
	[self updateStatus];
}

- (bool)isJailbroken
{
	return m_jailbroken;
}

- (void)startDisplayWaitingSheet:(NSString*)title message:(NSString*)msg
{
	
	if (title != nil) {
		[waitDialog setTitle:title];
	}
	
	if (msg != nil) {
		[waitDialogMessage setStringValue:msg];
	}

	[waitDialogSpinner startAnimation:self];
	
	[NSApp beginSheet:waitDialog modalForWindow:self modalDelegate:nil
	   didEndSelector:nil contextInfo:nil];
}

- (void)endDisplayWaitingSheet
{
	[waitDialogSpinner stopAnimation:self];
	[NSApp endSheet:waitDialog];
	[waitDialog orderOut:self];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApp
{
	return YES;
}

- (BOOL)validateMenuItem:(NSMenuItem*)menuItem
{
	
	switch ([menuItem tag]) {
		case MENU_ITEM_COPY_CORE_FILESYSTEM:
		case MENU_ITEM_COPY_FULL_FILESYSTEM:
		case MENU_ITEM_COPY_MEDIA_FILESYSTEM:

			if (![self isConnected] || ![self isJailbroken]) {
				return NO;
			}

			break;
		default:
			break;
	}
	
	return YES;
}

- (IBAction)copyCoreFilesystem:(id)sender
{
	NSOpenPanel *dirOpener = [NSOpenPanel openPanel];
	[dirOpener setTitle:@"Choose where you want to put it"];
	[dirOpener setCanChooseDirectories:YES];
	[dirOpener setCanChooseFiles:NO];
	[dirOpener setAllowsMultipleSelection:NO];

	if ([dirOpener runModalForTypes:nil] != NSOKButton) {
		return;
	}

	[self startDisplayWaitingSheet:@"Waiting for copy" message:@"Copying files from phone..."];
	m_phoneInteraction->copyPhoneFilesystem("/", [[dirOpener filename] UTF8String], true);
}

- (IBAction)copyFullFilesystem:(id)sender
{
	NSOpenPanel *dirOpener = [NSOpenPanel openPanel];
	[dirOpener setTitle:@"Choose where you want to put it"];
	[dirOpener setCanChooseDirectories:YES];
	[dirOpener setCanChooseFiles:NO];
	[dirOpener setAllowsMultipleSelection:NO];
	
	if ([dirOpener runModalForTypes:nil] != NSOKButton) {
		return;
	}

	[self startDisplayWaitingSheet:@"Waiting for copy" message:@"Copying files from phone..."];
	m_phoneInteraction->copyPhoneFilesystem("/", [[dirOpener filename] UTF8String], false);
}

- (IBAction)copyMediaFilesystem:(id)sender
{
	NSOpenPanel *dirOpener = [NSOpenPanel openPanel];
	[dirOpener setTitle:@"Choose where you want to put it"];
	[dirOpener setCanChooseDirectories:YES];
	[dirOpener setCanChooseFiles:NO];
	[dirOpener setAllowsMultipleSelection:NO];
	
	if ([dirOpener runModalForTypes:nil] != NSOKButton) {
		return;
	}

	[self startDisplayWaitingSheet:@"Waiting for copy" message:@"Copying files from phone..."];
	m_phoneInteraction->copyPhoneFilesystem("/var/root/Media/", [[dirOpener filename] UTF8String], true);
}

@end
