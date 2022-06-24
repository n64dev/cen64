#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "is_viewer.h"

#define READ_HEAD 0x04
#define WRITE_HEAD 0x14
#define BUFFER 0x20

// Minus text buffer base offset, plus NULL terminator
#define IS_BUFFER_SIZE IS_VIEWER_ADDRESS_LEN - BUFFER + 1

int is_viewer_init(struct is_viewer *is, int is_viewer_output) {
  memset(is, 0, sizeof(*is));

  // TODO support other addresses
  is->base_address = IS_VIEWER_BASE_ADDRESS;
  is->len = IS_VIEWER_ADDRESS_LEN;

  is->buffer = calloc(IS_VIEWER_ADDRESS_LEN, 1);
  is->output_buffer = calloc(IS_BUFFER_SIZE, 1);
  is->output_buffer_conv = calloc(IS_BUFFER_SIZE * 3, 1);
  is->show_output = is_viewer_output;

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
    uint32_t read_head;
    memcpy(&read_head, is->buffer + READ_HEAD, sizeof(read_head));
    read_head = byteswap_32(read_head);

    uint32_t write_head;
    memcpy(&write_head, is->buffer + WRITE_HEAD, sizeof(write_head));
    write_head = byteswap_32(write_head);

    uint32_t count;
    if (word < read_head) {
      // Ring buffer has wrapped
      uint32_t first_half = (IS_VIEWER_ADDRESS_LEN - BUFFER) - read_head;
      memcpy(is->output_buffer, is->buffer + BUFFER + read_head, first_half);
      memcpy(is->output_buffer + first_half, is->buffer + BUFFER, word);
      count = first_half + word;
    } else {
      // Fast path: string is in sequential memory
      count = word - read_head;
      memcpy(is->output_buffer, is->buffer + BUFFER + read_head, count);
    }
    is->output_buffer[count] = '\0';

    // once a full line is present, convert the output from EUC to UTF-8
    if (memchr(is->output_buffer, '\n', count)) {
      char *inptr = (char *)is->output_buffer;
      size_t len = count;
      size_t outlen = 3 * len;
      char *outptr = (char *)is->output_buffer_conv;
      memset(is->output_buffer_conv, 0, IS_BUFFER_SIZE * 3 + 1);
      iconv(is->cd, &inptr, &len, &outptr, &outlen);

      if (is->show_output)
        printf("%s", is->output_buffer_conv);
      else if (!is->output_warning) {
        printf("ISViewer debugging output detected and suppressed.\nRun cen64 with option -is-viewer to display it\n");
        is->output_warning = 1;
      }

      // Update read head
      read_head = byteswap_32(word);
      memcpy(is->buffer + READ_HEAD, &read_head, sizeof(read_head));
    }
  }

  word = byteswap_32(word);
  memcpy(is->buffer + offset, &word, sizeof(word));

  return 0;
}
