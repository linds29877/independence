#import "MainWindow.h"
#include "PhoneInteraction/UtilityFunctions.h"
#include "PhoneInteraction/PhoneInteraction.h"


enum
{
	MENU_ITEM_PUT_FILE = 10,
	MENU_ITEM_GET_FILE = 11,
	MENU_ITEM_CREATE_DIR = 12,
	MENU_ITEM_REMOVE_PATH = 13,
	MENU_ITEM_PUT_RINGTONE = 14,
	MENU_ITEM_PUT_WALLPAPER = 15
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
		[putFileButton setEnabled:YES];
		[getFileButton setEnabled:YES];
		[createDirButton setEnabled:YES];
		[removeButton setEnabled:YES];
		[putRingtoneButton setEnabled:YES];
		[putWallpaperButton setEnabled:YES];
	}
	else {
		[putFileButton setEnabled:NO];
		[getFileButton setEnabled:NO];
		[createDirButton setEnabled:NO];
		[removeButton setEnabled:NO];
		[putRingtoneButton setEnabled:NO];
		[putWallpaperButton setEnabled:NO];
	}
	
	[self updateStatus];
}

- (bool)isJailbroken
{
	return m_jailbroken;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApp
{
	return YES;
}

- (BOOL)validateMenuItem:(NSMenuItem*)menuItem
{
	
	switch ([menuItem tag]) {
		case MENU_ITEM_PUT_FILE:
		case MENU_ITEM_GET_FILE:
		case MENU_ITEM_CREATE_DIR:
		case MENU_ITEM_REMOVE_PATH:
		case MENU_ITEM_PUT_RINGTONE:
		case MENU_ITEM_PUT_WALLPAPER:

			if (![self isConnected] || ![self isJailbroken]) {
				return NO;
			}

			break;
		default:
			break;
	}
	
	return YES;
}

- (IBAction)putFile:(id)sender
{
	NSOpenPanel *fileOpener = [NSOpenPanel openPanel];
	[fileOpener setTitle:@"Choose the file"];
	[fileOpener setCanChooseDirectories:NO];
	[fileOpener setAllowsMultipleSelection:NO];

	if ([fileOpener runModalForTypes:nil] != NSOKButton) {
		return;
	}

	[NSApp beginSheet:pathWindow modalForWindow:self modalDelegate:nil didEndSelector:nil
		  contextInfo:nil];

	if ([NSApp runModalForWindow:pathWindow] == -1) {
		[NSApp endSheet:pathWindow];
		[pathWindow orderOut:self];
		return;
	}

	[NSApp endSheet:pathWindow];
	[pathWindow orderOut:self];
		
	if (m_phoneInteraction->putFile([[fileOpener filename] UTF8String], (char*)[[pathField stringValue] UTF8String]) == false) {
		[self displayAlert:@"Failed" message:@"Error writing file"];
	}
	else {
		[self displayAlert:@"Success" message:@"Successfully wrote to file"];
	}
	
}

- (IBAction)getFile:(id)sender
{
	[NSApp beginSheet:pathWindow modalForWindow:self modalDelegate:nil didEndSelector:nil
		  contextInfo:nil];

	if ([NSApp runModalForWindow:pathWindow] == -1) {
		[NSApp endSheet:pathWindow];
		[pathWindow orderOut:self];
		return;
	}

	[NSApp endSheet:pathWindow];
	[pathWindow orderOut:self];

	NSOpenPanel *dirOpener = [NSOpenPanel openPanel];
	[dirOpener setTitle:@"Choose where you want to put it"];
	[dirOpener setCanChooseDirectories:YES];
	[dirOpener setCanChooseFiles:NO];
	[dirOpener setAllowsMultipleSelection:NO];
	
	if ([dirOpener runModalForTypes:nil] != NSOKButton) {
		return;
	}

	const char *phoneFilePath = [[pathField stringValue] UTF8String];
	const char *phoneFileName = UtilityFunctions::getLastPathElement(phoneFilePath);
	NSString *computerFilePath = [[dirOpener filename] stringByAppendingFormat:@"/%s", phoneFileName];

	if (m_phoneInteraction->getFile(phoneFilePath, [computerFilePath UTF8String]) == false) {
		[self displayAlert:@"Failed" message:@"Error reading file"];
	}
	else {
		[self displayAlert:@"Success" message:@"Successfully obtained file"];
	}
	
}

- (IBAction)createDir:(id)sender
{
	[NSApp beginSheet:pathWindow modalForWindow:self modalDelegate:nil didEndSelector:nil
		  contextInfo:nil];
	
	if ([NSApp runModalForWindow:pathWindow] == -1) {
		[NSApp endSheet:pathWindow];
		[pathWindow orderOut:self];
		return;
	}

	[NSApp endSheet:pathWindow];
	[pathWindow orderOut:self];

	if (m_phoneInteraction->createDirectory([[pathField stringValue] UTF8String]) == false) {
		[self displayAlert:@"Failed" message:@"Error creating directory"];
	}
	else {
		[self displayAlert:@"Success" message:@"Successfully created directory"];
	}

}

- (IBAction)removePath:(id)sender
{
	[NSApp beginSheet:pathWindow modalForWindow:self modalDelegate:nil didEndSelector:nil
		  contextInfo:nil];
	
	if ([NSApp runModalForWindow:pathWindow] == -1) {
		[NSApp endSheet:pathWindow];
		[pathWindow orderOut:self];
		return;
	}
	
	[NSApp endSheet:pathWindow];
	[pathWindow orderOut:self];

	if (m_phoneInteraction->removePath([[pathField stringValue] UTF8String]) == false) {
		[self displayAlert:@"Failed" message:@"Error removing path"];
	}
	else {
		[self displayAlert:@"Success" message:@"Successfully removed path"];
	}

}

- (IBAction)putRingtone:(id)sender
{
	NSOpenPanel *fileOpener = [NSOpenPanel openPanel];
	[fileOpener setTitle:@"Choose the ringtone file"];
	[fileOpener setCanChooseDirectories:NO];
	[fileOpener setAllowsMultipleSelection:NO];
	
	NSArray *fileTypes = [NSArray arrayWithObject:@"m4a"];

	if ([fileOpener runModalForTypes:fileTypes] != NSOKButton) {
		return;
	}
	
	if (m_phoneInteraction->putRingtoneOnPhone([[fileOpener filename] UTF8String]) == false) {
		[self displayAlert:@"Failed" message:@"Error adding ringtone to phone"];
	}
	else {
		[self displayAlert:@"Success" message:@"Successfully added ringtone"];
	}
	
}

- (IBAction)putWallpaper:(id)sender
{
	NSOpenPanel *fileOpener = [NSOpenPanel openPanel];
	[fileOpener setTitle:@"Choose the wallpaper file"];
	[fileOpener setCanChooseDirectories:NO];
	[fileOpener setAllowsMultipleSelection:NO];
	
	NSArray *fileTypes = [NSArray arrayWithObject:@"png"];
	
	if ([fileOpener runModalForTypes:fileTypes] != NSOKButton) {
		return;
	}

	const char *wallpaperFile = [[fileOpener filename] UTF8String];

	[fileOpener setTitle:@"Choose the wallpaper thumbnail file"];
	[fileOpener setCanChooseDirectories:NO];
	[fileOpener setAllowsMultipleSelection:NO];

	fileTypes = [NSArray arrayWithObject:@"png"];
	
	if ([fileOpener runModalForTypes:fileTypes] != NSOKButton) {
		return;
	}

	if (m_phoneInteraction->putWallpaperOnPhone(wallpaperFile, [[fileOpener filename] UTF8String]) == false) {
		[self displayAlert:@"Failed" message:@"Error adding wallpaper to phone"];
	}
	else {
		[self displayAlert:@"Success" message:@"Successfully added wallpaper"];
	}
	
}

- (IBAction)dialogCancel:(id)sender
{
	[NSApp stopModalWithCode:-1];
}

- (IBAction)dialogOk:(id)sender
{
	[NSApp stopModalWithCode:0];
}

@end
