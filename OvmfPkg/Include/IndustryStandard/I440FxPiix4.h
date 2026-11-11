/** @file
  Various register numbers and value bits based on the following publications:
  - Intel(R) datasheet 290549-001
  - Intel(R) datasheet 290562-001
  - Intel(R) datasheet 297654-006
  - Intel(R) datasheet 297738-017

  Copyright (C) 2015, Red Hat, Inc.
  Copyright (c) 2014, Gabriel L. Somlo <somlo@cmu.edu>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef __I440FX_PIIX4_H__
#define __I440FX_PIIX4_H__

#include <Library/PciLib.h>

//
// Host Bridge Device ID (DID) value for I440FX
//
#define INTEL_82441_DEVICE_ID  0x1237

//
// B/D/F/Type: 0/0/0/PCI
//
#define PMC_REGISTER_PIIX4(Offset)  PCI_LIB_ADDRESS (0, 0, 0, (Offset))

#define PIIX4_PAM0  0x59
#define PIIX4_PAM1  0x5A
#define PIIX4_PAM2  0x5B
#define PIIX4_PAM3  0x5C
#define PIIX4_PAM4  0x5D
#define PIIX4_PAM5  0x5E
#define PIIX4_PAM6  0x5F

//
// OVMF-PLATFORM-PORTABILITY: generic, vendor-neutral power-management
// discovery.
//
// OVMF was written for QEMU's i440FX, where the ACPI PM function is at
// 00:01.3 with Intel DID 0x7113.  Other platforms put an equivalent device
// somewhere else (VMware 440BX: 00:07.3; ICH/LPC bridges: 00:1F.0) and may
// use a different vendor/device ID.  Rather than maintaining a chipset
// allowlist, discover the device from its PCI class code, which is defined
// by the PCI spec and is identical across vendors:
//
//   06:80 "other bridge" -> PIIX4-style PM (PMBA @0x40, PMIOSE @0x80 bit0)
//   06:01 "ISA bridge"   -> ICH-style PM   (PMBASE @0x40, ACPI_EN @0x44 bit7)
//
// Both expose the ACPI timer at PMBASE + 8.  VID/DID are HwInit (read-only)
// config registers, so a bootloader cannot fake this; the firmware itself
// has to be platform-neutral.
//
#define OVMF_PM_CLASS_PIIX4   0x0680
#define OVMF_PM_CLASS_ICH     0x0601
#define OVMF_PM_STYLE_PIIX4   1
#define OVMF_PM_STYLE_ICH     2

/**
  Find the power-management function on bus 0 and return the PCI_LIB_ADDRESS
  of `Offset` on it.  Scans all devices/functions of any vendor.  Prefers
  06:80 over 06:01 because a PIIX4 chipset has both an ISA bridge and a
  separate PM function, and the latter is the one with PMBA.
**/
STATIC
inline
UINTN
OvmfPmPciAddress (
  IN  UINT8  Offset
  )
{
  UINT8   Pass, Device, Function;
  UINT16  Want, VendorId, ClassCode;
  UINT32  ClassDword;

  for (Pass = 0; Pass < 2; Pass++) {
    Want = (Pass == 0) ? OVMF_PM_CLASS_PIIX4 : OVMF_PM_CLASS_ICH;
    for (Device = 0; Device < 32; Device++) {
      for (Function = 0; Function < 8; Function++) {
        VendorId = PciRead16 (PCI_LIB_ADDRESS (0, Device, Function, 0x00));
        if ((VendorId == 0xFFFF) || (VendorId == 0x0000)) {
          continue;                       // absent function
        }
        ClassDword = PciRead32 (PCI_LIB_ADDRESS (0, Device, Function, 0x08));
        ClassCode  = (UINT16)(ClassDword >> 16);
        if (ClassCode == Want) {
          return PCI_LIB_ADDRESS (0, Device, Function, Offset);
        }
      }
    }
  }

  return PCI_LIB_ADDRESS (0, 1, 3, Offset);   // conservative legacy default
}

/** Return OVMF_PM_STYLE_PIIX4 or OVMF_PM_STYLE_ICH for the discovered PM. **/
STATIC
inline
UINT8
OvmfPmStyle (
  VOID
  )
{
  UINT8   Device, Function;
  UINT16  VendorId;
  UINT32  ClassDword;

  for (Device = 0; Device < 32; Device++) {
    for (Function = 0; Function < 8; Function++) {
      VendorId = PciRead16 (PCI_LIB_ADDRESS (0, Device, Function, 0x00));
      if ((VendorId == 0xFFFF) || (VendorId == 0x0000)) {
        continue;
      }
      ClassDword = PciRead32 (PCI_LIB_ADDRESS (0, Device, Function, 0x08));
      if (((UINT16)(ClassDword >> 16)) == OVMF_PM_CLASS_PIIX4) {
        return OVMF_PM_STYLE_PIIX4;
      }
    }
  }

  return OVMF_PM_STYLE_ICH;
}

#define POWER_MGMT_REGISTER_PIIX4(Offset)  OvmfPmPciAddress (Offset)

#define PIIX4_PMBA       0x40
#define PIIX4_PMBA_MASK  (BIT15 | BIT14 | BIT13 | BIT12 | BIT11 |         \
                                  BIT10 | BIT9  | BIT8  | BIT7  | BIT6)

/* Style-aware ACPI-enable register and bit.  Keeping the historical macro
 * names means no caller anywhere in OvmfPkg needs to change. */
#define PIIX4_PMREGMISC \
  ((OvmfPmStyle () == OVMF_PM_STYLE_PIIX4) ? 0x80 : 0x44)
#define PIIX4_PMREGMISC_PMIOSE \
  ((OvmfPmStyle () == OVMF_PM_STYLE_PIIX4) ? BIT0 : BIT7)

//
// IO ports
//
#define PIIX4_CPU_HOTPLUG_BASE  0xAF00

#endif
