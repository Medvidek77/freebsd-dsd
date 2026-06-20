#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#ifndef AFMT_DSD
#define AFMT_DSD 0x00000800
#endif

unsigned char bit_swap(unsigned char c) {
    char r = (c << 4) | (c >> 4);
    r = ((r & 0x33) << 2) | ((r & 0xcc) >> 2);
    r = ((r & 0x55) << 1) | ((r & 0xaa) >> 1);
    return r;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <dsf_file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Cannot open file");
        return 1;
    }

    uint8_t header[128];
    if (read(fd, header, sizeof(header)) != sizeof(header)) {
        fprintf(stderr, "Failed to read header\n");
        close(fd);
        return 1;
    }

    uint32_t channels = 0, sample_rate = 0, block_size = 0;
    int data_offset = -1;

    for (int i = 0; i < 100; i++) {
        if (memcmp(&header[i], "fmt ", 4) == 0) {
            channels = *(uint32_t*)(&header[i + 24]);
            sample_rate = *(uint32_t*)(&header[i + 28]);
            block_size = *(uint32_t*)(&header[i + 44]);
        }
        if (memcmp(&header[i], "data", 4) == 0) {
            data_offset = i + 12;
            break;
        }
    }

    if (data_offset == -1 || block_size == 0) {
        fprintf(stderr, "Valid DSF data not found\n");
        close(fd);
        return 1;
    }

    int dsp = open("/dev/dsp0", O_WRONLY);
    if (dsp < 0) {
        perror("Cannot open /dev/dsp0");
        close(fd);
        return 1;
    }

    int format = AFMT_DSD;
    int speed = sample_rate / 32;

    if (ioctl(dsp, SNDCTL_DSP_SETFMT, &format) == -1) {
        perror("SNDCTL_DSP_SETFMT");
    }
    if (ioctl(dsp, SNDCTL_DSP_CHANNELS, &channels) == -1) {
        perror("SNDCTL_DSP_CHANNELS");
    }
    if (ioctl(dsp, SNDCTL_DSP_SPEED, &speed) == -1) {
        perror("SNDCTL_DSP_SPEED");
    }

    lseek(fd, data_offset, SEEK_SET);

    uint32_t chunk_size = channels * block_size;
    uint8_t *raw_buf = malloc(chunk_size);
    uint8_t *play_buf = malloc(chunk_size);
    ssize_t bytes_read;

    while ((bytes_read = read(fd, raw_buf, chunk_size)) == chunk_size) {
        for (uint32_t i = 0; i < block_size / 4; i++) {
            int read_offset = i * 4;

            for (uint32_t c = 0; c < channels; c++) {
                int block_offset = read_offset + (c * block_size);
                int play_offset = (i * channels + c) * 4;

                play_buf[play_offset + 0] = bit_swap(raw_buf[block_offset + 0]);
                play_buf[play_offset + 1] = bit_swap(raw_buf[block_offset + 1]);
                play_buf[play_offset + 2] = bit_swap(raw_buf[block_offset + 2]);
                play_buf[play_offset + 3] = bit_swap(raw_buf[block_offset + 3]);
            }
        }
        if (write(dsp, play_buf, chunk_size) != chunk_size) {
            perror("Write to DSP failed");
            break;
        }
    }

    free(raw_buf);
    free(play_buf);
    close(dsp);
    close(fd);
    return 0;
}
