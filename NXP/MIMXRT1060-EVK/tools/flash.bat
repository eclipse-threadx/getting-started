:: Copyright (c) Microsoft Corporation.
:: Licensed under the MIT License.

setlocal
cd /d %~dp0\..

c:\OpenOCD\bin\openocd -f tools\nxp_mimxrt1060-evk.cfg -c init -c "program _build/app/azure_iot_sample"
