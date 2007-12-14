/* MainWindow */

#import <Cocoa/Cocoa.h>


class PhoneInteraction;

@interface MainWindow : NSWindow
{
    IBOutlet NSButton *putFileButton;
    IBOutlet NSButton *getFileButton;
	IBOutlet NSButton *createDirButton;
	IBOutlet NSButton *removeButton;
	IBOutlet NSButton *putRingtoneButton;
	IBOutlet NSButton *putWallpaperButton;
	IBOutlet NSTextField *statusBar;
	IBOutlet NSWindow *pathWindow;
	IBOutlet NSTextField *pathField;
	
	bool m_connected;
	bool m_jailbroken;
	PhoneInteraction *m_phoneInteraction;
}

- (void)displayAlert:(NSString*)title message:(NSString*)msg;

- (void)setStatus:(NSString*)text;
- (void)updateStatus;

- (void)setConnected:(bool)connected;
- (bool)isConnected;

- (void)setJailbroken:(bool)jailbroken;
- (bool)isJailbroken;

- (IBAction)putFile:(id)sender;
- (IBAction)getFile:(id)sender;
- (IBAction)createDir:(id)sender;
- (IBAction)removePath:(id)sender;
- (IBAction)putRingtone:(id)sender;
- (IBAction)putWallpaper:(id)sender;

- (IBAction)dialogCancel:(id)sender;
- (IBAction)dialogOk:(id)sender;

@end
