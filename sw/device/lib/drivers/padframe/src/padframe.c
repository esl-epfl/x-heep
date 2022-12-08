
#include "padframe.h"
#define  PADFRAME_XHEEP_CONFIG0_BASE_ADDR PADFRAME_BASE_ADDRESS
#include "padframe_xheep_regs.h"
#include "bitfield.h"

#define REG_WRITE32(addr, value) *((volatile uint32_t*) addr) = (uint32_t) value;
#define REG_READ32(addr) *((volatile uint32_t*) addr)


void padframe_xheep_pad_io_30_cfg_chip2pad_set(uint8_t value) {
  uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_30_CFG_REG_OFFSET;
  uint32_t reg = REG_READ32(address);
  reg = bitfield_bit32_write(reg, PADFRAME_XHEEP_CONFIG_PAD_IO_30_CFG_CHIP2PAD_BIT, value);
  REG_WRITE32(address, reg);
}

uint8_t padframe_xheep_pad_io_30_cfg_chip2pad_get() {
  uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_30_CFG_REG_OFFSET;
  uint32_t reg = REG_READ32(address);
  return bitfield_bit32_read(reg, PADFRAME_XHEEP_CONFIG_PAD_IO_30_CFG_CHIP2PAD_BIT);
}

void padframe_xheep_pad_io_30_cfg_output_en_set(uint8_t value) {
  uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_30_CFG_REG_OFFSET;
  uint32_t reg = REG_READ32(address);
  reg = bitfield_bit32_write(reg, PADFRAME_XHEEP_CONFIG_PAD_IO_30_CFG_OUTPUT_EN_BIT, value);
  REG_WRITE32(address, reg);
}

uint8_t padframe_xheep_pad_io_30_cfg_output_en_get() {
  uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_30_CFG_REG_OFFSET;
  uint32_t reg = REG_READ32(address);
  return bitfield_bit32_read(reg, PADFRAME_XHEEP_CONFIG_PAD_IO_30_CFG_OUTPUT_EN_BIT);
}

void padframe_xheep_pad_io_30_mux_set(padframe_xheep_pad_io_30_mux_sel_t mux_sel) {
  const uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_30_MUX_SEL_REG_OFFSET;
  REG_WRITE32(address, mux_sel & 3);
}

padframe_xheep_pad_io_30_mux_sel_t padframe_xheep_pad_io_30_mux_get() {
  const uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_30_MUX_SEL_REG_OFFSET;

  return REG_READ32(address) & 3;
}

void padframe_xheep_pad_io_31_cfg_chip2pad_set(uint8_t value) {
  uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_31_CFG_REG_OFFSET;
  uint32_t reg = REG_READ32(address);
  reg = bitfield_bit32_write(reg, PADFRAME_XHEEP_CONFIG_PAD_IO_31_CFG_CHIP2PAD_BIT, value);
  REG_WRITE32(address, reg);
}

uint8_t padframe_xheep_pad_io_31_cfg_chip2pad_get() {
  uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_31_CFG_REG_OFFSET;
  uint32_t reg = REG_READ32(address);
  return bitfield_bit32_read(reg, PADFRAME_XHEEP_CONFIG_PAD_IO_31_CFG_CHIP2PAD_BIT);
}

void padframe_xheep_pad_io_31_cfg_output_en_set(uint8_t value) {
  uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_31_CFG_REG_OFFSET;
  uint32_t reg = REG_READ32(address);
  reg = bitfield_bit32_write(reg, PADFRAME_XHEEP_CONFIG_PAD_IO_31_CFG_OUTPUT_EN_BIT, value);
  REG_WRITE32(address, reg);
}

uint8_t padframe_xheep_pad_io_31_cfg_output_en_get() {
  uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_31_CFG_REG_OFFSET;
  uint32_t reg = REG_READ32(address);
  return bitfield_bit32_read(reg, PADFRAME_XHEEP_CONFIG_PAD_IO_31_CFG_OUTPUT_EN_BIT);
}

void padframe_xheep_pad_io_31_mux_set(padframe_xheep_pad_io_31_mux_sel_t mux_sel) {
  const uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_31_MUX_SEL_REG_OFFSET;
  REG_WRITE32(address, mux_sel & 3);
}

padframe_xheep_pad_io_31_mux_sel_t padframe_xheep_pad_io_31_mux_get() {
  const uint32_t address = PADFRAME_BASE_ADDRESS + PADFRAME_XHEEP_CONFIG_PAD_IO_31_MUX_SEL_REG_OFFSET;

  return REG_READ32(address) & 3;
}
