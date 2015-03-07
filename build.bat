%echo off

rem Definitions

setlocal
set PATH=%PATH%;%ICPP_COMPILER15%\bin\intel64\;C:\GnuWin32\bin\;"C:\Program Files (x86)\Git\bin"
git rev-list --max-count=1 --abbrev-commit HEAD > %TEMP%\jarvis-commit-id.txt
set /p COMMITID=<%TEMP%\jarvis-commit-id.txt
git status -s -uno >> %TEMP%\jarvis-commit-id.txt
sed "N;s/\n/ /" %TEMP%\jarvis-commit-id.txt > %TEMP%\jarvis-commit-id-plus.txt
set /p COMMITIDPLUS=<%TEMP%\jarvis-commit-id-plus.txt
del %TEMP%\jarvis-commit-id.txt %TEMP%\jarvis-commit-id-plus.txt

rem One-time code generation
rem There is not unistd.h on Windows platform, remove the line
rem Windows platform call the function _isatty() and fix the prototype

echo loader.y
bison -d -o util\loader.cc util\loader.y
echo scanner.l
flex -outil\scanner-tmp.cc util\scanner.l
sed -e "/#include <unistd.h>/d" ^
    -e "s/extern int isatty/extern \"C\" int isatty/" ^
    -e "s/isatty/_isatty/" util\scanner-tmp.cc > util\scanner.cc

rem Debug build of the library with the Intel compiler

icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\graph.cc /DCOMMIT_ID="""%COMMITIDPLUS%"""
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\GraphConfig.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\node.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\edge.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\property.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\stringid.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\StringTable.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\PropertyList.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\TransactionManager.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\transaction.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\Index.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\IndexManager.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\EdgeIndex.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\IndexString.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\AvlTree.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\AvlTreeIndex.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\FixedAllocator.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\Allocator.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Fosrc\ /c src\windows.cc

lib /nologo /OUT:src\jarvis.lib /MACHINE:X64 src\graph.obj ^
    src\GraphConfig.obj src\node.obj src\edge.obj src\property.obj ^
    src\stringid.obj src\StringTable.obj src\PropertyList.obj ^
    src\TransactionManager.obj src\transaction.obj src\Index.obj ^
    src\IndexManager.obj src\EdgeIndex.obj src\IndexString.obj ^
    src\AvlTree.obj src\AvlTreeIndex.obj src\FixedAllocator.obj ^
    src\Allocator.obj src\windows.obj

icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Foutil\ /c util\loader.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Foutil\ /c util\scanner.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Foutil\ /c util\exception.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Foutil\ /c util\text.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Foutil\ /c util\dump.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Foutil\ /c util\load_tsv.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /Od /GR- /EHsc /MDd /Z7 /DNOPM /Foutil\ /c util\load_gson.cc /I3rdparty

lib /nologo /OUT:util\jarvis-util.lib /MACHINE:X64 util\exception.obj ^
    util\text.obj util\dump.obj util\load_tsv.obj util\load_gson.obj ^
    util\loader.obj util\scanner.obj

icl /nologo /Od /MDd /Z7 /Dstrncasecmp=_strnicmp ^
    /Fo3rdparty\strptime\ /c 3rdparty\strptime\strptime.c
lib /nologo /OUT:3rdparty\strptime\strptime.lib /MACHINE:X64 ^
    3rdparty\strptime\strptime.obj

icl /nologo /TP /Qstd=c++11 /Od /GR- /EHsc /MDd /Z7 ^
    /Fo3rdparty\jsoncpp\ /c 3rdparty\jsoncpp\jsoncpp.cpp
lib /nologo /OUT:3rdparty\jsoncpp\jsoncpp.lib /MACHINE:X64 ^
    3rdparty\jsoncpp\jsoncpp.obj

mkdir lib\Debug
copy src\jarvis.lib       lib\Debug
copy util\jarvis-util.lib lib\Debug
mkdir 3rdparty\lib\Debug
copy 3rdparty\strptime\strptime.lib 3rdparty\lib\Debug
copy 3rdparty\jsoncpp\jsoncpp.lib   3rdparty\lib\Debug

rem Release build of the library with the Intel compiler

icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\graph.cc /DCOMMIT_ID="""%COMMITIDPLUS%"""
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\GraphConfig.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\node.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\edge.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\property.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\stringid.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\StringTable.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\PropertyList.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\TransactionManager.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\transaction.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\Index.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\IndexManager.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\EdgeIndex.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\IndexString.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\AvlTree.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\AvlTreeIndex.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\FixedAllocator.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\Allocator.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Fosrc\ /c src\windows.cc

lib /nologo /OUT:src\jarvis.lib /MACHINE:X64 src\graph.obj ^
    src\GraphConfig.obj src\node.obj src\edge.obj src\property.obj ^
    src\stringid.obj src\StringTable.obj src\PropertyList.obj ^
    src\TransactionManager.obj src\transaction.obj src\Index.obj ^
    src\IndexManager.obj src\EdgeIndex.obj src\IndexString.obj ^
    src\AvlTree.obj src\AvlTreeIndex.obj src\FixedAllocator.obj ^
    src\Allocator.obj src\windows.obj

icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Foutil\ /c util\loader.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Foutil\ /c util\scanner.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Foutil\ /c util\exception.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Foutil\ /c util\text.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Foutil\ /c util\dump.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Foutil\ /c util\load_tsv.cc
icl /nologo /TP /Qstd=c++11 /Iinclude /O3 /GR- /EHsc /MD  /Z7 /DNOPM /Foutil\ /c util\load_gson.cc /I3rdparty

lib /nologo /OUT:util\jarvis-util.lib /MACHINE:X64 util\exception.obj ^
    util\text.obj util\dump.obj util\load_tsv.obj util\load_gson.obj ^
    util\loader.obj util\scanner.obj

icl /nologo /O3 /MD /Z7 /Dstrncasecmp=_strnicmp ^
    /Fo3rdparty\strptime\ /c 3rdparty\strptime\strptime.c
lib /nologo /OUT:3rdparty\strptime\strptime.lib /MACHINE:X64 ^
    3rdparty\strptime\strptime.obj

icl /nologo /TP /Qstd=c++11 /Ijsoncpp /O3 /GR- /EHsc /MD /Z7 ^
    /Fo3rdparty\jsoncpp\ /c 3rdparty\jsoncpp\jsoncpp.cpp
lib /nologo /OUT:3rdparty\jsoncpp\jsoncpp.lib /MACHINE:X64 ^
    3rdparty\jsoncpp\jsoncpp.obj

mkdir lib\Release
copy src\jarvis.lib       lib\Release
copy util\jarvis-util.lib lib\Release
mkdir 3rdparty\lib\Release
copy 3rdparty\strptime\strptime.lib 3rdparty\lib\Release
copy 3rdparty\jsoncpp\jsoncpp.lib   3rdparty\lib\Release

rem Build the tools with the Visual Studio compiler

cl /nologo /W2 /TP /Iinclude /Iutil /O2 /GR- /EHsc /MD ^
    /Fotools\mkgraph.obj /c tools\mkgraph.cc
link /nologo tools\mkgraph.obj ^
    lib\Release\jarvis-util.lib ^
    lib\Release\jarvis.lib ^
    /OUT:tools\mkgraph.exe /MACHINE:X64 ^
    /LIBPATH:"%VSINSTALLDIR%\VC\lib\amd64" ^
    /LIBPATH:"%ICPP_COMPILER15%\compiler\lib\intel64" ^
    /LIBPATH:"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\x64"

cl /nologo /W2 /TP /Iinclude /Iutil /O2 /GR- /EHsc /MD ^
    /Fotools\dumpgraph.obj /c tools\dumpgraph.cc
link /nologo tools\dumpgraph.obj ^
    lib\Release\jarvis-util.lib ^
    lib\Release\jarvis.lib ^
    3rdparty\lib\Release\strptime.lib ^
    /OUT:tools\dumpgraph.exe /MACHINE:X64 ^
    /LIBPATH:"%VSINSTALLDIR%\VC\lib\amd64" ^
    /LIBPATH:"%ICPP_COMPILER15%\compiler\lib\intel64" ^
    /LIBPATH:"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\x64"

cl /nologo /W2 /TP /Iinclude /Iutil /O2 /GR- /EHsc /MD ^
    /Fotools\loadgraph.obj /c tools\loadgraph.cc
link /nologo tools\loadgraph.obj ^
    lib\Release\jarvis-util.lib ^
    lib\Release\jarvis.lib ^
    3rdparty\lib\Release\strptime.lib ^
    3rdparty\lib\Release\jsoncpp.lib ^
    /OUT:tools\loadgraph.exe /MACHINE:X64 ^
    /LIBPATH:"%VSINSTALLDIR%\VC\lib\amd64" ^
    /LIBPATH:"%ICPP_COMPILER15%\compiler\lib\intel64" ^
    /LIBPATH:"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\x64"

mkdir bin
copy tools\*.exe bin

rem Archive

zip -q jarvis-%COMMITID%-bin.zip ^
    include\*.h util\util.h util\neighbor.h util\annotate.h ^
    lib\Release\*.lib bin\*.exe ^
    3rdparty\lib\Release\*.lib 3rdparty\jsoncpp\LICENSE ^
    3rdparty\jsoncpp\ORIGIN 3rdparty\jsoncpp\jsoncpp.cpp ^
    3rdparty\jsoncpp\json\* ^
    3rdparty\strptime\README 3rdparty\strptime\COPYING ^
    3rdparty\strptime\ORIGIN 3rdparty\strptime\strptime.c
zip -q jarvis-%COMMITID%-dbg.zip lib\Debug\*.lib 3rdparty\lib\Debug\*.lib
zip -q jarvis-%COMMITID%-src.zip src\*.h src\*.cc ^
    util\loader.h util\*.l util\*.y util\dump.cc util\exception.cc ^
    util\load_gson.cc util\load_tsv.cc util\text.cc tools\*.cc

rem Build soltest and propertytest

cl /nologo /TP /Iinclude /Iutil /O2 /GR- /EHsc /MD ^
    /Fotest\soltest.obj /c test\soltest.cc
link /nologo test\soltest.obj ^
    lib\Release\jarvis-util.lib ^
    lib\Release\jarvis.lib ^
    /OUT:test\soltest.exe /MACHINE:X64 ^
    /LIBPATH:"%VSINSTALLDIR%\VC\lib\amd64" ^
    /LIBPATH:"%ICPP_COMPILER15%\compiler\lib\intel64" ^
    /LIBPATH:"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\x64"

cl /nologo /TP /Iinclude /Iutil /O2 /GR- /EHsc /MD ^
    /Fotest\propertytest.obj /c test\propertytest.cc
link /nologo test\propertytest.obj ^
    lib\Release\jarvis-util.lib ^
    lib\Release\jarvis.lib ^
    3rdparty\lib\Release\strptime.lib ^
    /OUT:test\propertytest.exe /MACHINE:X64 ^
    /LIBPATH:"%VSINSTALLDIR%\VC\lib\amd64" ^
    /LIBPATH:"%ICPP_COMPILER15%\compiler\lib\intel64" ^
    /LIBPATH:"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\x64"
