#ifndef TGAIMAGE_HPP
#define TGAIMAGE_HPP

#include <fstream>

static_assert(true);
#pragma pack(push, 1)
struct TGAHeader
{
    char idLength;
    char colorMapType;
    char dataTypeCode;
    short colorMapOrigin;
    short colorMapLength;
    char colorMapDepth;
    short originX;
    short originY;
    short width;
    short height;
    char bitsPerPixel;
    char imageDescriptor;
};
#pragma pack(pop)

class TGAColor
{
   public:
    union
    {
        struct
        {
            unsigned char b, g, r, a;
        };
        unsigned char raw[4];
        unsigned int val;
    };
    int bytespp;

    TGAColor();
    TGAColor(unsigned char R, unsigned char G, unsigned char B,
             unsigned char A);
    TGAColor(int v, int bpp);
    TGAColor(const TGAColor& c);
    TGAColor(const unsigned char* p, int bpp);
    TGAColor& operator=(const TGAColor& c);
};

class TGAImage
{
   public:
    enum FORMAT
    {
        GRAYSCALE = 1,
        RGB = 3,
        RGBA = 4
    };

    TGAImage();
    TGAImage(int w, int h, int bpp);
    TGAImage(const TGAImage& img);
    ~TGAImage();

    bool readTGAFile(const char* file);
    bool writeTGAFile(const char* file, bool rle = true);
    bool flipHorizontally();
    bool flipVertically();
    bool scale(int w, int h);
    TGAColor get(int x, int y);
    bool set(int x, int y, TGAColor c);
    int getWidth();
    int getHeight();
    int getBytesPP();
    unsigned char* buffer();
    void clear();

    TGAImage& operator=(const TGAImage& img);

   protected:
    unsigned char* data;
    int width;
    int height;
    int bytespp;

    bool loadRLEData(std::ifstream& in);
    bool unloadRLEData(std::ofstream& out);
};

#endif
