/*
VERSION 2 Y,U,V convert to true color
yuv420sp_to_bmp_2 <filename> <width> <height>
example: ./yuv420sp_to_bmp_2 file.yuv 1280 800
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "yuv_to_bmp_2.h"

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
            printf("example: ./yuv420sp_to_bmp_2 file.yuv 1280 800\n");
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
    ctx->buff_y = NULL;   /* init NULL for future allocation checks */
    ctx->buff_uv = NULL;   /* init NULL for future allocation checks */
    ctx->width_y = WIDTH;
    ctx->height_y = HEIGHT;
    ctx->plane_y = WIDTH * HEIGHT;
    ctx->buff_y = (uint8_t *)malloc(sizeof(uint8_t) * ctx->plane_y);
    /* exit (1) mem exhausted */
    if(!ctx->buff_y) {
        perror("plane Y buffer allocation failed.\n");
        dealloc_fileinfo_exit(ctx);
    }

    /* get the size of UV plane */
    FILE *file_input_star = fopen(ctx->filename, "rb+");
    if(!file_input_star) {
        perror("input filename missing or corrupted.\n");
        dealloc_fileinfo_exit(ctx);
    }
    /* move cursor to the end */
    fseek(file_input_star, 0, SEEK_END);
    /* get the index value in ctx->plane_uv var */
    ctx->plane_uv = (uint32_t)ftell(file_input_star) - ctx->plane_y;
    /* move back to the beginning */
    //fseek(file_input_star, 0, SEEK_SET);
    /* close for now */
    fclose(file_input_star);
    ctx->buff_uv = (uint8_t *)malloc(sizeof(uint8_t) * ctx->plane_uv);
    if(!ctx->buff_uv) {
        perror("plane UV buffer allocation failed.\n");
        dealloc_fileinfo_exit(ctx);
    }
    /* calc and write _uv vars in the structure */
    ctx->width_uv = ctx->width_y;
    ctx->height_uv = ctx->plane_uv / ctx->width_uv;
}

/* free all in case of err */
void dealloc_fileinfo_exit(struct str_YUV_FILE_INFO *ctx) {
    
    /* flush all errors and exit the program */
    fflush(stderr);
    
    /* free the memory in each case(allocated or failed(and earlier set to NULL)) */
    free(ctx->buff_y);
    ctx->buff_y = NULL;
    free(ctx->buff_uv);
    ctx->buff_uv = NULL;
    printf("Memory freed successfully.\n");
    
    /* exit the program */
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

    /* read all bytes from the Y plane and exit in case of error or missreading */
    if (fread(ctx->buff_y, 1, ctx->plane_y, file_input_star) != ctx->plane_y) {
        perror("error while reading Y plane from file.\n");
        fclose(file_input_star);
        dealloc_fileinfo_exit(ctx);
    }

    //printf("ftell between Y and UV planes: %lu\n", ftell(file_input_star));
    
    /* read all bytes from the UV plane and exit in case of error or missreading */
    if (fread(ctx->buff_uv, 1, ctx->plane_uv, file_input_star) != ctx->plane_uv) {
        perror("error while reading UV plane from file.\n");
        fclose(file_input_star);
        dealloc_fileinfo_exit(ctx);
    }
    
    fclose(file_input_star);
}

/* put calculated vals for rgb in 0-255 range and prevent overflows and underflows */
uint8_t clamp_to_uint8_t(int rand_result) {
    /* return temp value in ranges 0 - 255 */
    if(rand_result < 0x00)
        return 0x00;
    else if(rand_result > 0xFF)
        return 0xFF;
    else
        return (uint8_t)rand_result;
}

/* process rgb and write BMP */
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
    bfh.bfSize = 2 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + ctx->width_y * ctx->height_y * 3;
    bfh.bfOffBits = 0x36;

    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = ctx->width_y;
    bih.biHeight = ctx->height_y;
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

    /* vars for next iterations and rgb producing calcs */
    uint32_t plane_uv_index;
    uint32_t plane_y_index;
    int16_t width_y;
    int16_t height_y;

    uint8_t yValue, uValue, vValue;
    uint8_t _r, _g, _b;
    int16_t tmp_r, tmp_g, tmp_b;

    /* iterate on height Y */
    for(height_y = ctx->height_y - 1; height_y >= 0; --height_y) {
        /* for debug.. printf("height_y = %u\n", height_y); */
        /* iterate on width Y */
        for(width_y = 0; width_y < ctx->width_y; ++width_y) {
            /* calc indexes for two planes */
            plane_y_index = height_y * ctx->width_y + width_y;
            plane_uv_index = (height_y / 2) * ctx->width_y + width_y;

            /* get yuv values */
            yValue = ctx->buff_y[plane_y_index];
            if(0 == width_y % 2) {
                uValue = ctx->buff_uv[plane_uv_index];
                vValue = ctx->buff_uv[plane_uv_index + 1];
            }

            /* calc temp rgb values(int16_t) using yuv values */
            tmp_r = yValue + (1.370705 * (vValue - 128));
            tmp_g = yValue - (0.698001 * (vValue - 128)) - (0.337633 * (uValue - 128));
            tmp_b = yValue + (1.732446 * (uValue - 128));

            /* calc final rgb values(uint8_t) using temp ones */
            _r = clamp_to_uint8_t(tmp_r);
            _g = clamp_to_uint8_t(tmp_g);
            _b = clamp_to_uint8_t(tmp_b);

            /* write calculated values for rgb at current pix location */
            fwrite(&_b, 1, 1, file);
            fwrite(&_g, 1, 1, file);
            fwrite(&_r, 1, 1, file);

        }
    }

    /* close output file */
    fclose(file);
}