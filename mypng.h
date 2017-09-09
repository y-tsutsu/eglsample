#pragma once

#include <png.h>
#include <string>
#include <string.h>

struct PngInfo
{
    png_uint_32 width;
    png_uint_32 height;
    unsigned char *data;
    bool has_alpha;
};

PngInfo loadPng(std::string filename)
{
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp == nullptr)
    {
        std::cerr << "Error fopen." << std::endl;
        exit(EXIT_FAILURE);
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr)
    {
        std::cerr << "Error png_create_read_struct." << std::endl;
        exit(EXIT_FAILURE);
    }

    png_infop info = png_create_info_struct(png);
    if (info == nullptr)
    {
        std::cerr << "Error png_create_info_struct." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (setjmp(png_jmpbuf(png)))
    {
        std::cerr << "Error png_jmpbuf." << std::endl;
        exit(EXIT_FAILURE);
    }

    png_init_io(png, fp);

    unsigned int sig_bytes = 0;
    png_set_sig_bytes(png, sig_bytes);

    png_read_png(png, info, (PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND), nullptr);

    png_uint_32 width = 0;
    png_uint_32 height = 0;
    int bit_depth = 0;
    int color_type = 0;
    int interlace_type = 0;
    int compression_type = 0;
    int filter_method = 0;
    png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

    unsigned int row_bytes = png_get_rowbytes(png, info);
    unsigned char *data = new unsigned char[row_bytes * height];

    png_bytepp rows = png_get_rows(png, info);
    for (unsigned int i = 0; i < height; ++i)
    {
        memcpy(data + (row_bytes * i), rows[i], row_bytes);
    }

    png_destroy_read_struct(&png, &info, nullptr);

    return {width, height, data, ((color_type == PNG_COLOR_TYPE_RGBA) ? true : false)};
}

void deletePng(PngInfo &png)
{
    delete[] png.data;
}
