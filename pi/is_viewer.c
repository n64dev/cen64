#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "is_viewer.h"

// arbitrarily chosen
#define IS_BUFFER_SIZE 0x200

int is_viewer_init(struct is_viewer *is) {
  memset(is, 0, sizeof(*is));

  // TODO support other addresses
  is->base_address = IS_VIEWER_BASE_ADDRESS;
  is->len = IS_VIEWER_ADDRESS_LEN;

  is->buffer = calloc(IS_BUFFER_SIZE, 1);
  is->output_buffer = calloc(IS_BUFFER_SIZE, 1);
  is->output_buffer_conv = calloc(IS_BUFFER_SIZE * 3, 1);

  is->cd = iconv_open("UTF-8", "EUC-JP");

  if (is->buffer == NULL || is->output_buffer == NULL ||
      is->output_buffer_conv == NULL)
    return 0;
  else
    return 1;
}

int is_viewer_map(struct is_viewer *is, uint32_t address) {
  return address >= is->base_address && address + 4 <= is->base_address + is->len;
}

int read_is_viewer(struct is_viewer *is, uint32_t address, uint32_t *word) {
  uint32_t offset = address - is->base_address;
  assert(offset + 4 <= is->len);

  memcpy(word, is->buffer + offset, sizeof(*word));
  *word = byteswap_32(*word);

  return 0;
}

int write_is_viewer(struct is_viewer *is, uint32_t address, uint32_t word, uint32_t dqm) {
  uint32_t offset = address - is->base_address;
  assert(offset + 4 <= is->len);

  if (offset == 0x14) {
    if (word > 0) {
      assert(is->output_buffer_pos + word < is->len);
      memcpy(is->output_buffer + is->output_buffer_pos, is->buffer + 0x20, word);
      is->output_buffer_pos += word;
      is->output_buffer[is->output_buffer_pos] = '\0';

      // once a full line is present, convert the output from EUC to UTF-8
      if (memchr(is->output_buffer, '\n', is->output_buffer_pos)) {
        char *inptr = (char *)is->output_buffer;
        size_t len = strlen(inptr);
        size_t outlen = 3 * len;
        char *outptr = (char *)is->output_buffer_conv;
        memset(is->output_buffer_conv, 0, IS_BUFFER_SIZE * 3);
        iconv(is->cd, &inptr, &len, &outptr, &outlen);

        printf("%s", is->output_buffer_conv);

        memset(is->output_buffer, 0, is->output_buffer_pos);
        is->output_buffer_pos = 0;
      }
    }
    memset(is->buffer + 0x20, 0, word);
  } else {
    word = byteswap_32(word);
    memcpy(is->buffer + offset, &word, sizeof(word));
  }

  return 0;
}
