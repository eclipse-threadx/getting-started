:: Copyright (c) Microsoft Corporation.
:: Licensed under the MIT License.

setlocal
cd /d %~dp0\..

openocd -f tools\nxp_mimxrt1060-evk.cfg -c init -c "program build/app/mimxrt1060_azure_iot.elf verify reset exit"
