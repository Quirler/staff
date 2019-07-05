/*
BONUS VERSION: The utility parameters are as follows:

yuv420sp_to_bmp_4 <filename> <width> <height> <stride> <line_padding> <crop_x> <crop_y> <crop_width> <crop_height>
./yuv420sp_to_bmp_4 file_VENUS_NV12.yuv 1280 720 1280 16 0 0 1280 720

produces a cropped image based on the crop_x, crop_y, crop_width, crop_height parameters.
takes into account stride and line padding (for example for the VENUS NV12 format) to produce a full-color image
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "yuv_to_bmp_4.h"

int main(int argc, char **argv) {
    /* exactly 10 params needed and not infinit filename */
    if(argc < 10 || strlen(argv[1]) >= FILENAMES_SIZE) {
        printf("please enter correct input params\n");
        printf("example:\n./yuv420sp_to_bmp_4 file_VENUS_NV12.yuv 1280 720 1280 16 0 0 1280 720\n");
        exit(EXIT_FAILURE);
    }

    /* create yuv parameters structure with default set of params */
    struct str_YUV_PARAMS yuv_params = {
        /* default vals */
        .width = 1280,
        .height = 720,
        .stride = 1280,
        .line_padding = 16,
        .crop_x = 128,
        .crop_y = 32,
        .crop_width = 512,
        .crop_height = 640
    };

    /* input and validate params */
    input_validate_params(&yuv_params, argv);

    /* create yuv fileinfo struct and set NULL to pointers */
    struct str_YUV_FILE_INFO yuv_file_info;

    /* save input filename */
    yuv_file_info.filename = argv[1];

    /* init fileinfo struct */
    make_fileinfo(&yuv_file_info, &yuv_params);

    /* raed the yuv input file */
    YUV420SP_NV12_read(&yuv_file_info, &yuv_params);

    /* make bmp output */
    bitmap_output(&yuv_file_info, FILENAME_OUTPUT);

    /* free all and exit */
    dealloc_fileinfo_exit(&yuv_file_info);
    return 0;
}

/* input and validate params */
void input_validate_params(struct str_YUV_PARAMS *param, char **argv) {

    /* init global vars in the struct */
    param->width = atoi(argv[2]);
    param->height = atoi(argv[3]);
    param->stride = atoi(argv[4]);
    param->line_padding = atoi(argv[5]);
    param->crop_x = atoi(argv[6]);
    param->crop_y = atoi(argv[7]);
    param->crop_width = atoi(argv[8]);
    param->crop_height = atoi(argv[9]);

    /* make the check */
    if( param->stride < param->width || param->crop_x > param->width || param->crop_y > param->height || 
        param->crop_width > param->width || param->crop_height > param->height ) {
            printf("please enter correct input params\n");
            printf("example:\n./yuv420sp_to_bmp_4 file_VENUS_NV12.yuv 1280 720 1280 16 0 0 1280 720\n");
            exit(EXIT_FAILURE);
    }
}

/* init 'str_YUV_FILE_INFO' components */
void make_fileinfo(struct str_YUV_FILE_INFO *ctx, struct str_YUV_PARAMS *param) {
    ctx->buff_y = NULL;   /* init NULL for future allocation checks */
    ctx->buff_uv = NULL;   /* init NULL for future allocation checks */
    ctx->width_y = param->crop_width;
    ctx->height_y = param->crop_height;
    ctx->plane_y = ctx->width_y * ctx->height_y;
    ctx->buff_y = (uint8_t *)malloc(sizeof(uint8_t) * ctx->plane_y);
    /* exit (1) mem exhausted */
    if(!ctx->buff_y) {
        perror("plane Y buffer allocation failed.\n");
        dealloc_fileinfo_exit(ctx);
    }
    printf("allocated %luB for buff_y\n", sizeof(uint8_t) * ctx->plane_y);
    
    ctx->plane_uv = ctx->plane_y / 2;
    ctx->buff_uv = (uint8_t *)malloc(sizeof(uint8_t) * ctx->plane_uv);
    /* exit (2) mem exhausted */
    if(!ctx->buff_uv) {
        perror("plane UV buffer allocation failed.\n");
        dealloc_fileinfo_exit(ctx);
    }
    printf("allocated %uB for buff_uv\n", ctx->plane_uv);
    /* calc and write _uv vars in the structure */
    ctx->width_uv = ctx->width_y;
    ctx->height_uv = ctx->plane_uv / ctx->width_uv; // = ctx->height_y / 2
    fflush(stdout);
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
void YUV420SP_NV12_read(struct str_YUV_FILE_INFO *ctx, struct str_YUV_PARAMS *param) {
    /* input FILE* structure */
    FILE *file_input_star;
    file_input_star = fopen(ctx->filename, "rb+");

    /* if failed to open -> exit */
    if(!file_input_star) {
        perror("error opening input file.\n");
        dealloc_fileinfo_exit(ctx);
    }

    /* vars about reading the Y and UV planes */
    size_t read = 0u;
    uint16_t z;
    uint32_t index;
    uint32_t offset;

    /* read cropped Y plane */
    for(z = 0u; z < ctx->height_y; ++z) {
        /* index for rows in ctx->buff_y */
        index = z * ctx->width_y;
        /* calc the offset for cursor */
        offset = (param->crop_y + z) * param->stride + param->crop_x;
        /* set cursor in cropped area */
        fseek(file_input_star, offset, SEEK_SET);
        /* for each row read width_y elements */
        read += fread(ctx->buff_y + index, 1u, ctx->width_y, file_input_star);
    }
    /* print what was done after Y plane readed */
    printf("Y_plane readed: %luB of %dB\n", read, ctx->plane_y);
    /* if not read all plane_y elements */
    if(read < ctx->plane_y) {
        printf("error while reading Y plane from file.\n");
        fclose(file_input_star);
        dealloc_fileinfo_exit(ctx);
    }
    
    /* zeroes bytes counter */
    read = 0;

    /* read UV plane */
    for(z = 0u; z < ctx->height_uv; ++z) {
        /* index for ctx->buff_uv */
        index = z * ctx->width_uv;
        /* calc the offset for cursor */
        offset =  (z + param->height + param->line_padding + param->crop_y / 2) * param->stride + param->crop_x;
        /* set cursor in cropped area */
        fseek(file_input_star, offset, SEEK_SET);
        /* for each row read width_uv elements */
        read += fread(ctx->buff_uv + index, 1u, ctx->width_uv, file_input_star);
    }
    /* print what was done after UV plane readed */
    printf("UV_plane readed: %luB of %dB\n", read, ctx->plane_uv);
    /* if not read all plane_uv elements */
    if(read < ctx->plane_uv) {
        printf("error while reading UV plane from file.\n");
        fclose(file_input_star);
        dealloc_fileinfo_exit(ctx);
    }

    /* close file* */
    fclose(file_input_star);
    fflush(stdout);
    fflush(stderr);
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
    bfh.bfSize = sizeof(bfType) + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + ctx->width_y * ctx->height_y * 3;
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
        /* iterate on width Y */
        for(width_y = 0; width_y < ctx->width_y; ++width_y) {
            /* calc indexes for two planes */
            plane_y_index = PIX(height_y, width_y, ctx->width_y);
            plane_uv_index = PIX(height_y / 2, width_y, ctx->width_y);
            
            /* get YUV values */
            yValue = ctx->buff_y[plane_y_index];
            //yValue = 0x7f;    //to output UV plane only
            if(0 == width_y % 2) {
                uValue = ctx->buff_uv[plane_uv_index];
                vValue = ctx->buff_uv[plane_uv_index + 1];
            }

            /* calc temp RGB values(int16_t) using yuv values */
            tmp_r = yValue + (1.370705 * (vValue - 128));
            tmp_g = yValue - (0.698001 * (vValue - 128)) - (0.337633 * (uValue - 128));
            tmp_b = yValue + (1.732446 * (uValue - 128));

            /* calc final RGB values(uint8_t) using temp ones */
            _r = clamp_to_uint8_t(tmp_r);
            _g = clamp_to_uint8_t(tmp_g);
            _b = clamp_to_uint8_t(tmp_b);

            /* write calculated values for RGB at current pix location */
            /* correct order is b, g, r */
            fwrite(&_b, 1, 1, file);
            fwrite(&_g, 1, 1, file);
            fwrite(&_r, 1, 1, file);
        }
    }

    /* close output file */
    fclose(file);
}