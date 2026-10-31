@echo off
setlocal
set "PATH=C:\msys64\ucrt64\bin;%PATH%"
if not exist build mkdir build
gcc -std=c11 -O1 -ffunction-sections -fdata-sections -Isrc -Ires -Ithird_party tests/basic_editing.c src/adjust.c src/select.c src/layer.c src/doc.c src/util.c src/image_io.c src/composite.c -Wl,--gc-sections -lgdi32 -luser32 -lcomctl32 -lm -o build/test_basic.exe
if errorlevel 1 exit /b 1
build\test_basic.exe
exit /b %errorlevel%
