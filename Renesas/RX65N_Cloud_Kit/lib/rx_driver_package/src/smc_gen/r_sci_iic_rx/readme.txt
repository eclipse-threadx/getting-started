PLEASE REFER TO THE APPLICATION NOTE FOR THIS MIDDLEWARE FOR MORE INFORMATION

r_sci_iic_rx
=========

Overview
--------
The SCI simple I2C mode fit module provides a method to transmit and receive data between the master and slave devices using the serial communications interface (SCI). The SCI simple I2C mode is in compliance with single master mode of the XP I2C-bus (Inter-IC-Bus) interface.

The driver supports three channels available on the mcu.
The driver can be reducedin size by removing code used for parameter checking orfor unused channels.
These configuration options can be found in "r_config\r_sci_iic_rx_config.h".
An original copy of the configuration file is stored in 
"r_sci_iic_rx\ref\r_sci_iic_rx_config_reference.h".

Features
--------
* Single master mode (slave transmission or slave reception is not supported).
* Circuit to generate waveforms for conditions.
* Communication mode can be normal or fast mode and the maximum communication rate is 384 kbps.

File Structure
--------------
r_sci_iic_rx
|   readme.txt
|   r_sci_iic_rx_if.h
|
+---doc
|   |
|   +---en
|   |   r01an1691ej{VERSION_NUMBER}-rx-sci-iic.pdf
|   |
|   +---ja
|       r01an1691jj{VERSION_NUMBER}-rx-sci-iic.pdf
|
+---ref
|       r_sci_iic_rx_config_reference.h
|       r_sci_iic_rx_pin_config_reference.h
|
+---src
    |   r_sci_iic_rx.c
    |   r_sci_iic_rx_platform.h
    |   r_sci_iic_rx_private.h
    |
    +---targets
        |
        +---rx110
        |       r_sci_iic_rx110.c
        |       r_sci_iic_rx110_private.h
        |
        +---rx111
        |       r_sci_iic_rx111.c
        |       r_sci_iic_rx111_private.h
        |
        +---rx113
        |       r_sci_iic_rx113.c
        |       r_sci_iic_rx113_private.h
        |
        +---rx130
        |       r_sci_iic_rx130.c
        |       r_sci_iic_rx130_private.h
        |
        +---rx13t
        |       r_sci_iic_rx13t.c
        |       r_sci_iic_rx13t_private.h
        |
        +---rx230
        |       r_sci_iic_rx230.c
        |       r_sci_iic_rx230_private.h
        |
        +---rx231
        |       r_sci_iic_rx231.c
        |       r_sci_iic_rx231_private.h
        |
        +---rx23e-a
        |       r_sci_iic_rx23e_a.c
        |       r_sci_iic_rx23e_a_private.h
        |
        +---rx23t
        |       r_sci_iic_rx23t.c
        |       r_sci_iic_rx23t_private.h
        |
        +---rx23w
        |       r_sci_iic_rx23w.c
        |       r_sci_iic_rx23w_private.h
        |
        +---rx24t
        |        r_sci_iic_rx24t.c
        |        r_sci_iic_rx24t_private.h
        |
        +---rx24u
        |        r_sci_iic_rx24u.c
        |        r_sci_iic_rx24u_private.h
        |
        +---rx64m
        |       r_sci_iic_rx64m.c
        |       r_sci_iic_rx64m_private.h
        |
        +---rx65n
        |       r_sci_iic_rx65n.c
        |       r_sci_iic_rx65n_private.h
        |
        +---rx66t
        |       r_sci_iic_rx66t.c
        |       r_sci_iic_rx66t_private.h
        |
        +---rx66n
        |       r_sci_iic_rx66n.c
        |       r_sci_iic_rx66n_private.h
        |
        +---rx71m
        |       r_sci_iic_rx71m.c
        |       r_sci_iic_rx71m_private.h
        |
        +---rx72t
        |       r_sci_iic_rx72t.c
        |       r_sci_iic_rx72t_private.h
        |
        +---rx72m 
        |       r_sci_iic_rx72m.c
        |       r_sci_iic_rx72m_private.h
        |
        +---rx72n 
                r_sci_iic_rx72n.c
                r_sci_iic_rx72n_private.h

r_config
    r_sci_iic_rx_config.h
    r_sci_iic_rx_pin_config.h

