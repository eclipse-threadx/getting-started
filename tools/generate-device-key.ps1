# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

param(
    [Parameter(Mandatory=$true)][string]$key, 
    [Parameter(Mandatory=$true)][string]$id
)

$hmacsha = New-Object System.Security.Cryptography.HMACSHA256
$hmacsha.key = [Convert]::FromBase64String($key)

$deviceKey = $hmacsha.ComputeHash([Text.Encoding]::ASCII.GetBytes($id))
$deviceKey = [Convert]::ToBase64String($deviceKey)

Write-Output "Device key: $deviceKey"
