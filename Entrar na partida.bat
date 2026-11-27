@echo off
setlocal EnableDelayedExpansion
set "HOST_IP="
set /p "HOST_IP=Digite o IPv4 do computador que criou a partida: "
if not defined HOST_IP exit /b 1
"%~dp0pong.exe" join "!HOST_IP!"
if errorlevel 1 pause
