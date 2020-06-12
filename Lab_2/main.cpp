#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

int brightness;
double thickness, x_s, x_f, y_s, y_f, gamma;
char symbol;
int number, h, w, max_color;
std::vector<unsigned char> image1, image2;
FILE *fin, *fout;


int main(int argc, char *argv[])
{
    //program.exe <имя_входного_файла> <имя_выходного_файла> <яркость_линии> <толщина_линии> <x_начальный> <y_начальный> <x_конечный> <y_конечный> <гамма>
    if (argc == 10 || argc == 9)
    {
        try
        {
            brightness = atoi(argv[3]);
            if (brightness < 0 || brightness > 255)
            {
                cerr << "Brightness should be in [0..255]";
                return 1;
            }
            thickness = atof(argv[4]);
            if (thickness <= 0)
            {
                cerr << "Thickness should be > 0";
                return 1;
            }
            x_s = atof(argv[5]);
            y_s = atof(argv[6]);
            x_f = atof(argv[7]);
            y_f = atof(argv[8]);
            if(argc == 10)
            {
                gamma = atof(argv[9]);
                if(gamma <= 0)
                {
                    cerr << "Gamma should be > 0";
                    return 1;
                }
            }
            else
                gamma = 0;
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
    if (x_s < 0 || x_s >= w || x_f < 0 || x_f >= w || y_s < 0 || y_s >= h || y_f < 0 || y_f >= h)
    {
        fclose(fin);
        cerr << "Incorrect coordinates";
        return 1;
    }
    image1.resize(h * w, 0);
    image1_.resize(h * w, 0.);
    image2.resize(h * w, 0);
    quantity = fread(&image1[0], sizeof(unsigned char), image1.size(), fin);
    fclose(fin);
    if (quantity != h * w)
    {
        cerr << "Quantity != width * height";
        return 1;
    }
    gamma_correction_from_file();





    gamma_correction_to_file();

    fout = fopen(argv[2], "wb");
    if (fout == NULL)
    {
        cerr << "Output file can't be open";
        return 1;
    }
    fprintf(fout, "%c%d\n%d %d\n%d\n", symbol, number, w, h, max_color);
    fwrite(&image2[0], sizeof(unsigned char), image2.size(), fout);
    fclose(fout);
    return 0;
}