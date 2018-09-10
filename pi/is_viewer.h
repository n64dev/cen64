#ifndef __IS_VIEWER_H__
#define __IS_VIEWER_H__

#include <iconv.h>

// IS Viewer
#define IS_VIEWER_BASE_ADDRESS    0x13FF0000
#define IS_VIEWER_ADDRESS_LEN     0x00001000

struct is_viewer {
  uint32_t base_address;
  uint32_t len;

  uint8_t *buffer;
  uint8_t *output_buffer;
  size_t output_buffer_pos;
  uint8_t *output_buffer_conv;

  iconv_t cd;
};

int is_viewer_init(const struct is_viewer *is);
int is_viewer_map(const struct is_viewer *is, uint32_t address);
int read_is_viewer(const struct is_viewer *is, uint32_t address, uint32_t *word);
int write_is_viewer(const struct is_viewer *is, uint32_t address, uint32_t word, uint32_t dqm);

#endif /* __IS_VIEWER_H__ */
