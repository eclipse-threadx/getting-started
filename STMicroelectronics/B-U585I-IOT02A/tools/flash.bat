:: Copyright (c) Microsoft Corporation.
:: Licensed under the MIT License.

setlocal
cd /d %~dp0\..

openocd -f interface/stlink.cfg -f target/stm32u5x.cfg -c "program build/app/stm32u585_azure_iot.elf verify" -c "reset halt" -c "shutdown"