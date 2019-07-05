/*
VERSION 1
yuv420sp_to_bmp_1 <filename> <width> <height>
./yuv420sp_to_bmp_1 file.yuv 1280 800
yuv420sp_to_bmp_1 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "yuv_to_bmp_1.h"

uint16_t WIDTH = 1280;
uint16_t HEIGHT = 800;

int main(int argc, char **argv) {
    /* create yuv fileinfo struct and set NULL to pointers */
    struct str_YUV_FILE_INFO yuv_file_info;

    /* check input params */
    if(argc < 4 ||
       strlen(argv[1]) >= FILENAMES_SIZE ||
       !*argv[2] || !*argv[3]) {
            printf("please enter correct input params\n");
            printf("example: ./yuv420sp_to_bmp_1 file.yuv 1280 800\n");
            exit(EXIT_FAILURE);
    }
    /* init global vars and filename in the struct */
    yuv_file_info.filename = argv[1];

    /* init fileinfo struct */
    make_fileinfo(&yuv_file_info);

    /* raed the yuv input file */
    YUV420SP_NV12_read(&yuv_file_info);

    /* make bmp output */
    bitmap_output(&yuv_file_info, FILENAME_OUTPUT);

    /* free all before exit */
    dealloc_fileinfo_exit(&yuv_file_info);
    return 0;
}

/* init 'str_YUV_FILE_INFO' components */
void make_fileinfo(struct str_YUV_FILE_INFO *ctx) {
    ctx->buff = NULL;   /* init NULL for future allocation checks */
    ctx->width = WIDTH;
    ctx->height = HEIGHT;
    ctx->plane = WIDTH * HEIGHT;
    ctx->buff = (uint8_t *)malloc(sizeof(uint8_t) * ctx->plane);
    /* exit (1) mem exhausted */
    if(!ctx->buff) {
        dealloc_fileinfo_exit(ctx);
    }
}

/* free all in case of err */
void dealloc_fileinfo_exit(struct str_YUV_FILE_INFO *ctx) {
    if (!ctx->buff) {
        perror("plane buffer allocation failed.\n");
    }
    if (!ctx->filename) {
        perror("input filename missing or corrupted.\n");
        free(ctx->buff);
        ctx->buff = NULL;
    }

    /* flush all errors and exit the program */
    fflush(stderr);
    exit(EXIT_FAILURE);
}

/* open file, read it, close file */
void YUV420SP_NV12_read(struct str_YUV_FILE_INFO *ctx) {
    /* input FILE* structure */
    FILE *file_input_star;
    file_input_star = fopen(ctx->filename, "rb+");

    /* if failed to open -> exit */
    if(!file_input_star) {
        perror("error opening input file.\n");
        dealloc_fileinfo_exit(ctx);
    }

    /* read all bytes from the YUV and exit in case of error or missreading */
    if (fread(ctx->buff, 1, ctx->plane, file_input_star) != ctx->plane) {
        perror("error reading input file.\n");
        fclose(file_input_star);
        dealloc_fileinfo_exit(ctx);
    }
    
    fclose(file_input_star);
}

//output BMP
void bitmap_output(struct str_YUV_FILE_INFO *ctx, const char *filename) {
    uint32_t y, x, index;

    /* bmp file header and info structures */
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;

    /* Magic number for file. It does not fit in the header structure due to alignment requirements, so put it outside */
    /* fill bmp header and info structures */
    unsigned short bfType=0x4d42;
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfSize = 2 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + ctx->width * ctx->height * 3;
    bfh.bfOffBits = 0x36;

    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = ctx->width;
    bih.biHeight = ctx->height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = 0;
    bih.biSizeImage = 0;
    bih.biXPelsPerMeter = 5000;
    bih.biYPelsPerMeter = 5000;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    /* open output FILE and if err occur exit */
    FILE *file = fopen(filename, "wb+");
    if (!file) {
        printf("error opening output file.\n");
        dealloc_fileinfo_exit(ctx);
        return;
    }

    /* write headers */
    fwrite(&bfType, 1, sizeof(bfType), file);
    fwrite(&bfh, 1, sizeof(bfh), file);
    fwrite(&bih, 1, sizeof(bih), file);

    /* write out the bytes from ctx->buff like every byte is written three times to represent rgb componnets */
    for(uint32_t bl = 0; bl < ctx->plane; ++bl) {
        // printf("bla\n");
        fwrite(ctx->buff + bl, 1, 1, file);
        fwrite(ctx->buff + bl, 1, 1, file);
        fwrite(ctx->buff + bl, 1, 1, file);
    }

    /* close output file */
    fclose(file);
}