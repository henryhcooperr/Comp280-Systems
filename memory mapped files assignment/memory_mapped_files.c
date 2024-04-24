#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source file> <destination file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open the source file
    int src_fd = open(argv[1], O_RDONLY);
    if (src_fd < 0) {
        perror("Failed to open source file");
        exit(EXIT_FAILURE);
    }

    // Get the size of the source file
    struct stat statbuf;
    if (fstat(src_fd, &statbuf) < 0) {
        perror("Failed to get file size");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    if (statbuf.st_size == 0) {
        // Handle zero-length source file by creating a zero-length destination file
        int dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (dst_fd < 0) {
            perror("Failed to create zero-length destination file");
            close(src_fd);
            exit(EXIT_FAILURE);
        }
        close(dst_fd);  // Close immediately after creating
    } else {
        // Memory-map the source file
        char *src_data = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, src_fd, 0);
        if (src_data == MAP_FAILED) {
            perror("Failed to map source file");
            close(src_fd);
            exit(EXIT_FAILURE);
        }

        // Open the destination file
        int dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (dst_fd < 0) {
            perror("Failed to open destination file");
            munmap(src_data, statbuf.st_size);
            close(src_fd);
            exit(EXIT_FAILURE);
        }

        // Write the mapped data to the destination file
        if (write(dst_fd, src_data, statbuf.st_size) != statbuf.st_size) {
            perror("Failed to write to destination file");
            close(dst_fd);
            munmap(src_data, statbuf.st_size);
            close(src_fd);
            exit(EXIT_FAILURE);
        }

        // Clean up
        close(dst_fd);
        munmap(src_data, statbuf.st_size);
    }

    close(src_fd);
    return 0;
}
