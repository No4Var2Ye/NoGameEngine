@echo off
set "exe_path=.\MyEngine\myengine.exe"

if exist "%exe_path%" (
    start "" "%exe_path%"
) else (
    echo Error: %exe_path% 不存在
    pause
)