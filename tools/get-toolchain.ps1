# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

echo "`nInstalling prerequisites. Please leave the window open until the installation completes."

$cmake_path = 'https://github.com/Kitware/CMake/releases/download/v3.18.3'
$cmake_file = 'cmake-3.18.3-win32-x86.msi'
$cmake_name = 'CMake v3.18.3'
$cmake_hash = 'F39D53371F5330F0C18C544A78F9654B9F799050B157783BD9E0E20F28F27821'

$gccarm_path = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4'
$gccarm_file = 'gcc-arm-none-eabi-9-2019-q4-major-win32-sha2.exe'
$gccarm_name = 'GCC-ARM 9-2019q4-major'
$gccarm_hash = '3580550590B3D2C0998DCF01673EBAD7D9DFD4F6F0436F86CF2C507539C62450'

$termite_path = 'https://www.compuphase.com/software'
$termite_file = 'termite-3.4.exe'
$termite_name = 'Termite v3.4'
$termite_hash = 'CA440B6C7F6EAA812BA5F8BF42AED86E02022CA50A1C72585168C9B671D0FE19'

$iot_explorer_path = 'https://github.com/Azure/azure-iot-explorer/releases/download/v0.11.4'
$iot_explorer_file = 'Azure.IoT.Explorer.preview.0.11.4.msi'
$iot_explorer_name = 'Azure IoT Explorer v0.11.4'
$iot_explorer_hash = '8CAA36336C1C4C55C70D868904639263F56D5D20CB59DC0A18F6817DC0AEC900'

echo "`nDownloading packages..."

echo "(1/4) $cmake_name"
if ( -not (Test-Path "$env:TEMP\$cmake_file") -Or ((Get-FileHash "$env:TEMP\$cmake_file").Hash -ne $cmake_hash))
{
    (New-Object System.Net.WebClient).DownloadFile("$cmake_path\$cmake_file", "$env:TEMP\$cmake_file")
}

echo "(2/4) $gccarm_name"
if ( -not (Test-Path "$env:TEMP\$gccarm_file") -Or ((Get-FileHash "$env:TEMP\$gccarm_file").Hash -ne $gccarm_hash))
{
    (New-Object System.Net.WebClient).DownloadFile("$gccarm_path\$gccarm_file", "$env:TEMP\$gccarm_file")
}

echo "(3/4) $termite_name"
if ( -not (Test-Path "$env:TEMP\$termite_file") -Or ((Get-FileHash "$env:TEMP\$termite_file").Hash -ne $termite_hash))
{
    (New-Object System.Net.WebClient).DownloadFile("$termite_path\$termite_file", "$env:TEMP\$termite_file")
}

echo "(4/4) $iot_explorer_name"
if ( -not (Test-Path "$env:TEMP\$iot_explorer_file") -Or ((Get-FileHash "$env:TEMP\$iot_explorer_file").Hash -ne $iot_explorer_hash))
{
    (New-Object System.Net.WebClient).DownloadFile("$iot_explorer_path\$iot_explorer_file", "$env:TEMP\$iot_explorer_file")
}

echo "`nInstalling packages..."

echo "(1/4) $cmake_name"
Start-Process -wait -FilePath "$env:TEMP\$cmake_file" -ArgumentList "ADD_CMAKE_TO_PATH=System /passive"

echo "(2/4) $gccarm_name"
Start-Process -wait -FilePath "$env:TEMP\$gccarm_file" -ArgumentList "/S /P /R"

echo "(3/4) $termite_name"
Start-Process -wait -FilePath "$env:TEMP\$termite_file" -ArgumentList "/S"

echo "(4/4) $iot_explorer_name"
Start-Process -wait -FilePath "$env:TEMP\$iot_explorer_file" -ArgumentList "/passive"

echo "`nInstallation complete!"

echo "`nPress any key to continue..."
Read-Host
