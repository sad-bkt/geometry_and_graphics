#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

//using namespace std;
//bool flag_error = false;
int transformation;
char symbol;
int number, h, w, max_color;
std::vector<unsigned char> image1, image2;

void inversion()
{
	for (int i = 0; i < image2.size(); ++i)
		image2[i] = max_color - (int) image1[i];
}

void reflection_gorisontal()
{
	if (number == 5)
	{
		for (int i = 0; i < w; ++i)
			for (int j = 0; j < h; ++j)
				image2[i * h + j] = image1[i * h + h - j - 1];
	}
	else
	{
		for (int i = 0; i < w; ++i)
			for (int j = 0; j < 3 * h; j += 3)
			{
				image2[i * 3 * h + j] = image1[i * 3 * h + 3 * h - j - 3];
				image2[i * 3 * h + j + 1] = image1[i * 3 * h + 3 * h - j - 2];
				image2[i * 3 * h + j + 2] = image1[i * 3 * h + 3 * h - j - 1];
			}		
	}
}

void reflection_vertical()
{
	if (number == 5)
	{
		for (int i = 0; i < w; ++i)
			for (int j = 0; j < h; ++j)
				image2[i * h + j] = image1[(w - i - 1) * h + j];
	}
	else
	{
		for (int i = 0; i < w; ++i)
			for (int j = 0; j < 3 * h; ++j)
				image2[i * 3 * h + j] = image1[(w - i - 1) * 3 * h + j];
	}
}

void clockwise_rotation()
{
	std::swap(w, h);
	if (number == 5)
	{
		int k = 0;
		for (int i = 0; i < w; ++i)
			for (int j = 0; j < h; ++j)
			{
				image2[k] = image1[(h - 1 - j) * w + i];
				k++;
			}		
	}
	else
	{
		int k = 0;
		for (int i = 0; i < 3 * w; i += 3)
			for (int j = 0; j < h; ++j)
			{
				image2[k] = image1[(h - 1 - j) * 3 * w + i];
				image2[k + 1] = image1[(h - 1 - j) * 3 * w + i + 1];
				image2[k + 2] = image1[(h - 1 - j) * 3 * w + i + 2];
				k += 3;
			}
	}
}

void counterclockwise_rotation()
{
	std::swap(w, h);
	if (number == 5)
	{
		int k = 0;
		for (int i = 0; i < w; ++i)
			for (int j = 0; j < h; ++j)
			{
				image2[k] = image1[j * w + w - 1 - i];
				k++;
			}
	}
	else
	{
		int k = 0;
		for (int i = 0; i < 3 * w; i += 3)
			for (int j = 0; j < h; ++j)
			{
				image2[k] = image1[j * 3 * w + 3 * w - 3 - i];
				image2[k + 1] = image1[j * 3 * w + 3 * w - 2 - i];
				image2[k + 2] = image1[j * 3 * w + 3 * w - 1 - i];
				k += 3;
			}
	}
}

void err(int i)
{
	printf("Error\n");
	if (i == 1)
	{
		printf("You should use this structure:\n");
		printf("<input_file_name> <output_file_name> <transformation>\n");
		printf("where <transformation> is integer in [0, 4]");
	}
	else if (i == 2)
		printf("Incorrect pnm file");
	else if (i == 3)
		printf("Input file can't be open");
	else if (i == 4)
		printf("Output file can't be open");
	system("pause");
	exit(0);
}

int main(int argc, char *argv[])
{
	printf("PROCESSING\n");
	if (argc == 4)
	{
		/*sscanf(argv[1], "%s", fin_name);
		sscanf(argv[2], "%s", fout_name);
		sscanf(argv[3], "%d", transformation);
		fin_name = argv[1];
		fout_name = argv[2];*/
		transformation = atoi(argv[3]);
		if (transformation >= 5 && transformation <= 0)
			err(1);
	}
	else
		err(1);

	FILE *fin, *fout;
	fin = fopen(argv[1], "rb");
	if (fin == NULL)
		err(3);

	int quantity = fscanf(fin, "%c%d\n%d%d\n%d\n", &symbol, &number, &h, &w, &max_color);	//ñ÷èòûâàíèå è ïðîâåðêà íà êîððåêòíîñòü ïî ðàçìåðó
	if (quantity != 5 || symbol != 'P' || (number != 5 && number != 6) || h <= 0 || w <= 0 || max_color <= 0 || max_color > 255)
		err(2);
	if (number == 5)
	{
		image1.resize(h * w, 0);
		image2.resize(h * w, 0);
		quantity = fread(&image1[0], sizeof(unsigned char), image1.size(), fin);
		if (quantity != h * w)
			err(2);
	}
	else
	{
		image1.resize(3 * h * w, 0);
		image2.resize(3 * h * w, 0);
		quantity = fread(&image1[0], sizeof(unsigned char), image1.size(), fin);
		if (quantity != 3 * h * w)
			err(2);
	}
		
	switch (transformation)
	{
		case 0:
			inversion();
			break;
		case 1:
			reflection_gorisontal();
			break;
		case 2:
			reflection_vertical();
			break;
		case 3:
			clockwise_rotation();
			break;
		case 4:
			counterclockwise_rotation();
			break;
		default:
			err(1);
	}

	//if (!flag_error)
	//{
		fout = fopen(argv[2], "wb");
		if (fout == NULL)
			err(4);
		printf("DONE");
		fprintf(fout, "%c%d\n%d %d\n%d\n", symbol, number, h, w, max_color);
		fwrite(&image2[0], sizeof(unsigned char), image2.size(), fout);
	//}
	fclose(fin);
	fclose(fout);
	system("pause");
	return 0;
}
