/* MainWindow */

#import <Cocoa/Cocoa.h>


class PhoneInteraction;

@interface MainWindow : NSWindow
{
    IBOutlet NSButton *copyCoreButton;
    IBOutlet NSButton *copyFullButton;
	IBOutlet NSTextField *statusBar;
	IBOutlet NSWindow* waitDialog;
	IBOutlet NSTextField* waitDialogMessage;
	IBOutlet NSProgressIndicator* waitDialogSpinner;
	
	bool m_connected;
	bool m_jailbroken;
	PhoneInteraction *m_phoneInteraction;
}

- (void)displayAlert:(NSString*)title message:(NSString*)msg;

- (void)startDisplayWaitingSheet:(NSString*)title message:(NSString*)msg;
- (void)endDisplayWaitingSheet;

- (void)setStatus:(NSString*)text;
- (void)updateStatus;

- (void)setConnected:(bool)connected;
- (bool)isConnected;

- (void)setJailbroken:(bool)jailbroken;
- (bool)isJailbroken;

- (IBAction)copyCoreFilesystem:(id)sender;
- (IBAction)copyFullFilesystem:(id)sender;

@end
