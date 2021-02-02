# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12;

echo "`nInstalling prerequisites. Please leave the window open until the installation completes."

$gccarm_path = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/'
$gccarm_file = 'gcc-arm-none-eabi-10-2020-q4-major-win32.exe'
$gccarm_name = 'GCC-ARM 10-2020-q4-major'
$gccarm_hash = '9754D95905288AFC787837C3B211A3E502A9EDE8A5F544CFD369C30FEA509CA5'

$cmake_path = 'https://github.com/Kitware/CMake/releases/download/v3.19.2'
$cmake_file = 'cmake-3.19.2-win32-x86.msi'
$cmake_name = 'CMake v3.19.2'
$cmake_hash = '7ABFEA6278B7BF3F6EB1D3B13F9DF79FD675D274AC88A680831B81DF26C92611'

$termite_path = 'https://www.compuphase.com/software'
$termite_file = 'termite-3.4.exe'
$termite_name = 'Termite v3.4'
$termite_hash = 'CA440B6C7F6EAA812BA5F8BF42AED86E02022CA50A1C72585168C9B671D0FE19'

$azure_cli_path = 'https://azurecliprod.blob.core.windows.net/msi'
$azure_cli_file = 'azure-cli-2.17.1.msi'
$azure_cli_name = 'Azure CLI v2.17.1'
$azure_cli_hash = '414B5DC8030A6E1755650CA4966417735E42D1FCC89FE9D2E41C5720BC88AEA3'

$iot_explorer_path = 'https://github.com/Azure/azure-iot-explorer/releases/download/v0.13.2'
$iot_explorer_file = 'Azure.IoT.Explorer.preview.0.13.2.msi'
$iot_explorer_name = 'Azure IoT Explorer v0.13.2'
$iot_explorer_hash = '6B3931F96EFA7622DB8E123E91FFA395584C75E6AC28D08A37492AE30E212F6F'

echo "`nDownloading packages..."

$wc = New-Object System.Net.WebClient
$wc.Headers['User-Agent'] = "Mozilla/4.0"

echo "(1/5) $gccarm_name"
if ( -not (Test-Path "$env:TEMP\$gccarm_file") -Or ((Get-FileHash "$env:TEMP\$gccarm_file").Hash -ne $gccarm_hash))
{
    $wc.DownloadFile("$gccarm_path\$gccarm_file", "$env:TEMP\$gccarm_file")
}

echo "(2/5) $cmake_name"
if ( -not (Test-Path "$env:TEMP\$cmake_file") -Or ((Get-FileHash "$env:TEMP\$cmake_file").Hash -ne $cmake_hash))
{
    $wc.DownloadFile("$cmake_path\$cmake_file", "$env:TEMP\$cmake_file")
}

echo "(3/5) $termite_name"
if ( -not (Test-Path "$env:TEMP\$termite_file") -Or ((Get-FileHash "$env:TEMP\$termite_file").Hash -ne $termite_hash))
{
    $wc.DownloadFile("$termite_path\$termite_file", "$env:TEMP\$termite_file")
}

echo "(4/5) $azure_cli_name"
if ( -not (Test-Path "$env:TEMP\$azure_cli_file") -Or ((Get-FileHash "$env:TEMP\$azure_cli_file").Hash -ne $azure_cli_hash))
{
    $wc.DownloadFile("$azure_cli_path\$azure_cli_file", "$env:TEMP\$azure_cli_file")
}

echo "(5/5) $iot_explorer_name"
if ( -not (Test-Path "$env:TEMP\$iot_explorer_file") -Or ((Get-FileHash "$env:TEMP\$iot_explorer_file").Hash -ne $iot_explorer_hash))
{
    $wc.DownloadFile("$iot_explorer_path\$iot_explorer_file", "$env:TEMP\$iot_explorer_file")
}


echo "`nInstalling packages..."

echo "(1/5) $gccarm_name"
Start-Process -FilePath "$env:TEMP\$gccarm_file" -ArgumentList "/S /P /R" -Wait

echo "(2/5) $cmake_name"
Start-Process -FilePath "$env:TEMP\$cmake_file" -ArgumentList "ADD_CMAKE_TO_PATH=System /passive" -Wait

echo "(3/5) $termite_name"
Start-Process -FilePath "$env:TEMP\$termite_file" -ArgumentList "/S" -Wait

echo "(4/5) $azure_cli_name"
Start-Process -FilePath "$env:TEMP\$azure_cli_file" -ArgumentList "/passive" -Wait
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
Start-Process -FilePath "az" -ArgumentList "extension add --name azure-iot" -Wait

echo "(5/5) $iot_explorer_name"
Start-Process -FilePath "$env:TEMP\$iot_explorer_file" -ArgumentList "/passive" -Wait

echo "`nInstallation complete!"

echo "`nPress any key to continue..."
Read-Host
