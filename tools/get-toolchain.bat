:: Copyright (c) Microsoft Corporation.
:: Licensed under the MIT License.

@echo off

PowerShell.exe -NoProfile -Command "& {Start-Process PowerShell.exe -ArgumentList '-NoProfile -ExecutionPolicy Bypass -File ""%~dpn0.ps1""' -Verb RunAs}"

::pause
