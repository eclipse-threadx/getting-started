                        Express Logic's ThreadX for RX6xx 

                            Using the KPIT/GNU Tools

1.  Installation

ThreadX for the Renesas RX6xx is delivered on a single CD-ROM compatible disk. 
The entire distribution can be found in the sub-directory:

\threadx

To install ThreadX to your hard-disk, either run the supplied installer 
program Setup.exe or copy the distribution from the CD manually. 

To copy the ThreadX distribution manually, make a ThreadX directory on your 
hard-disk (we recommend c:\threadx\rx6xx\gnu) and copy all the contents 
of the ThreadX sub-directory on the distribution disk. The following 
is an example MS-DOS copy command from the distribution directory
(assuming source is d: and c: is your hard-drive):

d:\threadx> xcopy /S *.* c:\threadx\rx6xx\gnu


2.  Building the ThreadX run-time Library

Open a DOS prompt from the KPIT GNU installation: 

Windows menu -> All programs -> GNURXv12.01-ELF -> rx-elf Toolchain

Navigate to the ThreadX installation directory.

Run the command script from the ThreadX installation directory:

build_threadx.bat

The resulting library file tx.a will be created for use with all applications using ThreadX.


3.  Demonstration System

The ThreadX demonstration is designed to execute on the RX62N YRDK board. 
The instructions that follow describe how to get the ThreadX 
evaluation running.

Open a DOS prompt from the KPIT GNU installation: 

Windows menu -> All programs -> GNURXv12.01-ELF -> rx-elf Toolchain

Navigate to the ThreadX installation directory.

Run the command script from the ThreadX installation directory::

build_threadx_demo.bat

The linker script is in the file demo_threadx.ld.
Once the demo application is built, download the file to the target by right using HEW.

Notes on the runtime environment:
The supplied Start.S will initialize the KPIT GNU C runtime environment.
The code that configures the INTB has been commented out. INTB is now set up
inside tx_initialize_low_level.S.
Low level hardware routines can be called from Start.S
The exception tables are now all contained within tx_initialize_low_level.S


4.  System Initialization

The system entry point using the KPIT GNU tools is at the label _PowerON_Reset. 

The vector area is setup in the file tx_initialize_low_level.S. This file is also 
responsible for setting up various system data structures, interrupt vectors, and 
the periodic timer interrupt. This file is also an ideal place to add additional hardware 
initialization code.

The ThreadX demonstration for the RX6xx utilizes CMT0 as a periodic timer interrupt 
source. The CMT0 interrupt is typically setup for 10ms periodic interrupts and the 
interrupt priority level is set to level 7. You may change any of the timer 
parameters as needed. Increasing the timer interrupt frequency increases the overhead
of the timer handling code on the system.

In addition, _tx_initialize_low_level determines the first available address for use 
by the application, which is supplied as the sole input parameter to your application 
definition function, tx_application_define. The first available memory is determined 
by the location of the '_end' label the is defined in the linker script.
'_end' should reference the first memory AFTER all other RAM 
sections in your linker control file.


5.  Register Usage and Stack Frames

The Renesas EABI for RX6xx assumes that general purpose registers R1-R5 and R14 and R15 are 
scratch registers for each function.  All other registers used by a C function must be 
preserved by the function. ThreadX takes advantage of this in situations where a context 
switch happens as a result of making a ThreadX service call (which is itself a C function). 
In such cases, the saved context of a thread is only the non-scratch registers.

The following defines the saved context stack frames for context switches that occur as a 
result of interrupt handling or from thread-level API calls. All suspended threads have 
one of these two types of stack frames. The top of the suspended thread's stack is pointed 
to by tx_thread_stack_ptr in the associated thread control block TX_THREAD.


    Offset        Interrupted Stack Frame        Non-Interrupt Stack Frame

     0x00                   1                           0
     0x04                   R6                          PSW
     0x08                   R7                          R6
     0x0C                   R8                          R7
     0x10                   R9                          R8
     0x14                   R10                         R9
     0x18                   R11                         R10
     0x1C                   R12                         R11
     0x20                   R13                         R12
     0x24                   FPSW                        R13
     0x28                   R14                         PC - return address
     0x2C                   R15
     0x30                   R3
     0x34                   R4
     0x38                   R5
     0x3C                   R1
     0x40                   R2
     0x44                   PC - return address
     0x48                   PSW
     
Note: saving and restoring the ACC register inside ISRs is currently not supported,
if your application requires the ACC register, contact support@expresslogic.com
     
6.  Improving Performance

The distribution version of ThreadX is built without any compiler optimizations.  This 
makes it easy to debug because you can trace or set breakpoints inside of ThreadX itself.  
Of course, this costs some performance. To make ThreadX run faster, you can change the 
ThreadX Library project to disable debug information and enable the desired optimizations.  

In addition, you can eliminate the ThreadX basic API error checking by compiling your 
application code with the symbol TX_DISABLE_ERROR_CHECKING defined before tx_api.h 
is included. 


7.  Interrupt Handling

ThreadX provides complete and high-performance interrupt handling for RX6xx targets.  
There are a certain set of requirements that are defined in the following sub-sections:

This is an example of an interrupt service routine:

.global __tx_initialize_sample_ISR
__tx_initialize_sample_ISR:              ; insert this label in the vector table
    PUSHM R1-R2
    BSR __tx_thread_context_save         ; Threadx RX6xx ISR prologue 

    ; Your ISR processing here!
    ; interrupts may be enabled here or anytime in the ISR with SETPSW I
    ; BSR C_function_label

    BRA __tx_thread_context_restore     ; Threadx RX6xx ISR epilogue

Note(s): 

    1) Enabling interrupts after the call to __tx_thread_context_save is permitted to 
       enable interrupt nesting.

    2) Interrupts do not save the ACC register at this time, if this is an issue for 
       your application, contact Express Logic.

    3) High Speed interrupts are not supported at this time

    4) Do not use the compiler pragmas to set up interrupt service routine prologue 
       and epilogue

8. Execution Profiling

The RX port adds support for the Execution Profiling Kit (EPK). The EPK consists 
of the files tx_execution_profile.c and tx_execution_profile.h. See the documentation 
of the EPK for generic usage details.

To add the EPK to your RX6xx release make the following modifications:

* Enable the following define for both the Threadx library and the application
TX_ENABLE_EXECUTION_CHANGE_NOTIFY

* in tx_port.h, change around line 183
change #define TX_THREAD_EXTENSION_3     
into #include "tx_execution_profile.h"   

To use CMT1 as a free running 16 bit timer as TX_EXECUTION_TIME_SOURCE:

Enable CMT1 by enabling its bit in tx_initalize_low_level: 
Around line 152 change#1, to #3, to enable both CMT0 for the timer tick and CMT1 as a freerunning timer
    MOV.L #3, r1
    MOV.W  r1, [R2]                             ; start counter

In tx_execution_profile.h, change following around line 74:

#ifdef TX_EXECUTION_64BIT_TIME
typedef unsigned long long              EXECUTION_TIME;
#define TX_THREAD_EXTENSION_3           unsigned long long  tx_thread_execution_time_total; \
                                        unsigned long long  tx_thread_execution_time_last_start; 
#define TX_EXECUTION_MAX_TIME_SOURCE    0xFFFFFFFFFFFFFFFF
#else
typedef unsigned long                   EXECUTION_TIME;
#define TX_THREAD_EXTENSION_3           unsigned long   tx_thread_execution_time_total; \
                                        unsigned long   tx_thread_execution_time_last_start; 
#define TX_EXECUTION_MAX_TIME_SOURCE    0xFFFF
#endif
                                        
/* Define basic constants for the execution profile kit.  */

#define TX_EXECUTION_TIME_SOURCE         (EXECUTION_TIME)  *((USHORT *) 0x8800A)

Rebuild the Threadx library and the application.
Refer to the EPK documentation how to interpret the results.


9.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:


01/10/2013  ThreadX update of RX6xx/GNU port. The following files were 
            changed/added for port specific version 5.1:
 
            tx_port.h                           Updated version string. 
            tx_thread_system_return.S           Added support for EPK.
            tx_thread_schedule.S                Added support for EPK.
            tx_thread_context_save.S            Added support for EPK.
            tx_thread_context_restore.S         Added support for EPK.
            *.s                                 Modified comment(s).

06/04/2012  Initial ThreadX release for the RX6xx using GNU tools, version 5.0.


Copyright(c) 1996-2013 Express Logic, Inc.


Express Logic, Inc.
11423 West Bernardo Court
San Diego, CA  92127

www.expresslogic.com