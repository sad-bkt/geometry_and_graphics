#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

int count_in, count_out;
char symbol;
string color_space_from, color_space_to;
//const char*
string input_f, output_f;
int number, h, w, max_color;
vector<unsigned char> image1, image2, image3, image;
FILE *fin, *fout;

unsigned char find_nearest(double color)
{
    return (unsigned char)min(max(color, 0.), 255.);
}

void from_HSL_to_RGB()
{
    for(int i = 0; i < h * w; i++)
    {
        double H = image1[i] * 360. / 255.;
        double S = image2[i] / 255.;
        double L = image3[i] / 255.;
        double C = (1 - abs(2 * L - 1)) * S;
        double H_ = H / 60.;
        double X = C * (1 - abs((fmod(H_, 2.) - 1)));
        double R1, G1, B1;
        if (ceil(H_) - 1. < 0.00001)
        {
            R1 = C;
            G1 = X;
            B1 = 0;
        }
        else if (ceil(H_) - 2. < 0.00001)
        {
            R1 = X;
            G1 = C;
            B1 = 0;
        }
        else if (ceil(H_) - 3. < 0.00001)
        {
            R1 = 0;
            G1 = C;
            B1 = X;
        }
        else if (ceil(H_) - 4. < 0.00001)
        {
            R1 = 0;
            G1 = X;
            B1 = C;
        }
        else if (ceil(H_) - 5. < 0.00001)
        {
            R1 = X;
            G1 = 0;
            B1 = C;
        }
        else if (ceil(H_) - 6. < 0.00001)
        {
            R1 = C;
            G1 = 0;
            B1 = X;
        }
        else
        {
            R1 = 0;
            G1 = 0;
            B1 = 0;
        }
        double m = L - C / 2.;
        image1[i] = find_nearest(255 * (R1 + m));
        image2[i] = find_nearest(255 * (G1 + m));
        image3[i] = find_nearest(255 * (B1 + m));
    }
}

void from_HSV_to_RGB()
{
    for(int i = 0; i < h * w; i++)
    {
        double H = image1[i] * 360. / 255.;
        double S = image2[i] / 255.;
        double V = image3[i] / 255.;
        double C = V * S;
        double H_ = H / 60.;
        double X = C * (1 - abs((fmod(H_, 2.) - 1)));
        double R1, G1, B1;
        if (0 <= H_ && H_ <= 1)
        {
            R1 = C;
            G1 = X;
            B1 = 0;
        }
        else if (1 < H_ && H_ <= 2)
        {
            R1 = X;
            G1 = C;
            B1 = 0;
        }
        else if (2 < H_ && H_ <= 3)
        {
            R1 = 0;
            G1 = C;
            B1 = X;
        }
        else if (3 < H_ && H_ <= 4)
        {
            R1 = 0;
            G1 = X;
            B1 = C;
        }
        else if (4 < H_ && H_ <= 5)
        {
            R1 = X;
            G1 = 0;
            B1 = C;
        }
        else if (5 < H_ && H_ <= 6)
        {
            R1 = C;
            G1 = 0;
            B1 = X;
        }
        else // H undefined
        {
            R1 = 0;
            G1 = 0;
            B1 = 0;
        }
        double m = V - C;
        image1[i] = find_nearest(255 * (R1 + m));
        image2[i] = find_nearest(255 * (G1 + m));
        image3[i] = find_nearest(255 * (B1 + m));
    }
}

void from_YCbCr_601_to_RGB()
{
    double K_r = 0.299;
    double K_g = 0.587;
    double K_b = 0.114;
    for(int i = 0; i < h * w; i++)
    {
        double Y = image1[i] / 255.;
        double C_b = image2[i] / 255. - 0.5;
        double C_r = image3[i] / 255. - 0.5;
        double R = Y +  C_r * 2 * (1 - K_r);
        double B = Y + C_b * 2 * (1 - K_b);
        double G = (Y - K_r * R - K_b * B) / K_g;
        image1[i] = find_nearest(255 * R);
        image2[i] = find_nearest(255 * G);
        image3[i] = find_nearest(255 * B);
    }
}

void from_YCbCr_709_to_RGB()
{
    double K_b = 0.0722;
    double K_r = 0.2126;
    double K_g = 0.7152;//1 - K_b - K_r
    for(int i = 0; i < h * w; i++)
    {
        double Y = image1[i] / 255.;
        double C_b = image2[i] / 255. - 0.5;
        double C_r = image3[i] / 255. - 0.5;
        double R = Y +  C_r * 2 * (1 - K_r);
        double B = Y + C_b * 2 * (1 - K_b);
        double G = (Y - K_r * R - K_b * B) / K_g;
        image1[i] = find_nearest(255 * R);
        image2[i] = find_nearest(255 * G);
        image3[i] = find_nearest(255 * B);
    }
}

void from_YCoCg_to_RGB()
{
    for(int i = 0; i < h * w; i++)
    {
        double Y = image1[i] / 255.;
        double Co = image2[i] / 255. - 0.5;
        double Cg = image3[i] / 255. - 0.5;
        image1[i] = find_nearest(255 * (Y + Co - Cg));
        image2[i] = find_nearest(255 * (Y + Cg));
        image3[i] = find_nearest(255 * (Y - Co - Cg));
    }
}

void from_CMY_to_RGB()
{
    for(int i = 0; i < h * w; i++)
    {
        image1[i] = 255 - image1[i];
        image2[i] = 255 - image2[i];
        image3[i] = 255 - image3[i];
    }
}

void from_RGB_to_HSL()
{
    for(int i = 0; i < h * w; i++)
    {
        double R = image1[i] / 255.;
        double G = image2[i] / 255.;
        double B = image3[i] / 255.;
        double X_max = max(R, max(G, B));
        double X_min = min(R, min(G, B));
        double V = X_max;
        double L = (X_max + X_min) / 2.;
        double C = X_max - X_min;
        double H;
        if (C == 0)
            H = 0;
        else if (V == R)
            H = 60. * (G - B) / C;
        else if (V == G)
            H = 60. * (2 + (B - R) / C);
        else //if(V == B)
            H = 60. * (4 + (R - G) / C);
        if (H < 0) // V == R
            H += 360;
        double S;
        if(L== 0 || L == 1)
            S = 0;
        else
        {
            //S = 2 * (V - L) / (1 - abs(2 * L- 1));
            S = (V - L) / min(L, 1 - L);
        }

        image1[i] = find_nearest(255 * (H / 360.));
        image2[i] = find_nearest(255 * S);
        image3[i] = find_nearest(255 * L);
    }
}

void from_RGB_to_HSV()
{
    for(int i = 0; i < h * w; i++)
    {
        double R = image1[i] / 255.;
        double G = image2[i] / 255.;
        double B = image3[i] / 255.;
        double X_max = max(R, max(G, B));
        double X_min = min(R, min(G, B));
        double V = X_max;
        double C = V - X_min;
        double H;
        if (C == 0)
            H = 0;
        else if (V == R)
            H = 60. * (G - B) / C;
        else if (V == G)
            H = 60. * (2 + (B - R) / C);
        else if(V == B)
            H = 60. * (4 + (R - G) / C);
        if (H < 0)
            H += 360;
        double S;
        if(V == 0)
            S = 0;
        else
            S = C / V;
        image1[i] = find_nearest(255 * (H / 360.));
        image2[i] = find_nearest(255 * S);
        image3[i] = find_nearest(255 * V);
    }
}

void from_RGB_to_YCbCr_601()
{
    double K_r = 0.299;
    double K_g = 0.587;
    double K_b = 0.114;
    for(int i = 0; i < h * w; i++)
    {
        double R = image1[i] / 255.;
        double G = image2[i] / 255.;
        double B = image3[i] / 255.;
        double Y =   K_r * R + K_g * G + K_b * B;
        double C_b = 0.5 * (B - Y) / (1 - K_b);
        double C_r = 0.5 * (R - Y) / (1 - K_r);
        image1[i] = find_nearest(255 * Y);
        image2[i] = find_nearest(255 * (C_b + 0.5));
        image3[i] = find_nearest(255 * (C_r + 0.5));
    }
}

void from_RGB_to_YCbCr_709()
{
    double K_b = 0.0722;
    double K_r = 0.2126;
    double K_g = 0.7152;//1 - K_b - K_r
    for(int i = 0; i < h * w; i++)
    {
        double R = image1[i] / 255.;
        double G = image2[i] / 255.;
        double B = image3[i] / 255.;
        double Y =   K_r * R + K_g * G + K_b * B;
        double C_b = 0.5 * (B - Y) / (1 - K_b);
        double C_r = 0.5 * (R - Y) / (1 - K_r);
        image1[i] = find_nearest(255 * Y);
        image2[i] = find_nearest(255 * (C_b + 0.5));
        image3[i] = find_nearest(255 * (C_r + 0.5));
    }
}

void from_RGB_to_YCoCg()
{
    for(int i = 0; i < h * w; i++)
    {
        double R = image1[i] / 255.;
        double G = image2[i] / 255.;
        double B = image3[i] / 255.;
        image1[i] = find_nearest(255 * (R / 4. + G / 2. + B / 4.));
        image2[i] = find_nearest(255 * (R / 2. - B / 2. + 0.5)); // Co in[-0.5..0.5]
        image3[i] = find_nearest(255 * (- R / 4. + G / 2. - B / 4. + 0.5));// Cg in[-0.5..0.5]
    }
}

void from_RGB_to_CMY()
{
    for(int i = 0; i < h * w; i++)
    {
        image1[i] = 255 - image1[i];
        image2[i] = 255 - image2[i];
        image3[i] = 255 - image3[i];
    }
}


int main(int argc, char *argv[])
{
    //lab4.exe -f <from_color_space> -t <to_color_space> -i <count_> <input_file_name> -o <count_> <output_file_name>
    if (argc == 11)
    {
        try
        {
            int i = 1;
            while(i < 11)
            {
                string s = argv[i];
                if(s == "-f")
                {
                    color_space_from = argv[i + 1];
                    if(color_space_from != "RGB" && color_space_from != "HSL" && color_space_from != "HSV" && color_space_from != "YCbCr.601" && color_space_from != "YCbCr.709" && color_space_from != "YCoCg" && color_space_from != "CMY")
                    {
                        cerr << "Wrong color space";
                        return 1;
                    }
                    i += 2;
                }
                else if(s == "-t")
                {
                    color_space_to = argv[i + 1];
                    if(color_space_to != "RGB" && color_space_to != "HSL" && color_space_to != "HSV" && color_space_to != "YCbCr.601" && color_space_to != "YCbCr.709" && color_space_to != "YCoCg" && color_space_to != "CMY")
                    {
                        cerr << "Wrong color space";
                        return 1;
                    }
                    i += 2;
                }
                else if(s == "-i")
                {
                    count_in = stoi(argv[i + 1]);
                    input_f = argv[i + 2];
                    if(count_in != 1 && count_in != 3)
                    {
                        cerr << "count_ != 1 && count_ != 3";
                        return 1;
                    }
                    i += 3;
                }
                else if(s == "-o")
                {
                    count_out = stoi(argv[i + 1]);
                    output_f = argv[i + 2];
                    if(count_out != 1 && count_out != 3)
                    {
                        cerr << "count_ != 1 && count_ != 3";
                        return 1;
                    }
                    i += 3;
                }
                else
                {
                    cerr << "Wrong commands -i, -o, -f, -t "<< argv[i];
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


    if (count_in == 3)
    {
        for(int i = 1; i < 4; i++)
        {
            string name_of_file = input_f;
            if(i == 1)
                name_of_file.insert( name_of_file.find("."), "_1");
            else if(i == 2)
                name_of_file.insert( name_of_file.find("."), "_2");
            else
                name_of_file.insert( name_of_file.find("."), "_3");
            char *name = &name_of_file[0];
            fin = fopen(name, "rb");
            if (fin == NULL)
            {
                cerr << "Input file can't be open";
                return 1;
            }
            int quantity = fscanf(fin, "%c%d\n%d%d\n%d\n", &symbol, &number, &w, &h, &max_color);
            if (quantity != 5 || symbol != 'P' || number != 5 || h <= 0 || w <= 0 || max_color <= 0 || max_color > 255)
            {
                fclose(fin);
                cerr << "Invalid pgm file: quantity != h * w";
                return 1;
            }
            if(i == 1)
            {
                image1.resize(h * w, 0);
                quantity = fread(&image1[0], sizeof(unsigned char), image1.size(), fin);
            }
            else if(i == 2)
            {
                image2.resize(h * w, 0);
                quantity = fread(&image2[0], sizeof(unsigned char), image2.size(), fin);
            }

            else
            {
                image3.resize(h * w, 0);
                quantity = fread(&image3[0], sizeof(unsigned char), image3.size(), fin);
            }
            fclose(fin);
            if (quantity != h * w)
            {
                cerr << "Invalid pgm file";
                return 1;
            }
            image.resize(3 * h * w, 0);
        }
    }
    else
    {
        char *name = &input_f[0];
        fin = fopen(name, "rb");
        if (fin == NULL)
        {
            cerr << "Input file can't be open";
            return 1;
        }
        int quantity = fscanf(fin, "%c%d\n%d%d\n%d\n", &symbol, &number, &w, &h, &max_color);
        if (quantity != 5 || symbol != 'P' || number != 6 || h <= 0 || w <= 0 || max_color <= 0 || max_color > 255)
        {
            fclose(fin);
            cerr << "Invalid ppm file";
            return 1;
        }
        image.resize(3 * h * w, 0);
        image1.resize(h * w, 0);
        image2.resize(h * w, 0);
        image3.resize(h * w, 0);
        quantity = fread(&image[0], sizeof(unsigned char), image.size(), fin);
        fclose(fin);
        if (quantity != 3 * h * w)
        {
            cerr << "Invalid ppm file: quantity != 3 * h * w";
            return 1;
        }
        for(int i = 0; i < 3 * h * w; i += 3)
        {
            image1[i / 3] = image[i];
            image2[i / 3] = image[i + 1];
            image3[i / 3] = image[i + 2];
        }
    }


    if(color_space_from == "RGB")
    {
    }
    else if(color_space_from == "HSL")
        from_HSL_to_RGB();
    else if(color_space_from == "HSV")
        from_HSV_to_RGB();
    else if(color_space_from == "YCbCr.601")
        from_YCbCr_601_to_RGB();
    else if(color_space_from == "YCbCr.709")
        from_YCbCr_709_to_RGB();
    else if(color_space_from == "YCoCg")
        from_YCoCg_to_RGB();
    else if(color_space_from == "CMY")
        from_CMY_to_RGB();
    else
     {
         cerr << "Wrong color space";
         return 1;
     }

    if(color_space_to == "RGB")
    {
    }
    else if(color_space_to == "HSL")
        from_RGB_to_HSL();
    else if(color_space_to == "HSV")
        from_RGB_to_HSV();
    else if(color_space_to == "YCbCr.601")
        from_RGB_to_YCbCr_601();
    else if(color_space_to == "YCbCr.709")
        from_RGB_to_YCbCr_709();
    else if(color_space_to == "YCoCg")
        from_RGB_to_YCoCg();
    else if(color_space_to == "CMY")
        from_RGB_to_CMY();
    else
    {
        cerr << "Wrong color space";
        return 1;
    }


    if (count_out == 3)
    {
        for(int i = 1; i < 4; i++)
        {
            string name_of_file = output_f;
            if(i == 1)
                name_of_file.insert( name_of_file.find("."), "_1");
            else if(i == 2)
                name_of_file.insert( name_of_file.find("."), "_2");
            else
                name_of_file.insert( name_of_file.find("."), "_3");
            char *name = &name_of_file[0];
            fout = fopen(name, "wb");
            if (fout == NULL)
            {
                cerr << "Input file can't be open";
                return 1;
            }
            fprintf(fout, "%c%d\n%d %d\n%d\n", symbol, 5, w, h, max_color);
            int quantity;
            if(i == 1)
                quantity = fwrite(&image1[0], sizeof(unsigned char), image1.size(), fout);
            else if(i == 2)
                quantity = fwrite(&image2[0], sizeof(unsigned char), image2.size(), fout);
            else
                quantity = fwrite(&image3[0], sizeof(unsigned char), image3.size(), fout);
            fclose(fout);
            if (quantity != h * w)
            {
                cerr << "Quantity of output file != width * height";
                return 1;
            }
        }
    }
    else
    {
        for(int i = 0; i < 3 * h * w; i += 3)
        {
            image[i] = image1[i / 3];
            image[i + 1] = image2[i / 3];
            image[i + 2] = image3[i / 3];
        }
        char *name = &output_f[0];
        fout = fopen(name, "wb");
        if (fout == NULL)
        {
            cerr << "Input file can't be open";
            return 1;
        }
        fprintf(fout, "%c%d\n%d %d\n%d\n", symbol, 6, w, h, max_color);
        int quantity = fwrite(&image[0], sizeof(unsigned char), image.size(), fout);
        fclose(fout);
        if (quantity != 3 * h * w)
        {
            cerr << "Quantity of output file != width * height";
            return 1;
        }
    }
    return 0;
}