IF NOT EXIST %TEMP%\vs_community.exe powershell "wget https://aka.ms/vs/16/release/vs_community.exe -o %TEMP%\vs_community.exe"
start /wait %TEMP%\vs_community.exe ^
--wait ^
--passive ^
--norestart ^
--includeRecommended ^
--add Component.MDD.Linux ^
--add Component.Linux.CMake ^
--add Component.MDD.Linux.GCC.arm ^
--add Microsoft.VisualStudio.Component.Git > nul
echo %errorlevel%