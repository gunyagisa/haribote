#pragma once

#include <bits/stdint-uintn.h>
#include <stdint.h>
#include "bootpack.h"

#define PCI_CONFIG_ADDR         0xcf8
#define PCI_CONFIG_DATA         0xcfc

#define PCI_CONFIG_OFFSET_BAR          0x10


uint16_t PCIConfigRead(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset)
{
  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t ldev = (uint32_t)dev;
  uint32_t lfunc = (uint32_t)func;

  address = (uint32_t)((lbus << 16) | (ldev << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

  /* アドレスの書き出し*/
  io_out32(PCI_CONFIG_ADDR, address);
  return(io_in32(PCI_CONFIG_DATA));
}

uint16_t PCICheckVendor(uint8_t bus, uint8_t dev)
{
  uint16_t vendor, device;
  /* 最初のコンフィギュレーションを読み込むテスト */
  /* ベンダーなし(0xFFFF)の場合、デバイスは存在しないことになる */
  if((vendor = (uint16_t)PCIConfigRead(bus, dev, 0, 0)) != 0xFFFF)
  {
    device = (PCIConfigRead(bus, dev, 0, 0) >> 16);
  }
  return(vendor);
}

// PCIコンフィグレーションからBARを読み込む、オフセットは0x10
void dump_BAR(uint8_t bus, uint8_t dev, uint8_t func)
{
  uint32_t bar = PCIConfigRead(bus, dev, func, PCI_CONFIG_OFFSET_BAR);
}
