
Write a command-line utility in C or C++ in 3(4) versions, without using external libraries, which converts YUV420SP (NV12) images to BMP images. 


VERSION 1.The utility's parameters are as follows:

yuv420sp_to_bmp_1 <filename> <width> <height>

example:

yuv420sp_to_bmp_1 file.yuv 1920 1080 --> will produce "file.bmp" in grayscale by converting only the Y plane from the source image



VERSION 2. The utility's parameters are as follows:

yuv420sp_to_bmp_2 <filename> <width> <height>

example:

yuv420sp_to_bmp_2 file.yuv 1920 1080 1 --> will produce "file.bmp" in full-color by converting the Y, U and V planes from the source image



VERSION 3. The utility parameters are as follows:

yuv420sp_to_bmp_3 <filename> <width> <height> <stride> <line_padding>

takes into account stride and line padding (for example for the VENUS NV12 format) to produce a full-color image



BONUS VERSION: The utility parameters are as follows:

yuv420sp_to_bmp_4 <filename> <width> <height> <stride> <line_padding> <crop_x> <crop_y> <crop_width> <crop_height>

produces a cropped image based on the crop_x, crop_y, crop_width, crop_height parameters.



Useful links:

https://en.wikipedia.org/wiki/YUV
https://en.wikipedia.org/wiki/Chroma_subsampling

http://www.fourcc.org/pixel-format/yuv-nv12/
https://i.stack.imgur.com/XboNY.png

