RESET_MASK = 0x50021104
RESET_MASK_SYSRSTREQ0_EN = 1 << 4

def will_reset(core, reset_type):
    LOG.info("Enabling SYSRSTREQ0_EN")
    reset_mask = core.read32(RESET_MASK)
    reset_mask |= RESET_MASK_SYSRSTREQ0_EN
    core.write32(RESET_MASK, reset_mask)
