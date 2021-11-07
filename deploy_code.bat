@echo off

"C:\Program Files (x86)\WinSCP\WinSCP.com" ^
  /log="C:\Users\ryan1\Desktop\programming_stuff\HomeCode\deployment_log.log" ^
  /command ^
    "open sftp://pi:Balance1%%21@192.168.1.26/ -hostkey=""ssh-ed25519 255 4KFxWF1jZVc0Co65J/Ub0yhLjt1YILDdT079UzwmNp0=""" ^
    "lcd C:\Users\ryan1\Desktop\programming_stuff\HomeCode\SmartHouseServer" ^
    "cd /home/pi/Desktop/programming_stuff/smart_home_server" ^
    "put home_server.js" ^
    "exit"

set WINSCP_RESULT=%ERRORLEVEL%
if %WINSCP_RESULT% equ 0 (
  echo Success
) else (
  echo Error
)

ssh -T pi@192.168.1.26 "sudo systemctl restart home_server"

pause
exit /b %WINSCP_RESULT%