# Copyright (c) 2021 Linaro
#
# SPDX-License-Identifier: Apache-2.0

# List of all partitions supported by TF-M
# Name must match name in 'trusted-firmware-m/config/config_default.cmake'
set(TFM_VALID_PARTITIONS
  TFM_PARTITION_PROTECTED_STORAGE
  TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
  TFM_PARTITION_CRYPTO
  TFM_PARTITION_INITIAL_ATTESTATION
  TFM_PARTITION_PLATFORM
  TFM_PARTITION_AUDIT_LOG
  )

# Adds trusted-firmware-m as an external project.
# Also creates a target called 'tfm_api'
# which can be linked into the app.
#
# When called from a ThreadX module, the following input values can be provided
# to configure the TF-M build:
#
# BINARY_DIR: The location where the build outputs will be written
# BOARD: The string identifying the board target for TF-M (AN521, etc.)
# CMAKE_BUILD_TYPE: The TF-M build type to use, (Debug, Release, etc.)
# IPC: Build TFM IPC library. This library allows a non-secure application to
#      interface to secure domain using IPC.
# ISOLATION_LEVEL: The TF-M isolation level to use
# REGRESSION: Boolean if TF-M build includes building the TF-M regression tests
# BL2: Boolean if the TF-M build uses MCUboot. Default: True
# ENABLED_PARTITIONS: List of TFM partitions to enable.
#
# Example usage:
#
# trusted_firmware_build(BINARY_DIR ${CMAKE_BINARY_DIR}/tfm
#                        BOARD ${TFM_TARGET_PLATFORM}
#                        CMAKE_BUILD_TYPE Release
#                        IPC
#                        ISOLATION_LEVEL 2
#                        REGRESSION
#                        BL2
#                        BUILD_PROFILE profile_small
#                        ENABLED_PARTITIONS TFM_PARTITION_PLATFORM TFM_PARTITION_CRYPTO)
function(trusted_firmware_build)
  set(options IPC REGRESSION BL2)
  set(oneValueArgs BINARY_DIR BOARD ISOLATION_LEVEL CMAKE_BUILD_TYPE BUILD_PROFILE
    MCUBOOT_IMAGE_NUMBER KEY_FILE_S KEY_FILE_NS PSA_TEST_SUITE)
  set(multiValueArgs ENABLED_PARTITIONS)
  cmake_parse_arguments(TFM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  foreach(partition ${TFM_VALID_PARTITIONS})
    list(FIND TFM_ENABLED_PARTITIONS ${partition} idx)
    if (idx EQUAL -1)
      set(val "OFF")
    else()
      set(val "ON")
    endif()
    list(APPEND TFM_PARTITIONS_ARGS -D${partition}=${val})
  endforeach()

  if(TFM_IPC)
    set(TFM_IPC_ARG -DTFM_PSA_API=ON)
    # PSA API awareness for the Non-Secure application
    target_compile_definitions(mps3_524 PRIVATE "TFM_PSA_API")
  endif()

  if(DEFINED TFM_ISOLATION_LEVEL)
    set(TFM_ISOLATION_LEVEL_ARG -DTFM_ISOLATION_LEVEL=${TFM_ISOLATION_LEVEL})
  endif()

  if(TFM_REGRESSION)
    set(TFM_REGRESSION_ARG -DTEST_NS=ON)
  endif()

  if(DEFINED TFM_CMAKE_BUILD_TYPE)
    set(TFM_CMAKE_BUILD_TYPE_ARG -DCMAKE_BUILD_TYPE=${TFM_CMAKE_BUILD_TYPE})
  else()
    set(TFM_CMAKE_BUILD_TYPE_ARG -DCMAKE_BUILD_TYPE=RelWithDebInfo)
  endif()

  if(DEFINED TFM_BUILD_PROFILE)
    set(TFM_PROFILE_ARG -DTFM_PROFILE=${TFM_BUILD_PROFILE})
  endif()

  if(DEFINED TFM_MCUBOOT_IMAGE_NUMBER)
    set(MCUBOOT_IMAGE_NUM_ARG -DMCUBOOT_IMAGE_NUMBER=${TFM_MCUBOOT_IMAGE_NUMBER})
  endif()

  if(TFM_PSA_TEST_SUITE)
    set(PSA_TEST_ARG -DTEST_PSA_API=${TFM_PSA_TEST_SUITE})
  endif()

  set(TFM_GENERATED_INCLUDES ${TFM_BINARY_DIR}/generated/interface/include)

  if(TFM_BL2)
    set(BL2_BIN_FILE ${TFM_BINARY_DIR}/bin/bl2.bin)
    set(BL2_HEX_FILE ${TFM_BINARY_DIR}/bin/bl2.hex)
  endif()
  set(TFM_S_BIN_FILE ${TFM_BINARY_DIR}/bin/tfm_s.bin)
  set(TFM_S_HEX_FILE ${TFM_BINARY_DIR}/bin/tfm_s.hex)
  set(TFM_NS_BIN_FILE ${TFM_BINARY_DIR}/bin/tfm_ns.bin)
  set(TFM_NS_HEX_FILE ${TFM_BINARY_DIR}/bin/tfm_ns.hex)
  set(TFM_S_SIGNED_BIN_FILE ${TFM_BINARY_DIR}/bin/tfm_s_signed.bin)
  set(TFM_NS_SIGNED_BIN_FILE ${TFM_BINARY_DIR}/bin/tfm_ns_signed.bin)
  set(TFM_S_NS_SIGNED_BIN_FILE ${TFM_BINARY_DIR}/bin/tfm_s_ns_signed.bin)

  if (TFM_PSA_TEST_SUITE)
    set(PSA_TEST_VAL_FILE ${TFM_BINARY_DIR}/app/psa_api_tests/val/val_nspe.a)
    set(PSA_TEST_PAL_FILE ${TFM_BINARY_DIR}/app/psa_api_tests/platform/pal_nspe.a)
    set(COMBINE_DIR_STORAGE storage)
    set(COMBINE_DIR_PROTECTED_STORAGE storage)
    set(COMBINE_DIR_INTERNAL_TRUSTED_STORAGE storage)
    set(COMBINE_DIR_CRYPTO crypto)
    set(COMBINE_DIR_INITIAL_ATTESTATION initial_attestation)
    set(PSA_TEST_COMBINE_FILE ${TFM_BINARY_DIR}/app/psa_api_tests/dev_apis/${COMBINE_DIR_${TFM_PSA_TEST_SUITE}}/test_combine.a)
  endif()

  set(VENEERS_FILE ${TFM_BINARY_DIR}/install/interface/lib/s_veneers.o)
  set(PSA_API_NS_PATH ${TFM_BINARY_DIR}/app/libtfm_api_ns.a)

  if (TFM_REGRESSION)
    set(PSA_QCBOR_NS_REGRESSION_TEST_PATH ${TFM_BINARY_DIR}/test/suites/qcbor/libtfm_test_suite_qcbor_ns.a)
    set(PSA_NS_CORE_TEST_PATH ${TFM_BINARY_DIR}/test/suites/core/libtfm_test_suite_core_ns.a)
    set(PSA_NS_IPC_TEST_PATH ${TFM_BINARY_DIR}/test/suites/ipc/libtfm_test_suite_ipc_ns.a)
    set(PSA_ATTESTATION_NS_REGRESSION_TEST_PATH ${TFM_BINARY_DIR}/test/suites/attestation/libtfm_test_suite_attestation_ns.a)
    set(PSA_CRYPTO_NS_REGRESSION_TEST_PATH ${TFM_BINARY_DIR}/test/suites/crypto/libtfm_test_suite_crypto_ns.a)
    set(PSA_PLATFORM_NS_REGRESSION_TEST_PATH ${TFM_BINARY_DIR}/test/suites/platform/libtfm_test_suite_platform_ns.a)
    set(PSA_PS_NS_REGRESSION_TEST_PATH ${TFM_BINARY_DIR}/test/suites/ps/libtfm_test_suite_ps_ns.a)
    set(PSA_ITS_NS_REGRESSION_TEST_PATH ${TFM_BINARY_DIR}/test/suites/its/libtfm_test_suite_its_ns.a)
    set(PSA_T_COSE_NS_REGRESSION_TEST_PATH ${TFM_BINARY_DIR}/test/suites/t_cose/libtfm_test_suite_t_cose_ns.a)
    set(PSA_NS_REGRESSION_TEST_PATH ${TFM_BINARY_DIR}/app/libtfm_ns_integration_test.a)
    set(PSA_QCBOR_TEST_PATH ${TFM_BINARY_DIR}/lib/ext/qcbor/libtfm_qcbor_test.a)
    set(PSA_T_COSE_TEST_PATH ${TFM_BINARY_DIR}/lib/ext/t_cose/libtfm_t_cose_test.a)
    set(PSA_QCBOR_PATH ${TFM_BINARY_DIR}/lib/ext/qcbor/libtfm_qcbor.a)
    set(PSA_T_COSE_PATH ${TFM_BINARY_DIR}/lib/ext/t_cose/libtfm_t_cose.a)
  endif()

  if(TFM_REGRESSION OR TFM_PSA_TEST_SUITE)
    set(PSA_PLATFORM_NS_PATH ${TFM_BINARY_DIR}/platform/libplatform_ns.a)
  endif()

  set(BUILD_BYPRODUCTS
    ${VENEERS_FILE}
    ${PSA_API_NS_PATH}
    ${PSA_QCBOR_NS_REGRESSION_TEST_PATH}
    ${PSA_NS_CORE_TEST_PATH}
    ${PSA_NS_IPC_TEST_PATH}
    ${PSA_ATTESTATION_NS_REGRESSION_TEST_PATH}
    ${PSA_CRYPTO_NS_REGRESSION_TEST_PATH}
    ${PSA_PLATFORM_NS_REGRESSION_TEST_PATH}
    ${PSA_PS_NS_REGRESSION_TEST_PATH}
    ${PSA_ITS_NS_REGRESSION_TEST_PATH}
    ${PSA_T_COSE_NS_REGRESSION_TEST_PATH}
    ${PSA_NS_REGRESSION_TEST_PATH}
    ${PSA_QCBOR_TEST_PATH}
    ${PSA_T_COSE_TEST_PATH}
    ${PSA_PLATFORM_NS_PATH}
    ${PSA_QCBOR_PATH}
    ${PSA_T_COSE_PATH}
    ${TFM_GENERATED_INCLUDES}/psa_manifest/sid.h
    ${PSA_TEST_VAL_FILE}
    ${PSA_TEST_PAL_FILE}
    ${PSA_TEST_COMBINE_FILE}
    ${BL2_BIN_FILE}
    ${BL2_HEX_FILE}
    ${TFM_S_BIN_FILE}
    ${TFM_S_HEX_FILE}
    ${TFM_NS_BIN_FILE}
    ${TFM_NS_HEX_FILE}
    ${TFM_S_SIGNED_BIN_FILE}
    ${TFM_NS_SIGNED_BIN_FILE}
    ${TFM_S_NS_SIGNED_BIN_FILE}
    )

  set(TFM_TOOLCHAIN_FILE "trusted-firmware-m/toolchain_GNUARM.cmake")
  set(TFM_TOOLCHAIN_PREFIX "arm-none-eabi")
  set(TFM_TOOLCHAIN_PATH ${ARM_TOOLCHAIN_PATH})

  include(ExternalProject)

  set(TX_TFM_BASE_DIR ${GSG_BASE_DIR}/core/lib/tfm)
  set(TX_TFM_TESTS_DIR ${GSG_BASE_DIR}/core/lib/tf-m-tests)

  ExternalProject_Add(
    tfm
    SOURCE_DIR ${TX_TFM_BASE_DIR}
    BINARY_DIR ${TFM_BINARY_DIR}
    CMAKE_ARGS -DTFM_TOOLCHAIN_FILE=${TX_TFM_BASE_DIR}/toolchain_GNUARM.cmake
               -DTFM_PLATFORM=${TFM_BOARD}
               -DCROSS_COMPILE=${TFM_TOOLCHAIN_PATH}/${TFM_TOOLCHAIN_PREFIX}
               ${TFM_CMAKE_BUILD_TYPE_ARG}
               -DBL2=${TFM_BL2}
               ${TFM_IPC_ARG}
               ${TFM_ISOLATION_LEVEL_ARG}
               ${TFM_REGRESSION_ARG}
               ${TFM_PROFILE_ARG}
               ${MCUBOOT_IMAGE_NUM_ARG}
               ${PSA_TEST_ARG}
               -DTFM_TEST_REPO_PATH=${TX_TFM_TESTS_DIR}
               ${TFM_PARTITIONS_ARGS}
    BUILD_ALWAYS True
    USES_TERMINAL_BUILD True
    BUILD_BYPRODUCTS ${BUILD_BYPRODUCTS}
  )

  # Set BL2 (MCUboot) executable file paths as target properties on 'tfm'
  # These files are produced by the TFM build system.
  if(TFM_BL2)
    set_target_properties(tfm PROPERTIES
      BL2_BIN_FILE ${BL2_BIN_FILE}
      BL2_HEX_FILE ${BL2_HEX_FILE}
      )
  endif()

  # Set TFM S/NS executable file paths as target properties on 'tfm'
  # These files are produced by the TFM build system.
  # Note that the Nonsecure FW is replaced by the ThreadX app in regular ThreadX
  # builds.
  set_target_properties(tfm PROPERTIES
    TFM_S_BIN_FILE ${TFM_S_BIN_FILE} # TFM Secure FW (unsigned)
    TFM_S_HEX_FILE ${TFM_S_HEX_FILE} # TFM Secure FW (unsigned)
    TFM_NS_BIN_FILE ${TFM_NS_BIN_FILE} # TFM Nonsecure FW (unsigned)
    TFM_NS_HEX_FILE ${TFM_NS_HEX_FILE} # TFM Nonsecure FW (unsigned)
    TFM_S_SIGNED_BIN_FILE ${TFM_S_SIGNED_BIN_FILE} # TFM Secure FW (signed)
    TFM_NS_SIGNED_BIN_FILE ${TFM_NS_SIGNED_BIN_FILE} # TFM Nonsecure FW (signed)
    TFM_S_NS_SIGNED_BIN_FILE ${TFM_S_NS_SIGNED_BIN_FILE} # Merged TFM Secure/Nonsecure FW (signed)
    )

  add_library(tfm_api INTERFACE
    # ${TX_TFM_TESTS_DIR}/app/os_wrapper_cmsis_rtos_v2.c
  )

  target_include_directories(tfm_api
    # PRIVATE
    # ${TX_TFM_TESTS_DIR}/CMSIS/RTOS2/Include
    INTERFACE
    ${TX_TFM_BASE_DIR}/interface/include
    ${TX_TFM_BASE_DIR}/../tf-m-tests/test/framework
    # INTERFACE
    ${TFM_GENERATED_INCLUDES}
  )

  target_link_libraries(tfm_api
    INTERFACE
    ${PSA_NS_CORE_TEST_PATH}
    ${PSA_ATTESTATION_NS_REGRESSION_TEST_PATH}
    ${PSA_CRYPTO_NS_REGRESSION_TEST_PATH}
    ${PSA_NS_IPC_TEST_PATH}
    ${PSA_ITS_NS_REGRESSION_TEST_PATH}
    ${PSA_PLATFORM_NS_REGRESSION_TEST_PATH}
    ${PSA_PS_NS_REGRESSION_TEST_PATH}
    ${PSA_QCBOR_NS_REGRESSION_TEST_PATH}
    ${PSA_T_COSE_NS_REGRESSION_TEST_PATH}
    ${PSA_QCBOR_TEST_PATH}
    ${PSA_T_COSE_TEST_PATH}
    ${PSA_NS_REGRESSION_TEST_PATH}
    ${PSA_QCBOR_PATH}
    ${PSA_T_COSE_PATH}
    ${PSA_TEST_VAL_FILE}
    ${PSA_TEST_PAL_FILE}
    ${PSA_TEST_COMBINE_FILE}
    ${PSA_PLATFORM_NS_PATH}
    ${PSA_API_NS_PATH}
    ${VENEERS_FILE}
    # $<TARGET_FILE:tfm_api>
  )

  add_dependencies(tfm_api tfm)

  set(PREPROCESSED_FILE_S "${TFM_BINARY_DIR}/bl2/ext/mcuboot/CMakeFiles/signing_layout_s.dir/signing_layout_s_ns.o")
  set(PREPROCESSED_FILE_NS ${PREPROCESSED_FILE_S})
  set(TFM_IMAGE_VERSION_S "1.3.0")
  set(TFM_IMAGE_VERSION_NS "0.0.1")
  set(HEADER_SIZE "0x400")
  set(TFM_PUBLIC_KEY_FORMAT "full")

  function(tfm_combine OUT_ARG INPUT_S_FILE INPUT_NS_FILE OUTPUT_FILE)
    set (${OUT_ARG}
      ${PYTHON_EXECUTABLE} ${TX_TFM_BASE_DIR}/bl2/ext/mcuboot/scripts/assemble.py
      --layout ${PREPROCESSED_FILE_S}
      -s ${INPUT_S_FILE}
      -n ${INPUT_NS_FILE}
      -o ${OUTPUT_FILE}
      PARENT_SCOPE
    )
  endfunction()

  function(tfm_sign OUT_ARG PAD INPUT_FILE OUTPUT_FILE)
    if(PAD)
      set(pad_args --pad --pad-header)
    else()
      set(pad_args --pad-header)
    endif()
    set (${OUT_ARG}
      ${PYTHON_EXECUTABLE} ${TX_TFM_BASE_DIR}/bl2/ext/mcuboot/scripts/wrapper/wrapper.py
      --layout ${PREPROCESSED_FILE_S}
      -k ${TFM_KEY_FILE_S}
      --public-key-format ${TFM_PUBLIC_KEY_FORMAT}
      --align 1
      -v ${TFM_IMAGE_VERSION_S}
      ${pad_args}
      -s auto
      -H ${HEADER_SIZE}
      ${INPUT_FILE}
      ${OUTPUT_FILE}
      PARENT_SCOPE)
  endfunction()

  set(S_NS_SIGNED_FILE ${CMAKE_BINARY_DIR}/tfm_s_ns_signed.bin)
  set(S_NS_FILE ${CMAKE_BINARY_DIR}/tfm_s_ns.bin)

  tfm_combine(combine_cmd $<TARGET_PROPERTY:tfm,TFM_S_BIN_FILE> ${CMAKE_BINARY_DIR}/app/mps3_524.bin ${S_NS_FILE})

  tfm_sign(sign_cmd TRUE ${S_NS_FILE} ${S_NS_SIGNED_FILE})

  if (TFM_BL2)
    add_custom_command(
      TARGET mps3_524
      POST_BUILD
      COMMAND ${TFM_TOOLCHAIN_PATH}/${TFM_TOOLCHAIN_PREFIX}-objcopy -O binary ${CMAKE_BINARY_DIR}/app/mps3_524.elf ${CMAKE_BINARY_DIR}/app/mps3_524.bin
      COMMAND ${combine_cmd}
      COMMAND ${sign_cmd}
    )
  endif()

endfunction()
