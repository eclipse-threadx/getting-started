# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

param(
    [Parameter(Mandatory=$true)][string]$group_sas_key, 
    [Parameter(Mandatory=$true)][string]$device_id
)

$hmacsha = New-Object System.Security.Cryptography.HMACSHA256
$hmacsha.key = [Convert]::FromBase64String($group_sas_key)

$deviceKey = $hmacsha.ComputeHash([Text.Encoding]::ASCII.GetBytes($device_id))
$deviceKey = [Convert]::ToBase64String($deviceKey)

Write-Output "Device key: $deviceKey"
