# Copyright (c) 2021 Linaro Limited.
# Licensed under the MIT License.

set(TX_TFM_BASE_DIR ${GSG_BASE_DIR}/core/lib/tfm)
set(TX_TFM_BL2_DIR ${TX_TFM_BASE_DIR}/bl2/ext/mcuboot)

option(BUILD_WITH_TFM "Build with TF-M as the Secure Execution Environment" ON)
include(CMakeDependentOption)
set(TFM_KEY_FILE_S
    "${TX_TFM_BL2_DIR}/root-RSA-3072.pem"
    CACHE FILEPATH
    "The path and filename for the .pem file containing the private key
     that should be used by the BL2 bootloader when signing secure
     firmware images."
)

set(TFM_KEY_FILE_NS
    "${TX_TFM_BL2_DIR}/root-RSA-3072_1.pem"
    CACHE FILEPATH
    "The path and filename for the .pem file containing the private key
     that should be used by the BL2 bootloader when signing non-secure
     firmware images."
)

set(TFM_PROFILE
    OFF
    CACHE STRING
    "The build profile used for TFM Secure image."
)

set(TFM_ISOLATION_LEVEL
    "1"
    CACHE STRING
    "Manually set the required TFM isolation level. Possible values are
     1,2 or 3; the default is set by build configuration."
)

cmake_dependent_option(TFM_BL2
    "TFM is designed to run with MCUboot in a certain configuration.
     This config adds MCUboot to the build - built via TFM's build system."
    ON
    BUILD_WITH_TFM OFF
)

set(TFM_MCUBOOT_IMAGE_NUMBER
    "1"
    CACHE STRING
    "How many images the bootloader sees when it looks at TFM and the app.
     When this is 1, the S and NS are considered as 1 image and must be
     updated in one atomic operation. When this is 2, they are split and
     can be updated independently if dependency requirements are met."
)

cmake_dependent_option(TFM_PARTITION_PROTECTED_STORAGE
    "Setting this option will cause '-DTFM_PARTITION_PROTECTED_STORAGE'
     to be passed to the TF-M build system. Look at 'config_default.cmake'
     in the trusted-firmware-m repository for details regarding this
     parameter. Any dependencies between the various TFM_PARTITION_*
     options are handled by the build system in the trusted-firmware-m
     repository."
    ON
    BUILD_WITH_TFM OFF
)

cmake_dependent_option(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    "Setting this option will cause '-DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE'
     to be passed to the TF-M build system. Look at 'config_default.cmake'
     in the trusted-firmware-m repository for details regarding this
     parameter. Any dependencies between the various TFM_PARTITION_*
     options are handled by the build system in the trusted-firmware-m
     repository."
    ON
    BUILD_WITH_TFM OFF
)

cmake_dependent_option(TFM_PARTITION_CRYPTO
    "Setting this option will cause '-DTFM_PARTITION_CRYPTO'
     to be passed to the TF-M build system. Look at 'config_default.cmake'
     in the trusted-firmware-m repository for details regarding this
     parameter. Any dependencies between the various TFM_PARTITION_*
     options are handled by the build system in the trusted-firmware-m
     repository."
    ON
    BUILD_WITH_TFM OFF
)

cmake_dependent_option(TFM_PARTITION_INITIAL_ATTESTATION
    "Setting this option will cause '-DTFM_PARTITION_INITIAL_ATTESTATION'
     to be passed to the TF-M build system. Look at 'config_default.cmake'
     in the trusted-firmware-m repository for details regarding this
     parameter. Any dependencies between the various TFM_PARTITION_*
     options are handled by the build system in the trusted-firmware-m
     repository."
    ON
    BUILD_WITH_TFM OFF
)

cmake_dependent_option(TFM_PARTITION_PLATFORM
    "Setting this option will cause '-DTFM_PARTITION_PLATFORM'
     to be passed to the TF-M build system. Look at 'config_default.cmake'
     in the trusted-firmware-m repository for details regarding this
     parameter. Any dependencies between the various TFM_PARTITION_*
     options are handled by the build system in the trusted-firmware-m
     repository."
    ON
    BUILD_WITH_TFM OFF
)

cmake_dependent_option(TFM_PARTITION_AUDIT_LOG
    "Setting this option will cause '-DTFM_PARTITION_AUDIT_LOG'
     to be passed to the TF-M build system. Look at 'config_default.cmake'
     in the trusted-firmware-m repository for details regarding this
     parameter. Any dependencies between the various TFM_PARTITION_*
     options are handled by the build system in the trusted-firmware-m
     repository."
    ON
    BUILD_WITH_TFM OFF
)

cmake_dependent_option(TFM_PARTITION_FIRMWARE_UPDATE
    "Setting this option will cause '-DTFM_PARTITION_FIRMWARE_UPDATE'
     to be passed to the TF-M build system. Look at 'config_default.cmake'
     in the trusted-firmware-m repository for details regarding this
     parameter. Any dependencies between the various TFM_PARTITION_*
     options are handled by the build system in the trusted-firmware-m
     repository."
    On
    BUILD_WITH_TFM OFF
)

cmake_dependent_option(TFM_IPC
    "When enabled, this option signifies that the TF-M build supports
     the PSA API (IPC mode) instead of the secure library mode."
    ON
    BUILD_WITH_TFM OFF
)

cmake_dependent_option(TFM_REGRESSION
    "When enabled, this option signifies that the TF-M build includes
     the Secure and the Non-Secure regression tests."
    ON
    BUILD_WITH_TFM OFF
)

set(APP_CONFIG OFF CACHE FILEPATH
    "Configuration file for the MPS3 AN524 example app"
)

if (APP_CONFIG)
    include(${APP_CONFIG})
endif()
