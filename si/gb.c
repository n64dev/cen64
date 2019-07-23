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


static const uint16_t CARTRIDGE_TYPE_ADDRESS = 0x0147;
static const uint16_t ROM_SIZE_CODE_ADDRESS = 0x0148;
static const uint16_t EXTRAM_SIZE_CODE_ADDRESS = 0x0149;

static uintmax_t power(unsigned x, unsigned y) {
  uintmax_t z = 1;
  uintmax_t base = x;
  while (y) {
    if (y & 1) {  // or y%2
      z *= base;
    }
    y >>= 1; // or y /= 2
    base *= base;
  }
  return z;
}

static uint8_t mbc_read_ff( struct controller *controller, uint16_t address )
{
  return 0xff;
}

static void mbc_write_dummy( struct controller *controller, uint16_t address, uint8_t data )
{
}

// write A000-BFFF extram
void mbc_mbc1_write_extram( struct controller *controller, uint16_t address, uint8_t data ) {
  controller->cart.extram_bank[address&0x1fff] = data;
}

// read A000-BFFF (extram)
uint8_t mbc_mbc1_read_extram( struct controller *controller, uint16_t address ) {
  return controller->cart.extram_bank[address&0x1fff];
}

static void mbc_write_ram_enable( struct controller *controller, uint16_t address, uint8_t data )
{
  data &= 0xF;
  if (data == 0) {
    controller->cart.extram_enabled = false;
  } else if (data == 0x0A) {
    controller->cart.extram_enabled = true;
  }
}

static void mbc_rom_only_install(struct controller *controller) {
  mbc_mbc3_install(controller);
}


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

void mbc_unsupported_install(struct controller *controller) {
  printf("Transfer Pak ROM type not fully supported. Defaulting to MBC3 behaviour\n");
  mbc_mbc3_install(controller);
}

void gb_init(struct controller *controller) {
  for(int i=0;i<0x100;i++) {
    controller->gb_readmem [i] = NULL;
    controller->gb_writemem[i] = NULL;
  }
  
  struct gb_cart *cart = &controller->cart;
  
  // set up pak rom
  cart->cartromsize = controller->tpak_rom.size;
  cart->cartrom = (uint8_t *)(controller->tpak_rom.ptr);
  cart->cartrom_bank_zero = cart->cartrom;
  cart->cartrom_bank_n = cart->cartrom + 0x4000;
  
  // set up pak ram
  if( controller->tpak_save.ptr != NULL ) {
    cart->extram = (uint8_t *)(controller->tpak_save.ptr);
    cart->extram_size = controller->tpak_save.size;
  } else {
    cart->extram = calloc( 1, 65536 );  // TODO: deallocate this eventually?
    cart->extram_size = 65536;
  }   
  cart->extram_bank = cart->extram;

  uint8_t rom_size_code = cart->cartrom_bank_zero[ROM_SIZE_CODE_ADDRESS];
  if (rom_size_code <= 8) {
    cart->cartrom_num_banks = power(2, rom_size_code + 1);
  } else {
    switch(rom_size_code) {
      case 0x52: 
        cart->cartrom_num_banks = 72;
        break;
      case 0x53: 
        cart->cartrom_num_banks = 80;
        break;
      case 0x54:
        cart->cartrom_num_banks = 96;
        break;
    }
  }

  switch(cart->cartrom_bank_zero[EXTRAM_SIZE_CODE_ADDRESS]) {
    case 0: 
      cart->extram_num_banks = 0;
      break;
    case 1: 
      cart->extram_num_banks = 1;
      break;
    case 2:
      cart->extram_num_banks = 1;
      break;
    case 3: 
      cart->extram_num_banks = 4;
      break;
    case 4:
      cart->extram_num_banks = 16;    
      break;
    case 5: 
      cart->extram_num_banks = 8;    
      break;
  }  

  switch(cart->cartrom_bank_zero[CARTRIDGE_TYPE_ADDRESS]) {
    case 0:
      mbc_rom_only_install(controller);
      break;
    case 1:
    case 2:
    case 3:
      mbc_mbc1_install(controller);
      break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
      mbc_mbc3_install(controller);
      break;
    default:
      mbc_unsupported_install(controller);
      break;
  }
}

uint8_t mbc_mbc1_read_bank_0(struct controller *controller, uint16_t address) {
  return controller->cart.cartrom_bank_zero[address];
}

uint8_t mbc_mbc1_read_bank_n(struct controller *controller, uint16_t address) {
  return controller->cart.cartrom_bank_n[address - 0x4000];
}

// write 0000 - 1FFF
void mbc_mbc1_write_ram_enable(struct controller *controller, uint16_t address, uint8_t data) {
  mbc_write_ram_enable(controller, address, data);
}

// write 2000-3FFF
void mbc_mbc1_write_rom_bank_select( struct controller *controller, uint16_t address, uint8_t data ) {
  struct gb_cart *cart = &controller->cart;
  
  // Only first 5 bits of this register count.
  data &= 0x1F;

  // And then we shift the 2 bits from the high register on top of it.
  uint8_t bank = (cart->reg_rom_bank_high << 5) | data;
  // lower bank register cannot contain 0, so it automatically becomes 1.
  if (bank == 0x00 || bank == 0x20 || bank == 0x40 || bank == 0x60) {
      bank++;
  }

  // If the selected bank is higher than the number of banks, it loops around.
  if (bank >= cart->cartrom_num_banks) {
      bank = bank % cart->cartrom_num_banks;
  }

  cart->reg_rom_bank_low = bank & 0x1F;
  cart->cart_bank_num = bank;  
  
  cart->cartrom_bank_n = cart->cartrom + bank * 0x4000;    
}

// write 4000-5FFF
void mbc_mbc1_write_rom_ram_bank_select( struct controller *controller, uint16_t address, uint8_t data ) {
  // this is where MBC1 gets stupidly complicated.
  // we only care abotu the first two bits.
  data &= 3;

  struct gb_cart *cart = &controller->cart;  
  if (cart->mbc1_mode == 0 || cart->cartrom_num_banks > 0x1F) {
    // According to the mooneye-gb tests writing here updates rom_bank_n for larger ROMS, even in RAM mode.
    // This is contrary to more easy to find info and common sense.
    if (cart->cartrom_num_banks > 0x1F) {
      // reg_rom_bank_low is 5 bits.
      uint8_t bank = (data << 5) | cart->reg_rom_bank_low;
      // lower bank register cannot contain 0, so it automatically becomes 1.
      if (bank == 0x00 || bank == 0x20 || bank == 0x40 || bank == 0x60) {
        bank++;
      }

      // If the selected bank is higher than the number of banks, it loops around.
      if (bank >= cart->cartrom_num_banks) {
        bank = bank % cart->cartrom_num_banks;
      }      

      cart->reg_rom_bank_high = bank >> 5;
      cart->cartrom_bank_n = cart->cartrom + bank * 0x4000;
    }

    // in ROM mode, 0x0000 should always point to bank 0
    cart->cartrom_bank_zero = cart->cartrom;

    if (cart->mbc1_mode == 0) {
      // If we're not in mode 1, 'RAM' mode, we're done for now.
      return;
    }
  }

  if (cart->mbc1_mode == 1) {
    // Again according to mooneye-gb tests, bank_zero actually changes when you write in mode 1.
    // This is how you can access banks 20, 40 & 60
    if (cart->cartrom_num_banks > 0x20) {
      uint8_t bank = data << 5;
      if (bank >= cart->cartrom_num_banks) {
        bank = bank % cart->cartrom_num_banks;
      }          

      cart->cartrom_bank_zero = cart->cartrom + bank * 0x4000;
    }

    // There are no banks to switch.
    if (cart->extram_num_banks <= 1) {
      return;
    }

    cart->extram_bank_num = data;
    cart->extram_bank = cart->extram + data * 0x2000;    
  }
}

// 6000 - 7FFF
void mbc_mbc1_write_bank_mode_select(struct controller *controller, uint16_t address, uint8_t data ) {
  struct gb_cart *cart = &controller->cart;    
  cart->mbc1_mode = data & 0x1;
  if (cart->mbc1_mode == 0) {
    // Can only access ram bank 0 in mode 0.
    cart->extram_bank_num = 0;
    cart->extram_bank = cart->extram;
  } else {
    // can only access first 32 rom banks in mode 1, high reg set to 0.
    cart->reg_rom_bank_high = 0;
    cart->cartrom_bank_n = cart->cartrom + cart->reg_rom_bank_low * 0x4000;
  }
}

void mbc_mbc1_install(struct controller *controller) {
  int i;
  controller->cart.mbc1_mode = 0;

  // cart bank zero
  for( i=0x00; i<=0x3F; ++i ) {
    controller->gb_readmem[i] = mbc_mbc1_read_bank_0;
  }
  // cart bank n
  for( i=0x40; i<=0x7F; ++i ) {
    controller->gb_readmem[i] = mbc_mbc1_read_bank_n;
  }
  
  // write 0000-1FFF: ram enable
  for( i=0x00; i<=0x1F; ++i ) {
    controller->gb_writemem[i] = mbc_mbc1_write_ram_enable;
  }
  // write 2000-3FFF: rom bank select
  for( i=0x20; i<=0x3F; ++i ) {
    controller->gb_writemem[i] = mbc_mbc1_write_rom_bank_select;
  }
  // write 4000-5FFF: ram bank select
  for( i=0x40; i<=0x5F; ++i ) {
    controller->gb_writemem[i] = mbc_mbc1_write_rom_ram_bank_select;
  }

  // write 6000-7FFF: bank mode select
  for( i=0x60; i<=0x7F; ++i ) {
    controller->gb_writemem[i] = mbc_mbc1_write_bank_mode_select;
  }  
  
  // read A000-BFFF: read extram
  // calculate the last address where extram is installed
  int extram_end = 0xA0 + (controller->cart.extram_size>8192?8192:controller->cart.extram_size)/256;
  for( i=0xA0; i<extram_end; ++i ) {
    controller->gb_readmem[i] = mbc_mbc1_read_extram;
  }
  for( i=extram_end; i<=0xBF; ++i ) {
    controller->gb_readmem[i] = mbc_read_ff;
  }
  
  // write A000-BFFF: write extram
  for( i=0xA0; i<extram_end; ++i ) {
    controller->gb_writemem[i] = mbc_mbc1_write_extram;
  }
  for( i=extram_end; i<=0xBF; ++i ) {
    controller->gb_writemem[i] = mbc_write_dummy;
  }  
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
    controller->gb_readmem[i] = mbc_read_ff;
  }
  
  // write A000-BFFF: write extram
  for( i=0xA0; i<extram_end; ++i ) {
    controller->gb_writemem[i] = mbc_mbc3_write_extram;
  }
  for( i=extram_end; i<=0xBF; ++i ) {
    controller->gb_writemem[i] = mbc_write_dummy;
  }
  
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
  return mbc_write_ram_enable(controller, address, data);
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
        controller->gb_readmem[i] = mbc_read_ff;
      }
      
      // write A000-BFFF: write extram
      for( i=0xA0; i<extram_end; ++i ) {
        controller->gb_writemem[i] = mbc_mbc3_write_extram;
      }
      for( i=extram_end; i<=0xBF; ++i ) {
        controller->gb_writemem[i] = mbc_write_dummy;
      }
      break;
    case 0x08:        // seconds
    case 0x09:        // minutes
    case 0x0A:        // hours
    case 0x0B:        // day bits 0-7
    case 0x0C:        // day bit 8, carry bit, halt flag
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
  // TODO
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
  return 0x00;  // TODO
}

// write A000-BFFF rtc
void mbc_mbc3_write_rtc( struct controller *controller, uint16_t address, uint8_t data ) {
  // TODO
}
