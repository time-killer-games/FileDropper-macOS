cd "${0%/*}"
g++ -c -std=c++17 "FileDropper.part1.cpp" -fPIC -arch arm64 -arch x86_64
g++ -c -ObjC "FileDropper.part2.mm" "DragInWindow.mm" -fPIC -arch arm64 -arch x86_64
g++ "FileDropper.part1.o" "FileDropper.part2.o" "DragInWindow.o" -o "FileDropper (x64)/FileDropper.dylib" -shared -framework Cocoa -fPIC -arch arm64 -arch x86_64
