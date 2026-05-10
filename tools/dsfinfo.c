#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

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

    uint32_t channels = 0, sample_rate = 0, block_size = 0, bits_per_sample = 0;
    uint64_t sample_count = 0;

    for (int i = 0; i < 100; i++) {
        if (memcmp(&header[i], "fmt ", 4) == 0) {
            channels = *(uint32_t*)(&header[i + 24]);
            sample_rate = *(uint32_t*)(&header[i + 28]);
            bits_per_sample = *(uint32_t*)(&header[i + 32]);
            sample_count = *(uint64_t*)(&header[i + 36]);
            block_size = *(uint32_t*)(&header[i + 44]);
            break;
        }
    }

    if (sample_rate == 0) {
        fprintf(stderr, "Valid DSF fmt chunk not found\n");
        close(fd);
        return 1;
    }

    printf("File: %s\n", argv[1]);
    printf("Channels: %u\n", channels);
    printf("Sample Rate: %u Hz\n", sample_rate);
    printf("Bits per Sample: %u\n", bits_per_sample);
    printf("Block Size: %u bytes\n", block_size);
    printf("Total Samples: %lu\n", (unsigned long)sample_count);

    close(fd);
    return 0;
}
