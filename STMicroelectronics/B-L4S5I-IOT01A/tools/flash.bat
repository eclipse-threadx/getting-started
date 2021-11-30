:: Copyright (c) Microsoft Corporation.
:: Licensed under the MIT License.

setlocal
cd /d %~dp0\..

openocd -f board/stm32l4discovery.cfg -c "program build/app/stm32l4s5_azure_iot.elf verify reset exit"