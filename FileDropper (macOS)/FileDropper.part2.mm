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

NSWindow *window = nil;
NSPasteboard *pboard = nil;
bool enabled = false;

bool cocoa_file_dnd_get_enabled() {
  return enabled;
}

void cocoa_file_dnd_set_enabled(bool enable) {
  if (window != nil) {
    enabled = enable;
    if (enable) {
      [window registerForDraggedTypes:[NSArray arrayWithObject:NSPasteboardTypeFileURL]];
      [window setDraggedPasteboard:nil];
    } else {
      [window setDraggedPasteboard:nil];
      [window unregisterDraggedTypes];
    }
  }
}

const char *cocoa_file_dnd_get_files() {
  if (window != nil && enabled) {
    pboard = [window draggedPasteboard];
    if (pboard != nil && [pboard pasteboardItems] != nil && [[pboard types] containsObject:NSPasteboardTypeFileURL]) {
      NSArray *URLs = [pboard readObjectsForClasses:@[[NSURL class]] options:nil];
      int URLsSize = [URLs count];
      if (URLsSize > 1) {
        NSMutableArray *fileArray = [[NSMutableArray alloc] init];
        for (int URLIndex = 0; URLIndex < URLsSize; URLIndex += 1) {
          [fileArray addObject:[[URLs objectAtIndex:URLIndex] path]];
        }
        [window setDraggedFiles:[fileArray componentsJoinedByString:@"\n"]];
        [fileArray release];
      } else
        [window setDraggedFiles:[[URLs objectAtIndex:0] path]];
      return [[window draggedFiles] UTF8String];
    }
  }
  return "";
}

void *cocoa_file_dnd_get_hwnd() {
  return (void *)window;
}

void cocoa_file_dnd_set_hwnd(void *hwnd) {
  window = (NSWindow *)hwnd;
}
