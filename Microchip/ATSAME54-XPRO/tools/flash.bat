:: Copyright (c) Microsoft Corporation.
:: Licensed under the MIT License.

setlocal
cd /d %~dp0\..

openocd -f board/microchip_same54_xplained_pro.cfg -c "program build/app/atsame54_azure_iot.elf verify reset exit"