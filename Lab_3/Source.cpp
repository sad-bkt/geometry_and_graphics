#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

int grad, dithering, bitness;
double gamma_;
char symbol;
int number, h, w, max_color;
vector<unsigned char> image1, image2;
vector<double> image1_;
FILE *fin, *fout;

double gamma_correction(double color)
{
    if (gamma_ == 0)//sRGB
    {
        if (color <= 0.04045)
            color = color / 12.92;
        else
            color = pow((color + 0.055) / 1.055, 2.4);
    }
    else
    {
        color = pow(color, gamma_);
    }
    return color * 255;

}
unsigned char find_nearest_palette_color(double color)
{
    color = gamma_correction(color);
    return (unsigned char)min(max(color, 0.), 255.);
}


unsigned char change_bitness(unsigned char color)
{
    unsigned char new_ = (((1u << bitness) - 1) << (8 - bitness)) & color; //11111111->11...000->color...00
    color = 0;
    for (int i = 0; i <= 8 / bitness; i++)
        color |= (unsigned char)(new_ >> bitness * i);
    return color;
}

unsigned char change_bitness_for_random(unsigned char color)
{
    unsigned char color1 = color, color2;
    unsigned char new_ = (((1u << bitness) - 1) << (8 - bitness)) & color; //11111111->11...000->color...00
    unsigned char new__ = new_;
    color2 = 0;
    for (int i = 0; i <= 8 / bitness; i++)
        color2 |= (unsigned char)(new_ >> bitness * i);

    if((double)rand() / (double)RAND_MAX > 0.5)
    {
        if(color2 > color1)
            return color2;
        else
        {
            if((new__ >> (8 - bitness)) != (1 << (bitness)) - 1)
            {
                new_ = ((new__ >> (8 - bitness)) + 1) << (8 - bitness);
                color2 = 0;
                for (int i = 0; i <= 8 / bitness; i++)
                    color2 |= (unsigned char)(new_ >> bitness * i);
            }
            return color2;
        }
    }
    else
    {
        if(color2 > color1)
            return color1;
        else
        {
            if((int)new__ != 0)
            {
                new_ = ((new__ >> (8 - bitness)) - 1) << (8 - bitness);
                color2 = 0;
                for (int i = 0; i <= 8 / bitness; i++)
                    color2 |= (unsigned char)(new_ >> bitness * i);
            }
            return color2;
        }
    }


}

void dithering_no()
{
    //if(bitness != 8)
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            image2[y * w + x] = change_bitness(find_nearest_palette_color(image1_[y * w + x]));
}

void dithering_ordered()
{
    const int n = 8;
    double m[n][n] = {
            { 0, 48, 12, 60,  3, 51, 15, 63},
            {32, 16, 44, 28, 35, 19, 47, 31},
            { 8, 56,  4, 52, 11, 59,  7, 55},
            {40, 24, 36, 20, 43, 27, 39, 23},
            { 2, 50, 14, 62,  1, 49, 13, 61},
            {34, 18, 46, 30, 33, 17, 45, 29},
            {10, 58,  6, 54,  9, 57,  5, 53},
            {42, 26, 38, 22, 41, 25, 37, 21},
    };
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            image2[y * w + x] = change_bitness(
                    find_nearest_palette_color
                            (
                                    (image1_[y * w + x] + ((m[x % n][y % n] + 1) / 64 - 0.5))
                            )
            );
}

void dithering_random()
{
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            image2[y * w + x] = change_bitness(
                    find_nearest_palette_color
                            (
                                    (image1_[y * w + x] + 2 * ((double)rand() / (double)RAND_MAX - 0.5) * ((1 << (8 - bitness)) * 1. / 256.))
                            )
            );
//            image1_[y * w + x] += ((double)rand() / (double)RAND_MAX - 0.5) * ((1 << (8 - bitness)) * 1. / 256.);
//            image1_[y * w + x] = min(max(image1_[y * w + x], 0.), 1.);
//            image1_[y * w + x] = change_bitness_for_random(255 * image1_[y * w + x]) / 255.;
//            image2[y * w + x] = find_nearest_palette_color(image1_[y * w + x]);

//            image1_[y * w + x] = gamma_correction(image1_[y * w + x]);
//            if((double)rand() / (double)RAND_MAX > 0.5)
//                image1_[y * w + x] = ceil(image1_[y * w + x]);
//            else
//                image1_[y * w + x] = floor(image1_[y * w + x]);
//            image1_[y * w + x] = (unsigned char)min(max(image1_[y * w + x], 0.), 255.);
//            image2[y * w + x] = change_bitness(image1_[y * w + x]);
        }
}

void dithering_floyd_steinberg()
{
    //  X 7
    //3 5 1
    //*(1/16)
    vector<double> error;
    error.resize(h * w, 0);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            unsigned char tmp = find_nearest_palette_color
                    (
                            (image1_[y * w + x] + error[y * w + x])
                    );
            image2[y * w + x] = change_bitness(tmp);
            double quant_error = (tmp - image2[y * w + x]) / 255.;

            if (x < w - 1)
                error[y * w + x + 1] += quant_error * 7. / 16.;
            if (y < h - 1)
            {
                if (x < w - 1)
                    error[(y + 1) * w + x + 1] += quant_error / 16.;
                error[(y + 1) * w + x] += quant_error * 5. / 16.;
                if (x > 0)
                    error[(y + 1) * w + x - 1] += quant_error * 3. / 16.;
            }
        }
}

void dithering_jarvis_judice_ninke()
{
    //    X 7 5
    //3 5 7 5 3
    //1 3 5 3 1
    // (1/48)
    vector<double> error;
    error.resize(h * w, 0);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            unsigned char tmp = find_nearest_palette_color
                    (
                            (image1_[y * w + x] + error[y * w + x])
                    );
            image2[y * w + x] = change_bitness(tmp);
            double quant_error = (tmp - image2[y * w + x]) / 255.;
            if (x < w - 1)
            {
                if (x < w - 2)
                    error[(y)* w + x + 2] += quant_error * 5. / 48.;
                error[(y)* w + x + 1] += quant_error * 7. / 48.;
            }
            if (y < h - 1)
            {
                if (x < w - 2)
                    error[(y + 1) * w + x + 2] += quant_error * 3. / 48.;
                if (x < w - 1)
                    error[(y + 1) * w + x + 1] += quant_error * 5. / 48.;
                error[(y + 1) * w + x] += quant_error * 7. / 48.;
                if (x > 0)
                    error[(y + 1) * w + x - 1] += quant_error * 5. / 48.;
                if (x > 1)
                    error[(y + 1) * w + x - 2] += quant_error * 3. / 48.;
                if (y < h - 2)
                {
                    if (x < w - 2)
                        error[(y + 2) * w + x + 2] += quant_error / 48.;
                    if (x < w - 1)
                        error[(y + 2) * w + x + 1] += quant_error * 3. / 48.;
                    error[(y + 2) * w + x] += quant_error * 5. / 48.;
                    if (x > 0)
                        error[(y + 2) * w + x - 1] += quant_error * 3. / 48.;
                    if (x > 1)
                        error[(y + 2) * w + x - 2] += quant_error / 48.;
                }
            }
        }
}

void dithering_sierra()
{
    //    X 5 3
    //2 4 5 4 2
    //  2 3 2
    //  (1/32)
    vector<double> error;
    error.resize(h * w, 0);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            unsigned char tmp = find_nearest_palette_color
                    (
                            (image1_[y * w + x] + error[y * w + x])
                    );
            image2[y * w + x] = change_bitness(tmp);
            double quant_error = (tmp - image2[y * w + x]) / 255.;
            if (x < w - 1)
            {
                if (x < w - 2)
                    error[(y)* w + x + 2] += quant_error * 3. / 32.;
                error[(y)* w + x + 1] += quant_error * 5. / 32.;
            }
            if (y < h - 1)
            {
                if (x < w - 2)
                    error[(y + 1) * w + x + 2] += quant_error * 2. / 32.;
                if (x < w - 1)
                    error[(y + 1) * w + x + 1] += quant_error * 4. / 32.;
                error[(y + 1) * w + x] += quant_error * 5 / 32.;
                if (x > 0)
                    error[(y + 1) * w + x - 1] += quant_error * 4. / 32.;
                if (x > 1)
                    error[(y + 1) * w + x - 2] += quant_error * 2. / 32.;
                if (y < h - 2)
                {
                    if (x < w - 1)
                        error[(y + 2) * w + x + 1] += quant_error * 2. / 32.;
                    error[(y + 2) * w + x] += quant_error * 3. / 32.;
                    if (x > 0)
                        error[(y + 2) * w + x - 1] += quant_error * 2. / 32.;
                }
            }
        }
}

void dithering_atkinson()
{
    //  X 1 1
    //1 1 1
    //  1
    //  (1/8)
    vector<double> error;
    error.resize(h * w, 0);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            unsigned char tmp = find_nearest_palette_color
                    (
                            (image1_[y * w + x] + error[y * w + x])
                    );
            image2[y * w + x] = change_bitness(tmp);
            double quant_error = (tmp - image2[y * w + x]) / 255.;
            if (x < w - 1)
            {
                if (x < w - 2)
                    error[(y)* w + x + 2] += quant_error / 8.;
                error[(y)* w + x + 1] += quant_error / 8.;
            }
            if (y < h - 1)
            {
                if (x < w - 1)
                    error[(y + 1) * w + x + 1] += quant_error / 8.;
                error[(y + 1) * w + x] += quant_error / 8.;
                if (x > 0)
                    error[(y + 1) * w + x - 1] += quant_error / 8.;
                if (y < h - 2)
                    error[(y + 2) * w + x] += quant_error / 8.;
            }
        }
}

void dithering_halftone()
{
    const int n = 4;
    double m[n][n] = {
            { 7, 13, 11, 4},
            {12, 16, 14, 8},
            {10, 15,  6, 2},
            { 5,  9,  3, 1},
    };

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            image2[y * w + x] = change_bitness(
                    find_nearest_palette_color
                            (
                                    (image1_[y * w + x] + ((m[x % n][y % n]) - 8.5)/ 16)
                            )
            );
}

int main(int argc, char *argv[])
{
    //printf("PROCESSING\n");

    //program.exe <имя_входного_файла> <имя_выходного_файла> <градиент> <дизеринг> <битность> <гамма>

    if (argc == 7)
    {
        try
        {
            grad = stoi(argv[3]);
            if (grad != 0 && grad != 1)
            {
                cerr << "Gradient != 1 and gradient != 0";
                return 1;
            }
            dithering = stoi(argv[4]);
            if (dithering < 0 || dithering > 7)
            {
                cerr << "Dithering should be in [0..7]";
                return 1;
            }
            bitness = stoi(argv[5]);
            if (bitness < 1 || bitness > 8)
            {
                cerr << "Bit should be in [1..8]";
                return 1;
            }
            gamma_ = stod(argv[6]);
            if (gamma_ < 0)
            {
                cerr << "Gamma should be >= 0";
                return 1;
            }
        }
        catch(...)
        {
            cerr << "Invalid arguments";
            return 1;
        }
    }
    else
    {
        cerr << "Invalid number of arguments";
        return 1;
    }

    fin = fopen(argv[1], "rb");
    if (fin == NULL)
    {
        cerr << "Input file can't be open";
        return 1;
    }
    int quantity = fscanf(fin, "%c%d\n%d%d\n%d\n", &symbol, &number, &w, &h, &max_color);
    if (quantity != 5 || symbol != 'P' || number != 5 || h <= 0 || w <= 0 || max_color <= 0 || max_color > 255)
    {
        fclose(fin);
        cerr << "Invalid pnm file";
        return 1;
    }
    if (grad == 0)
    {
        image1.resize(h * w, 0);
        image1_.resize(h * w, 0.);
        image2.resize(h * w, 0);
        quantity = fread(&image1[0], sizeof(unsigned char), image1.size(), fin);
        if (quantity != h * w)
        {
            fclose(fin);
            cerr << "Quantity != width * height";
            return 1;
        }
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                image1_[y * w + x] = (double)image1[y * w + x] / 255.;
    }
    else
    {
        image1.resize(h * w, 0);
        image1_.resize(h * w, 0.);
        image2.resize(h * w, 0);
        //рисую градиент
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                image1_[y * w + x] = (double)x / (double)w * 256. / 255.;
    }
    fclose(fin);
    switch (dithering)
    {
        case 0:
            dithering_no();
            break;
        case 1:
            dithering_ordered();
            break;
        case 2:
            dithering_random();
            break;
        case 3:
            dithering_floyd_steinberg();
            break;
        case 4:
            dithering_jarvis_judice_ninke();
            break;
        case 5:
            dithering_sierra();
            break;
        case 6:
            dithering_atkinson();
            break;
        case 7:
            dithering_halftone();
            break;
        default:
            cerr << "Dithering should be in [0..7]";
            return 1;
    }

    fout = fopen(argv[2], "wb");
    if (fout == NULL)
    {
        cerr << "Output file can't be open";
        return 1;
    }
    fprintf(fout, "%c%d\n%d %d\n%d\n", symbol, number, w, h, max_color);
    quantity = fwrite(&image2[0], sizeof(unsigned char), image2.size(), fout);
    fclose(fout);
    if (quantity != h * w)
    {
        cerr << "Quantity of outputfile != width * height";
        return 1;
    }

    //printf("DONE");
    //system("pause");
    return 0;
}