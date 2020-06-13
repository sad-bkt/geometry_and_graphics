#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

int quantity;
double sharpen;
char symbol;
int number, h, w, max_color;
vector<unsigned char> image;
//P6
vector<unsigned char> R_, G_, B_;
vector<double> R, G, B;
//P5
vector<double> RGB;
FILE *fin, *fout;

double Sat(double c)
{
    return min(1.0, max(0.0, c));
}

double mix(double x, double y, double a)
{
    return x * (1 - a) + y * a;
}

int main(int argc, char *argv[])
{
    //lab7.exe <input> <output> <sharpen>
    if (argc == 4)
    {
        try
        {
            sharpen = stof(argv[3]);
            if(sharpen < 0 || sharpen > 1)
            {
                cerr << "sharpen < 0 || sharpen > 1";
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
        RGB.resize(h * w, 0);
        quantity = fread(&image[0], sizeof(unsigned char), image.size(), fin);
        fclose(fin);
        if (quantity != h * w)
        {
            cerr << "Invalid pgm file: quantity != h * w";
            return 1;
        }
        for(int i = 0; i < h * w; i++)
        {
            RGB[i] = image[i] / 255.;
        }
    }
    else
    {
        image.resize(3 * h * w, 0);
        R.resize(h * w, 0);
        G.resize(h * w, 0);
        B.resize(h * w, 0);
        R_.resize(h * w, 0);
        G_.resize(h * w, 0);
        B_.resize(h * w, 0);
        quantity = fread(&image[0], sizeof(unsigned char), image.size(), fin);
        fclose(fin);
        if (quantity != 3 * h * w)
        {
            cerr << "Invalid pnm file: quantity != 3 * h * w";
            return 1;
        }
        for(int i = 0; i < quantity; i += 3)
        {
            R[i / 3] = image[i] / 255.;
            G[i / 3] = image[i + 1] / 255.;
            B[i / 3] = image[i + 2] / 255.;
            R_[i / 3] = image[i] ;
            G_[i / 3] = image[i + 1];
            B_[i / 3] = image[i + 2];
        }
    }

    //Contrast Adaptive Sharpening with better diagonals
    if (number == 5)
    {
        for (int y = 1; y < h - 1; ++y)
            for (int x = 1; x < w - 1; ++x)
            {
                int a = (y - 1) * w + x - 1;
                int b = (y - 1) * w + x;
                int c = (y - 1) * w + x + 1;
                int d = y * w + x - 1;
                int e = y * w + x;
                int f = y * w + x + 1;
                int g = (y + 1) * w + x - 1;
                int h = (y + 1) * w + x;
                int i = (y + 1) * w + x + 1;

                // Soft min and max.
                //  a b c             b
                //  d e f * 0.5  +  d e f * 0.5
                //  g h i             h
                // These are 2.0x bigger (factored out the extra multiply).
                double min_ = min(RGB[d], min(RGB[e], min(RGB[f], min(RGB[b], RGB[h]))));
                double min_2 = min(min_, min(RGB[a], min(RGB[c], (min(RGB[g], RGB[i])))));
                min_ += min_2;

                double max_ = max(RGB[d], max(RGB[e], max(RGB[f], max(RGB[b], RGB[h]))));
                double max_2 = max(max_, max(RGB[a], max(RGB[c], (max(RGB[g], RGB[i])))));
                max_ += max_2;

                double rcpM = 1. / max_;

                double amp= Sat(min(min_, 2.0 - max_) * rcpM);

                amp = sqrt(amp);

                // Filter shape.
                //  0 w 0
                //  w 1 w
                //  0 w 0
                //double peak=AF1_AU1(const1.x);
                double sharp = - 1. / (mix(8.0,5.0, Sat(sharpen)));
                double peak = sharp;
                double wRGB = amp * peak;

                double rcpWeight = 1. / (1.0 + 4.0 * wRGB);

                image[y * w + x] = (unsigned char)(255 * Sat((RGB[b] * wRGB + RGB[d] * wRGB + RGB[f] * wRGB + RGB[h] * wRGB + RGB[e]) * rcpWeight));
            }
    }
    else
    {
        for (int y = 1; y < h - 1; ++y)
            for (int x = 1; x < w - 1; ++x)
            {
                int a = (y - 1) * w + x - 1;
                int b = (y - 1) * w + x;
                int c = (y - 1) * w + x + 1;
                int d = y * w + x - 1;
                int e = y * w + x;
                int f = y * w + x + 1;
                int g = (y + 1) * w + x - 1;
                int h = (y + 1) * w + x;
                int i = (y + 1) * w + x + 1;

                double min_R = min(R[d], min(R[e], min(R[f], (min(R[b], R[h])))));
                double min_G = min(G[d], min(G[e], min(G[f], (min(G[b], G[h])))));
                double min_B = min(B[d], min(B[e], min(B[f], (min(B[b], B[h])))));
                double min_R2 = min(min_R, min(R[a], min(R[c], (min(R[g], R[i])))));
                double min_G2 = min(min_G, min(G[a], min(G[c], (min(G[g], G[i])))));
                double min_B2 = min(min_B, min(B[a], min(B[c], (min(B[g], B[i])))));
                min_R += min_R2;
                min_G += min_G2;
                min_B += min_B2;

                double max_R = max(R[d], max(R[e], max(R[f], (max(R[b], R[h])))));
                double max_G = max(G[d], max(G[e], max(G[f], (max(G[b], G[h])))));
                double max_B = max(B[d], max(B[e], max(B[f], (max(B[b], B[h])))));
                double max_R2 = max(max_R, max(R[a], max(R[c], (max(R[g], R[i])))));
                double max_G2 = max(max_G, max(G[a], max(G[c], (max(G[g], G[i])))));
                double max_B2 = max(max_B, max(B[a], max(B[c], (max(B[g], B[i])))));
                max_R += max_R2;
                max_G += max_G2;
                max_B += max_B2;

                // Soft min and max.
                //  a b c             b
                //  d e f * 0.5  +  d e f * 0.5
                //  g h i             h
                // These are 2.0x bigger (factored out the extra multiply).

                double rcpMR = 1. / max_R;
                double rcpMG = 1. / max_G;
                double rcpMB = 1. / max_B;

                double ampR= Sat(min(min_R, 2.0 - max_R) * rcpMR);
                double ampG= Sat(min(min_G, 2.0 - max_G) * rcpMG);
                double ampB = Sat(min(min_B, 2.0 - max_B) * rcpMB);

                ampR = sqrt(ampR);
                ampG = sqrt(ampG);
                ampB = sqrt(ampB);

                // Filter shape.
                //  0 w 0
                //  w 1 w
                //  0 w 0
                //double peak=AF1_AU1(const1.x);
                double sharp = - 1. / (mix(8.0,5.0, Sat(sharpen)));
                double peak = sharp;
                double wR = ampR * peak;
                double wG = ampG * peak;
                double wB = ampB * peak;

                double rcpWeightR = 1. / (1.0 + 4.0 * wR);
                double rcpWeightG = 1. / (1.0 + 4.0 * wG);
                double rcpWeightB = 1. / (1.0 + 4.0 * wB);

                R_[y * w + x] = (unsigned char)(255 * Sat((R[b] * wR + R[d] * wR + R[f] * wR + R[h] * wR + R[e]) * rcpWeightR));
                G_[y * w + x] = (unsigned char)(255 * Sat((G[b] * wG + G[d] * wG + G[f] * wG + G[h] * wG + G[e]) * rcpWeightG));
                B_[y * w + x] = (unsigned char)(255 * Sat((B[b] * wB + B[d] * wB + B[f] * wB + B[h] * wB + B[e]) * rcpWeightB));
            }

        for(int i = 0; i < h * w; i++)
        {
            image[3 * i] = R_[i];
            image[3 * i + 1] = G_[i];
            image[3 * i + 2] = B_[i];
        }
    }


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