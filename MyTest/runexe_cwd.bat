@echo off
echo Searching for exe files in current directory...
dir /b *.exe

for %%f in (*.exe) do (
    echo Starting %%f...
    start "" "%%f"
    exit /b
)

echo No exe files found in current directory.
pause