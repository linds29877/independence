#import "CustomizeBrowser.h"
#import "CustomizeBrowserCell.h"
#import "CustomizeBrowserDelegate.h"


@implementation CustomizeBrowser

- (void)awakeFromNib
{
	[self setCellClass:[CustomizeBrowserCell class]];
	[self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
	//[self selectRow:0 inColumn:0];
}

- (NSDragOperation)handleDragEvent:(id <NSDraggingInfo>)sender
{
	NSPoint loc = [sender draggingLocation];
	int selColumn = [self selectedColumn];
	bool bHandleDrag = false;
	
	if (selColumn > 0) {
		NSPoint newPt = [self convertPoint:loc fromView:nil];
		NSRect frame = [self frameOfColumn:2];
		
		if (NSPointInRect(newPt, frame)) {
			bHandleDrag = true;
		}
		
	}
	
	if (!bHandleDrag) {
		NSMatrix *matrix = [self matrixInColumn:0];
		NSPoint newPt = [matrix convertPoint:loc fromView:nil];
		int row, col;
			
		if ([matrix getRow:&row column:&col forPoint:newPt]) {

			if ([self selectedRowInColumn:0] != row) {
				[self selectRow:row inColumn:0];
			}
			
		}
		else {
			matrix = [self matrixInColumn:1];
			newPt = [matrix convertPoint:loc fromView:nil];

			if ([matrix getRow:&row column:&col forPoint:newPt]) {
				bHandleDrag = true;
				
				if ([self selectedRowInColumn:1] != row) {
					[self selectRow:row inColumn:1];
				}

			}

		}

	}
	
	if (bHandleDrag) {
		NSPasteboard *pboard = [sender draggingPasteboard];
		NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
		
		if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
			
			if (sourceDragMask & NSDragOperationCopy) {
				NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];

				if ([(CustomizeBrowserDelegate*)[self delegate] acceptDraggedFiles:files]) {
					return NSDragOperationCopy;
				}
				
			}
			
		}
		
	}
	
    return NSDragOperationNone;
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
	return [self handleDragEvent:sender];
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
	return [self handleDragEvent:sender];
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    NSPasteboard *pboard = [sender draggingPasteboard];
    NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
	
    if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
		
        if (sourceDragMask & NSDragOperationCopy) {
			[(CustomizeBrowserDelegate*)[self delegate] addFilesToPhone:[pboard propertyListForType:NSFilenamesPboardType]
														   wasCancelled:NULL];
        }
		
    }
	
    return YES;
}

@end
