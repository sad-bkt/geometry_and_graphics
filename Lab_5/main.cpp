#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

int transformation, offset, quantity;
double multiplier;
char symbol;
int number, h, w, max_color;
vector<unsigned char> image;
FILE *fin, *fout;

unsigned char find_nearest(double color)
{
    return (unsigned char)min(max(color, 0.), 255.);
}

void from_RGB_to_YCbCr_601()
{
    double K_r = 0.299;
    double K_g = 0.587;
    double K_b = 0.114;
    for(int i = 0; i < 3 * h * w; i++)
    {
        double R = image[i] / 255.;
        double G = image[i + 1] / 255.;
        double B = image[i + 2] / 255.;
        double Y =   K_r * R + K_g * G + K_b * B;
        double C_b = 0.5 * (B - Y) / (1 - K_b);
        double C_r = 0.5 * (R - Y) / (1 - K_r);
        image[i] = find_nearest(255 * Y);
        image[i + 1] = find_nearest(255 * (C_b + 0.5));
        image[i + 2] = find_nearest(255 * (C_r + 0.5));
    }
}

void from_YCbCr_601_to_RGB()
{
    double K_r = 0.299;
    double K_g = 0.587;
    double K_b = 0.114;
    for(int i = 0; i < 3 * h * w; i++)
    {
        double Y = image[i] / 255.;
        double C_b = image[i + 1] / 255. - 0.5;
        double C_r = image[i + 2] / 255. - 0.5;
        double R = Y +  C_r * 2 * (1 - K_r);
        double B = Y + C_b * 2 * (1 - K_b);
        double G = (Y - K_r * R - K_b * B) / K_g;
        image[i] = find_nearest(255 * R);
        image[i + 1] = find_nearest(255 * G);
        image[i + 2] = find_nearest(255 * B);
    }
}


int main(int argc, char *argv[])
{
    //lab5.exe <имя_входного_файла> <имя_выходного_файла> <преобразование> [<смещение> <множитель>]
    if (argc == 4 || argc == 6)
    {
        try
        {
            transformation = stoi(argv[3]);
            if(transformation < 0 || transformation > 5)
            {
                cerr << "transformation < 0 || transformation > 5";
                return 1;
            }
            if(transformation == 0 || transformation == 1)
            {
                offset = stoi(argv[4]);
                if(offset < -255 || offset > 255)
                {
                    cerr << "offset < -255 || offset > 255";
                    return 1;
                }
                multiplier = stof(argv[5]);
                if(multiplier < 1./255. || multiplier > 255)
                {
                    cerr << "multiplier < 1./255. || multiplier > 255";
                    return 1;
                }
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
    quantity = fscanf(fin, "%c%d\n%d%d\n%d\n", &symbol, &number, &w, &h, &max_color);
    if (quantity != 5 || symbol != 'P' || (number != 5 && number != 6) || h <= 0 || w <= 0 || max_color <= 0 || max_color > 255)
    {
        fclose(fin);
        cerr << "Invalid ppm file";
        return 1;
    }
    if (number == 5)
    {
        image.resize(h * w, 0);
        quantity = fread(&image[0], sizeof(unsigned char), image.size(), fin);
        if (quantity != h * w)
        {
            fclose(fin);
            cerr << "Invalid pgm file: quantity != h * w";
            return 1;
        }
    }
    else
    {
        image.resize(3 * h * w, 0);
        quantity = fread(&image[0], sizeof(unsigned char), image.size(), fin);
        if (quantity != 3 * h * w)
        {
            fclose(fin);
            cerr << "Invalid pnm file: quantity != 3 * h * w";
            return 1;
        }
    }

    int plus = 1;
    if(transformation % 2 == 1 && number == 6)//YCbCr P6
    {
        plus = 3;
        from_RGB_to_YCbCr_601();
    }

    if(transformation != 0 && transformation != 1)//нахожу offset, multiplier
    {
        //можно отсортировать, но это дольше
        int colors[256] = {0};
        for(int i = 0; i < quantity; i += plus)
            colors[image[i]]++;
        int ignor = 0;
        if(transformation == 4 || transformation == 5)
            ignor = ceil(0.39 * quantity / plus / 100.);

        int sum = 0, i = 0;
        while(sum <= ignor)
        {
            sum += colors[i];
            i++;
        }
        int min_ = i - 1;

        sum = 0;
        i = 255;
        while(sum <= ignor)
        {
            sum += colors[i];
            i--;
        }
        int max_ = i + 1;

        offset = min_;
        multiplier = 255. / (max_ - min_);
        cout << offset << " " << multiplier;
    }
    for(int i = 0; i < quantity; i += plus)
        image[i] = find_nearest((image[i] - offset) * multiplier);

    if(transformation % 2 == 1 && number == 6)//YCbCr P6
        from_YCbCr_601_to_RGB();


    fout = fopen(argv[2], "wb");
    if (fout == NULL)
    {
        cerr << "Output file can't be open";
        return 1;
    }
    fprintf(fout, "%c%d\n%d %d\n%d\n", symbol, number, w, h, max_color);
    int quantity_out = fwrite(&image[0], sizeof(unsigned char), image.size(), fout);
    fclose(fout);
    if (quantity != quantity_out)
    {
        cerr << "Quantity of output file != width * height";
        return 1;
    }
    return 0;
}