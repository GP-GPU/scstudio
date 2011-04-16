set VERSION=0.4.0
set BUILD_TYPE=Release
"C:\Program Files\NSIS\makensis" /DVERSION=%VERSION% /DBUILD_TYPE=%BUILD_TYPE% scstudio.nsi
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin\signtool.exe" sign /a scstudio-setup-%VERSION%.exe
