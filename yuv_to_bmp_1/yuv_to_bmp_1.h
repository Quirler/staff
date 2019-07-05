#define FILENAME_OUTPUT "file.bmp"

#define FILENAMES_SIZE 20

#define PIX(ROW, COL, COLS)((ROW) * (COLS) + (COL))

/* struct 'str_YUV_FILE_INFO' containing YUV file info */
struct str_YUV_FILE_INFO {
    uint16_t width, height;
    uint32_t plane;
    uint8_t *buff;
    char *filename;
};

/* init 'str_YUV_FILE_INFO' components */
void make_fileinfo(struct str_YUV_FILE_INFO *ctx);

/* free all in case of err */
void dealloc_fileinfo_exit(struct str_YUV_FILE_INFO *ctx);

/* open file, read it, close file */
void YUV420SP_NV12_read(struct str_YUV_FILE_INFO *ctx);

/* bitmap output function */
void bitmap_output(struct str_YUV_FILE_INFO *ctx, const char *filename);

typedef struct                       /**** BMP file header structure ****/
    {
    unsigned int   bfSize;           /* Size of file */
    unsigned short bfReserved1;      /* Reserved */
    unsigned short bfReserved2;      /* ... */
    unsigned int   bfOffBits;        /* Offset to bitmap data */
    } BITMAPFILEHEADER;

typedef struct                       /**** BMP file info structure ****/
    {
    unsigned int   biSize;           /* Size of info header */
    int            biWidth;          /* Width of image */
    int            biHeight;         /* Height of image */
    unsigned short biPlanes;         /* Number of color planes */
    unsigned short biBitCount;       /* Number of bits per pixel */
    unsigned int   biCompression;    /* Type of compression to use */
    unsigned int   biSizeImage;      /* Size of image data */
    int            biXPelsPerMeter;  /* X pixels per meter */
    int            biYPelsPerMeter;  /* Y pixels per meter */
    unsigned int   biClrUsed;        /* Number of colors used */
    unsigned int   biClrImportant;   /* Number of important colors */
    } BITMAPINFOHEADER;