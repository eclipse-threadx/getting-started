OUTPUT_ARCH(rx)
SECTIONS
{
	.fvectors 0xFFFFFFA0 : AT (0xFFFFFFA0)
	{
		KEEP(*(.fvectors))
	}
	
	.text 0xFFF80000 : AT (0xFFF80000)
	{
	    start = .;
		*(.text)
		*(.text.*)
		*(P)
		etext = .;
	}
	.vectors :
	{
	    KEEP(*(.vectors))
	}
	.rvectors :
	{
		_rvectors_start = .;
		KEEP(*(.rvectors))
		_rvectors_end = .;
	}
	.init :
	{
		*(.init)
	 __preinit_array_start = .;
     *(.preinit_array)
     __preinit_array_end = .;
     __init_array_start = (. + 3) & ~ 3;
	      *(.init_array)
    *(SORT(.init_array.*))
     __init_array_end = .;

     __fini_array_start = .;
     *(.fini_array)
     *(SORT(.fini_array.*))
     __fini_array_end = .;
	}
	.fini :
	{
		*(.fini)
	}
	.got :
	{
		*(.got)
		*(.got.plt)
	}
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
		*(C_1)
		*(C_2)
		*(C)
		_erodata = .;
	}
	.eh_frame_hdr :
	{
		*(.eh_frame_hdr)
	}
	.eh_frame :
	{
		*(.eh_frame)
	}
	.jcr :
	{
		*(.jcr)
	}
	.tors :
	{
		__CTOR_LIST__ = .;
		___ctors = .;
		*(.ctors)
		___ctors_end = .;
		__CTOR_END__ = .;
		__DTOR_LIST__ = .;
		___dtors = .;
		*(.dtors)
		___dtors_end = .;
		__DTOR_END__ = .;
		_mdata = .;
	}
	.istack 0x00018000 : AT (0x00018000)
	{
		_istack = .;
	}
	.ustack 0x00017000 : AT (0x00017000)
	{
		_ustack = .;
	}
	.data 0x0 : AT (_mdata)
	{
		_data = .;
		*(.data)
		*(.data.*)
		*(D)
		*(D_1)
		*(D_2)
		_edata = .;
	}
	.gcc_exc :
	{
		*(.gcc_exc)
	}
	.bss :
	{
		_bss = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		*(B)
		*(B_1)
		*(B_2)
		_ebss = .;
		_end = .;
	}
	
}