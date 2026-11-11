/** @file
  OVMF Platform definitions

  Copyright (C) 2015, Red Hat, Inc.
  Copyright (c) 2014, Gabriel L. Somlo <somlo@cmu.edu>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __OVMF_PLATFORMS_H__
#define __OVMF_PLATFORMS_H__

#include <Library/PciLib.h>
#include <IndustryStandard/Pci22.h>
#include <IndustryStandard/Q35MchIch9.h>
#include <IndustryStandard/I440FxPiix4.h>
#include <IndustryStandard/Bhyve.h>
#include <IndustryStandard/Microvm.h>
#include <IndustryStandard/CloudHv.h>

//
// OVMF Host Bridge DID Address
//
#define OVMF_HOSTBRIDGE_DID \
  PCI_LIB_ADDRESS (0, 0, 0, PCI_DEVICE_ID_OFFSET)

//
// OVMF-PLATFORM-PORTABILITY: one firmware for every host bridge.
//
// These DIDs have *dedicated* OVMF code paths and pass through unchanged.
// EVERY other DID -- VMware 440BX, another hypervisor, real silicon, a
// chipset that has not been invented yet -- falls back to the conservative
// i440FX/PIIX4-family path.  This is a fallback, not an allowlist: adding
// support for a new machine requires no source edit, no patch and no
// separate build.  The PM registers are resolved from the real device by
// PCI class code (see I440FxPiix4.h), so the fallback is functionally
// correct rather than merely "does not ASSERT".
//
// VID/DID are read-only in config space, so this cannot be done by a
// bootloader at runtime; it belongs in the firmware.
//
STATIC
inline
UINT16
OvmfHostBridgeDevId (
  VOID
  )
{
  UINT16  DeviceId;

  DeviceId = PciRead16 (OVMF_HOSTBRIDGE_DID);
  switch (DeviceId) {
    case INTEL_82441_DEVICE_ID:      // QEMU i440FX / generic fallback value
    case INTEL_Q35_MCH_DEVICE_ID:    // QEMU Q35 / ICH9
    case CLOUDHV_DEVICE_ID:
    case MICROVM_PSEUDO_DEVICE_ID:
    case 0xffff:                     // microvm
    case 0x7432:                     // bhyve (AMD host bridge)
    case 0x1275:                     // bhyve (Intel host bridge)
      return DeviceId;
    default:
      return INTEL_82441_DEVICE_ID;
  }
}

//
// Values we program into the PM base address registers
//
#define PIIX4_PMBA_VALUE   0xB000
#define ICH9_PMBASE_VALUE  0x0600

//
// Common bits in same-purpose registers
//
#define PMBA_RTE  BIT0

//
// Common IO ports relative to the Power Management Base Address
//
#define ACPI_TIMER_OFFSET  0x8

#endif
