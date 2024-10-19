#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

typedef uint8_t BYTE;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint16_t WORD;

typedef struct {
    BYTE red;
    BYTE green;
    BYTE blue;
} pixel;

#pragma pack(push, 1)
typedef struct {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

typedef struct {
    BYTE* key;
    size_t keyLength;
    size_t originalFileSize;
} EncryptionKey;

pixel convertToColor(BYTE byte, size_t position);
BYTE recoverByte(pixel p, size_t position);
void encryptByte(BYTE* data, EncryptionKey* key, size_t position);
void decryptByte(BYTE* data, EncryptionKey* key, size_t position);
void decodeImage(char* fileName, char* outputFile, EncryptionKey* key);
void encodeImage(char* fileName, EncryptionKey* key);

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Correct usage: [-o (output) or -i (input)] [file] [key] [outputFile (if output)]\n");
        return 1;
    }

    EncryptionKey key;
    key.key = (BYTE*)argv[3];
    key.keyLength = strlen(argv[3]);

    if (strcmp(argv[1], "-o") != 0 && strcmp(argv[1], "-i") != 0) {
        printf("Please provide valid -o or -i arg\n");
        return 1;
    }

    if (strcmp(argv[1], "-i") == 0) {
        encodeImage(argv[2], &key);
        printf("Conversion complete. Output saved as encoded.bmp\n");
    }
    else if (strcmp(argv[1], "-o") == 0) {
        if (argc < 5) {
            printf("Please provide output file as the last arg.\n");
            return 1;
        }
        decodeImage(argv[2], argv[4], &key);
        printf("Decoded successfully. Output saved as %s\n", argv[4]);
    }

    return 0;
}

pixel convertToColor(BYTE byte, size_t position) {
    pixel p;
    switch (position % 3) {
        case 0:
            p.red = byte;
            p.green = byte / 3;
            p.blue = byte / 2;
            break;
        case 1:
            p.red = byte / 2;
            p.green = byte;
            p.blue = byte / 3;
            break;
        case 2:
            p.red = byte / 3;
            p.green = byte / 2;
            p.blue = byte;
            break;
    }
    return p;
}

BYTE recoverByte(pixel p, size_t position) {
    switch (position % 3) {
        case 0:
            return p.red;
        case 1:
            return p.green;
        case 2:
            return p.blue;
        default:
            return p.red;
    }
}

void encryptByte(BYTE* data, EncryptionKey* key, size_t position) {
    *data ^= key->key[position % key->keyLength];
}

void decryptByte(BYTE* data, EncryptionKey* key, size_t position) {
    encryptByte(data, key, position);
}

void decodeImage(char* fileName, char* outputFile, EncryptionKey* key) {
    FILE* fIn = NULL;
    fopen_s(&fIn, fileName, "rb");
    if (!fIn) {
        perror("Error opening input file\n");
        exit(1);
    }

    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;

    fread(&bfh, sizeof(BITMAPFILEHEADER), 1, fIn);
    fread(&bih, sizeof(BITMAPINFOHEADER), 1, fIn);

    size_t originalSize = bfh.bfReserved1 | ((size_t)bfh.bfReserved2 << 16);

    int padding = (4 - (bih.biWidth * sizeof(pixel)) % 4) % 4;

    FILE* fOut = NULL;
    fopen_s(&fOut, outputFile, "wb");
    if (!fOut) {
        perror("Error opening output file\n");
        fclose(fIn);
        exit(1);
    }

    pixel p;
    size_t position = 0;
    size_t bytesWritten = 0;

    for (int y = 0; y < bih.biHeight && bytesWritten < originalSize; y++) {
        for (int x = 0; x < bih.biWidth && bytesWritten < originalSize; x++) {
            fread(&p, sizeof(pixel), 1, fIn);
            BYTE original = recoverByte(p, position);
            decryptByte(&original, key, position++);
            fwrite(&original, 1, 1, fOut);
            bytesWritten++;
        }
        fseek(fIn, padding, SEEK_CUR);
    }

    fclose(fIn);
    fclose(fOut);
}

void encodeImage(char* fileName, EncryptionKey* key) {
    FILE* fIn = NULL;
    fopen_s(&fIn, fileName, "rb");
    if (!fIn) {
        printf("Could not open input file.\n");
        exit(1);
    }

    fseek(fIn, 0, SEEK_END);
    key->originalFileSize = ftell(fIn);
    fseek(fIn, 0, SEEK_SET);

    int width = (int)sqrt(key->originalFileSize) + 1;
    int height = width;
    int padding = (4 - (width * sizeof(pixel)) % 4) % 4;
    int rowSize = width * sizeof(pixel) + padding;

    BITMAPFILEHEADER bfh = { 0 };
    BITMAPINFOHEADER bih = { 0 };

    bfh.bfType = 0x4D42;
    bfh.bfSize = 54 + rowSize * height;
    bfh.bfReserved1 = (WORD)(key->originalFileSize & 0xFFFF);
    bfh.bfReserved2 = (WORD)((key->originalFileSize >> 16) & 0xFFFF);
    bfh.bfOffBits = 54;

    bih.biSize = 40;
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biSizeImage = rowSize * height;

    FILE* fOut = NULL;
    fopen_s(&fOut, "encoded.bmp", "wb");
    if (!fOut) {
        printf("Could not create output file.\n");
        fclose(fIn);
        exit(1);
    }

    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fOut);
    fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fOut);

    BYTE byte;
    pixel p;
    BYTE padByte = 0;
    size_t position = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (fread(&byte, 1, 1, fIn) == 1) {
                encryptByte(&byte, key, position);
                p = convertToColor(byte, position);
                position++;
                fwrite(&p, sizeof(pixel), 1, fOut);
            }
            else {
                p.red = (BYTE)(x * 255 / width);
                p.green = (BYTE)(y * 255 / height);
                p.blue = (BYTE)((x + y) * 255 / (width + height));
                fwrite(&p, sizeof(pixel), 1, fOut);
            }
        }

        for (int i = 0; i < padding; i++) {
            fwrite(&padByte, 1, 1, fOut);
        }
    }

    fclose(fIn);
    fclose(fOut);
}
