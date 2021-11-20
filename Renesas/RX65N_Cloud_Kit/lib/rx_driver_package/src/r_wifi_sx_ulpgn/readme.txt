
r_wifi_sx_ulpgn
=======

Overview
--------------------------------------------------------------------------------
The r_wifi_sx_ulpgn module is a wifi driver.

The queue control blocks can be allocated at compile time or dynamically at run
time. A configuration option for this exists in "r_config\r_wifi_sx_ulpgn_config.h".
An original copy of the configuration file is stored in "r_wifi_sx_ulpgn\ref\
r_wifi_sx_ulpgn_config_reference.h".


Features
--------
* Statically or dynamically allocated queue control blocks.
* Number of queues limited only by the amount of RAM available on the mcu.

File Structure
--------------
r_wifi_sx_ulpgn 
|   readme.txt
|   r_wifi_sx_ulpgn_if.h
|
+---doc
|    +---en
|    |      r01an4664ej{VERSION_NUMBER}-rx.pdf
|    |
|    +---ja
|           r01an4664jj{VERSION_NUMBER}-rx.pdf
|
+---ref
|       r_wifi_sx_ulpgn_reference.h
|
+---src
        r_wifi_sx_ulpgn_api.c
        r_wifi_sx_ulpgn_atcmd.c
        r_wifi_sx_ulpgn_os_wrap.c
        r_wifi_sx_ulpgn_os_wrap.h
        r_wifi_sx_ulpgn_private.h

r_config
    r_wifi_sx_ulpgn_config.h

