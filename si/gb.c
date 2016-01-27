//
// si/gb.c: Game Boy pak functions
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2016, Jason Benaim.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <stddef.h>
#include "pak.h"
#include "gb.h"

void cart_reset_mbc(struct controller *controller);
void cart_default_cleanup();

uint8_t gb_read(struct controller *controller, uint16_t address) {
  uint8_t (*myfunc)(struct controller *,uint16_t) = controller->gb_readmem[address>>8];
  uint8_t data;
  if(myfunc != NULL)
    data = myfunc(controller, address);
  else
    data = 0xFF;
//   printf("gb_read: %04X:%02X\n", address, data);
  return data;
}

void gb_write(struct controller *controller, uint16_t address, uint8_t data) {
  void (*myfunc)(struct controller *,uint16_t,uint8_t) = controller->gb_writemem[address>>8];
  if(myfunc != NULL)
    myfunc(controller, address, data);
//   printf("gb_write: %04X:%02X\n", address, data);
}

void gb_init(struct controller *controller) {
  printf("gb_init\n");
  for(int i=0;i<0x100;i++) {
    controller->gb_readmem [i] = NULL;
    controller->gb_writemem[i] = NULL;
  }
  
  // set up cart params
  struct gb_cart *cart = &controller->cart;
  cart->cartromsize = controller->tpak_rom.size;
  cart->cartrom = (uint8_t *)(controller->tpak_rom.ptr);
  cart->cartrom_bank_zero = cart->cartrom;
  cart->cartrom_bank_n = cart->cartrom + 0x4000;
  cart->extram = (uint8_t *)(controller->tpak_save.ptr);
  cart->extram_bank = cart->extram;
  cart->extram_size=32768;  // HACK
  
  mbc_mbc3_install(controller);
}

void mbc_mbc3_install(struct controller *controller)
{
  int i;
  // cart bank zero
  for( i=0x00; i<=0x3F; ++i ) {
    controller->gb_readmem[i] = mbc_mbc3_read_bank_0;
  }
  // cart bank n
  for( i=0x40; i<=0x7F; ++i ) {
    controller->gb_readmem[i] = mbc_mbc3_read_bank_n;
  }
  
  // write 0000-1FFF: ram enable
  for( i=0x00; i<=0x1F; ++i ) {
    controller->gb_writemem[i] = mbc_mbc3_write_ram_enable;
  }
  // write 2000-3FFF: rom bank select
  for( i=0x20; i<=0x3F; ++i ) {
    controller->gb_writemem[i] = mbc_mbc3_write_rom_bank_select;
  }
  // write 4000-5FFF: ram bank select
  for( i=0x40; i<=0x5F; ++i ) {
    controller->gb_writemem[i] = mbc_mbc3_write_ram_bank_select;
  }
  // write 6000-7FFF: clock data latch
  for( i=0x60; i<=0x7F; ++i ) {
    controller->gb_writemem[i] = mbc_mbc3_write_clock_data_latch;
  }
  
  // read A000-BFFF: read extram
  // calculate the last address where extram is installed
  int extram_end = 0xA0 + (controller->cart.extram_size>8192?8192:controller->cart.extram_size)/256;
  for( i=0xA0; i<extram_end; ++i ) {
    controller->gb_readmem[i] = mbc_mbc3_read_extram;
  }
  for( i=extram_end; i<=0xBF; ++i ) {
    controller->gb_readmem[i] = mbc_mbc3_read_ff;
  }
  
  // write A000-BFFF: write extram
  for( i=0xA0; i<extram_end; ++i ) {
    controller->gb_writemem[i] = mbc_mbc3_write_extram;
  }
  for( i=extram_end; i<=0xBF; ++i ) {
    controller->gb_writemem[i] = mbc_mbc3_write_dummy;
  }
  
}

uint8_t mbc_mbc3_read_ff( struct controller *controller, uint16_t address )
{
  return 0xff;
}

void mbc_mbc3_write_dummy( struct controller *controller, uint16_t address, uint8_t data )
{
}

uint8_t mbc_mbc3_read_bank_0( struct controller *controller, uint16_t address )
{
  return controller->cart.cartrom_bank_zero[address];
}

uint8_t mbc_mbc3_read_bank_n( struct controller *controller, uint16_t address )
{
  return controller->cart.cartrom_bank_n[address&0x3fff];
}

// write 0000-1FFF
void mbc_mbc3_write_ram_enable( struct controller *controller, uint16_t address, uint8_t data )
{
  // TODO
}

// write 2000-3FFF
void mbc_mbc3_write_rom_bank_select( struct controller *controller, uint16_t address, uint8_t data ) {
  
  struct gb_cart *cart = &controller->cart;
  
  size_t offset;
  data &= 0x7F;
  cart->cart_bank_num = data;
  if( data == 0 )
    offset = (size_t)16384;
  else
    offset = (size_t)data*16384 % cart->cartromsize;
  
//   printf( "switch cart bank num: %02X\n", cart->cart_bank_num );
//   assert("MBC3 rom bank select: offset computation", offset <= (cart->cartromsize - 16384));
  cart->cartrom_bank_n = cart->cartrom + offset;
}

// write 4000-5FFF
void mbc_mbc3_write_ram_bank_select( struct controller *controller, uint16_t address, uint8_t data ) {
  struct gb_cart *cart = &controller->cart;
  int i;
  switch( data )
  {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
//       printf("Switching to RAM bank %02X \n", data );
      cart->extram_bank_num = data;
      cart->extram_bank = cart->extram + data*8192;
      // read A000-BFFF: read extram
      // calculate the last address where extram is installed
      int extram_end = 0xA0 + (cart->extram_size>8192?8192:cart->extram_size)/256;
      for( i=0xA0; i<extram_end; ++i ) {
        controller->gb_readmem[i] = mbc_mbc3_read_extram;
      }
      for( i=extram_end; i<=0xBF; ++i ) {
        controller->gb_readmem[i] = mbc_mbc3_read_ff;
      }
      
      // write A000-BFFF: write extram
      for( i=0xA0; i<extram_end; ++i ) {
        controller->gb_writemem[i] = mbc_mbc3_write_extram;
      }
      for( i=extram_end; i<=0xBF; ++i ) {
        controller->gb_writemem[i] = mbc_mbc3_write_dummy;
      }
      break;
    case 0x08:	// seconds
    case 0x09:	// minutes
    case 0x0A:	// hours
    case 0x0B:	// day bits 0-7
    case 0x0C:	// day bit 8, carry bit, halt flag
//       printf("Switching to RTC bank %02X \n", data );
      cart->extram_bank_num = data;
      // read A000-BFFF: read rtc
      for( i=0xA0; i<=0xBF; ++i ) {
	controller->gb_readmem[i] = mbc_mbc3_read_rtc;
      }
      // write A000-BFFF: write rtc
      for( i=0xA0; i<=0xBF; ++i ) {
	controller->gb_writemem[i] = mbc_mbc3_write_rtc;
      }
      break;
    default:
      printf("Switching to invalid extram bank %02X \n", data );
      break;
  }
}

// write 6000-7FFF
void mbc_mbc3_write_clock_data_latch( struct controller *controller, uint16_t address, uint8_t data ) {
  printf("wrote clock data latch: %04X:%02X\n", address, data);
}

// read A000-BFFF extram
uint8_t mbc_mbc3_read_extram( struct controller *controller, uint16_t address ) {
  return controller->cart.extram_bank[address&0x1fff];
}

// write A000-BFFF extram
void mbc_mbc3_write_extram( struct controller *controller, uint16_t address, uint8_t data ) {
  controller->cart.extram_bank[address&0x1fff] = data;
}

// read A000-BFFF rtc
uint8_t mbc_mbc3_read_rtc( struct controller *controller, uint16_t address ) {
  return 0x00;
}

// write A000-BFFF rtc
void mbc_mbc3_write_rtc( struct controller *controller, uint16_t address, uint8_t data ) {
}
