/*

 MIT License

 Copyright © 2020 Samuel Venable

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

*/

#import "DragInWindow.h"
#import "SharedFuncs.h"

@implementation NSWindow(DragInWindow)

NSString *_draggedFiles = @"";
NSPasteboard *_draggedPasteboard = nil;
BOOL _initDrag = NO;

-(NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
  if (file_dnd_get_enabled() && !_initDrag)
  _initDrag = YES; else _initDrag = NO;
  return NSDragOperationCopy;
}

-(NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
  return NSDragOperationCopy;
}

-(BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  if (file_dnd_get_enabled() && _initDrag) {
    [_draggedPasteboard clearContents];
    _draggedPasteboard = [sender draggingPasteboard];
    [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
    _initDrag = NO;
    return YES;
  } else {
    [_draggedPasteboard clearContents];
    _draggedPasteboard = nil;
    _initDrag = NO;
    return NO;
  }
}

-(NSPasteboard *)draggedPasteboard {
  return _draggedPasteboard;
}

-(void)setDraggedPasteboard:(NSPasteboard *)draggedPasteboard {
  if (draggedPasteboard != _draggedPasteboard) {
    [_draggedPasteboard clearContents];
    _draggedPasteboard = draggedPasteboard;
  }
}

-(NSString *)draggedFiles {
  return _draggedFiles;
}

-(void)setDraggedFiles:(NSString *)draggedFiles {
  if (draggedFiles != _draggedFiles)
    _draggedFiles = draggedFiles;
}

-(BOOL)initDrag {
  return _initDrag;
}

@end
