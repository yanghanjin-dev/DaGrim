@echo off
setlocal EnableExtensions
set "PATH=C:\msys64\ucrt64\bin;%PATH%"

where gcc >nul 2>&1
if errorlevel 1 (
  echo gcc not found. Expected C:\msys64\ucrt64\bin\gcc.exe
  exit /b 1
)

if not exist build mkdir build

windres -I res -I src --codepage=65001 res\DaGrim.rc -O coff -o build\dagrim.res
if errorlevel 1 exit /b 1

gcc -O2 -std=c11 -municode -mwindows -finput-charset=UTF-8 ^
  -Isrc -Ires -Ithird_party ^
  src\main.c src\util.c src\layer.c src\doc.c src\composite.c ^
  src\image_io.c src\undo.c src\adjust.c src\select.c src\brush.c src\paintbar.c src\tools.c src\ui.c ^
  build\dagrim.res ^
  -lgdi32 -lcomctl32 -lcomdlg32 -luser32 -lshell32 -luxtheme -lm -static-libgcc ^
  -o DaGrim.exe
if errorlevel 1 exit /b 1

echo Built DaGrim.exe
endlocal
