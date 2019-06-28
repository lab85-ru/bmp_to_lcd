// Converting BMP grayscale(8 bit) -> array C code for SSD1309 (128x64)

#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "git_commit.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>


const char PRINT_TIRE[] = {"================================================================================\n\r"};
const char PRINT_PROG_NAME[] = {" Converting BMP grayscale(8 bit) -> array C code for SSD1309 (128x64)\n\r"};
const char PRINT_VERSION[] = {" Ver 0.1 2019.\n\r"};

const uint32_t BUF_SIZE = 1*1024*1024; // размер пам€ти дл€ файла BMP.
#define FILE_RW_BLOCK_SIZE (1024)

void print_rule(size_t len, uint8_t density);

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char * argv[])
{
	struct stat stat_buf;
	FILE * fi = 0;
	uint8_t *mem = NULL;       // ukazatel na buf-mem dla BMP FILE

	char *file_name_in = NULL;
	size_t pos    = 0;
    size_t r_size = 0;
    size_t result = 0;

	size_t x,yo,yi;
	uint8_t po, pixo, p;

	int c; // счетчик количества HEX елементов с строке

	BITMAPFILEHEADER *bmp_head_p;      // ”казатель на заголовок BMP
	BITMAPINFOHEADER *bmp_info_head_p; // ”казатель на заголовок информации о данных и формате картинки
	uint8_t *v;                        // указатель на начало видео-данных

	uint16_t pic_color_bit;// количество цветов
	uint32_t pic_w;        // ширина каринки
	uint32_t pic_wl;       // число пикселей которых нехватает до длинны строки кратной 4
	uint32_t pic_h;        // высота картинки


	printf( PRINT_TIRE );
    printf( PRINT_PROG_NAME );
	printf( PRINT_VERSION );
    printf( " GIT = %s\n\r", git_commit_str );
	printf( PRINT_TIRE );

	if (argc == 1){
		printf("RUN 1:  Output to console\n");
		printf("  bmp_to_lcd.exe  (file input BMP)\n\n");
		printf("RUN 2:  Output to FILE\n");
		printf("  bmp_to_lcd.exe  (file input BMP) > (FILE OUTPUT)\n\n");
		return 0;
	}

	file_name_in = argv[1];

	{
		if (stat(file_name_in, &stat_buf) == -1){
			printf("ERROR: fstat return error !\n\r");
			return 1;
		}

		// proverka diapazona offset+size <= density
		if (stat_buf.st_size > BUF_SIZE ){
			printf("ERROR: file is BIG !!! file_size( %lld ) > chip_size(+offset %lld )\n\r", (long long)stat_buf.st_size, (long long)(BUF_SIZE));
			return 1;
		}

		mem = (uint8_t *)malloc(BUF_SIZE);
		if (mem == NULL){
			printf("ERROR: mem = malloc(BUF_SIZE).\n");
			return 1;
		}

		printf("File size = %lld bytes.\n\r", (long long)stat_buf.st_size);

		if (stat_buf.st_size == 0){
			printf("ERROR: file is empty.\n\r");
			return 1;
		}

		//printf("Read data from file.\n\r");
        // read data from file
		fi = fopen(file_name_in, "rb");
		if ( fi == NULL ){
			printf("ERROR: open file.\n\r");
			return 1;
		}

		pos = 0;
		while(pos != stat_buf.st_size){
            if (stat_buf.st_size - pos > FILE_RW_BLOCK_SIZE)
				r_size = FILE_RW_BLOCK_SIZE;
			else
                r_size = stat_buf.st_size - pos;

            result = fread( &mem[pos], 1, r_size, fi);
			if (result != r_size){
			    printf("ERROR: read data from file. result( %d ) != fread( r_size( %u ))\n\r", result, r_size);
				return 1;
			}

            pos = pos + r_size;
		}
        fclose( fi );
	}
//-----------------------------------------------------------------------------
    bmp_head_p = (BITMAPFILEHEADER *)mem;

	if (bmp_head_p->bfType != ('B' | ('M' << 8))){
		printf("ERROR: This file not BMP !\n");
		return 1;
	}

	if (bmp_head_p->bfSize != stat_buf.st_size){
		printf("ERROR: Header file LENGTH !\n");
		return 1;
	}

	bmp_info_head_p   = (BITMAPINFOHEADER*)(mem + sizeof(BITMAPFILEHEADER));
	pic_color_bit     = bmp_info_head_p->biBitCount;
	pic_w             = bmp_info_head_p->biWidth;
	pic_h             = bmp_info_head_p->biHeight;

	printf("=================================================\n");
	printf("BMP Picture INFO:\n");
	printf(" Colors     : %d bits\n", pic_color_bit);
	printf(" Resolution : %d x %d\n", pic_w, pic_h);
	printf("=================================================\n");

	if (pic_color_bit != 8){
		printf("ERROR: This BMP file color bit %d != 8\n", pic_color_bit);
		return 1;
	}

/* обрабатываем картинку с любым разрешением
	if (pic_w != 128){
		printf("ERROR: Picture resolution width %d != 128\n", pic_w);
		return 1;
	}
	if (pic_h != 64){
		printf("ERROR: Picture resolution height %d != 64\n", pic_w);
		return 1;
	}
*/
	v = mem + bmp_head_p->bfOffBits;

//  артинка располагаетс€ в пам€ти в следующем пор€дке
// начало пам€ти соотвествует - левый нижний угол картинки
//
// RAM OFFSET = 0
// +-------------------------------------------+
// + 0.0                                       +
// +                                           +
// +                                           +
// +                                           +
// +                                           +
// +                                       X,Y +
// +-------------------------------------------+


	// длинна строки данных (в массиве) кратна 4 !
    pic_wl = (3 * pic_w) % 4; // разница сколько нехватае пикселей до кратности 4

	printf("\nData from BMP picture:\n");
    print_rule(pic_w + pic_wl, 0); // выводим линейку на экран

	// выводим картинку с разворотом (сначала выбираем нижние элементы) 
	for (yi=0; yi<pic_h; yi++){
		for (x=0; x<pic_w; x++){
			p = *(v + x + (((pic_h - 1) - yi) * (pic_w + pic_wl)));
			if (p) pixo = '*'; else pixo = ' ';
			printf("%c", pixo);
		}
		printf("|\n");
    }
	printf("\n");
    print_rule(pic_w + pic_wl, 0); // выводим линейку на экран

//-----------------------------------------------------------------------------
	printf("\nData TO LCD array:\n");
    print_rule(pic_w + pic_wl, 1); // выводим линейку на экран

	// выводим данные на экран дл€ проверки
	for (yo=0; yo<pic_h / 8; yo++){
		for (x=0; x<pic_w; x++){

			po = 0;

			for (yi=0; yi<pic_h / 8; yi++){
				
				p = *(v + x + (( (pic_h - 1) - yi - (yo * 8)) * (pic_w + pic_wl)));

				if (p) pixo = 0x80; else pixo = 0;
				po = po >> 1;
				po |= pixo;
			}
			printf("%02X", po);
		}
		printf("\n");
	}
//-----------------------------------------------------------------------------
	printf("\nData TO C code array:\n");
	printf("\n    ");
	c = 0;
	for (yo=0; yo<pic_h/8; yo++){
		for (x=0; x<pic_w; x++){

			po = 0;

			for (yi=0; yi<pic_h/8; yi++){
				
				p = *(v + x + (( (pic_h - 1) - yi - (yo * 8)) * (pic_w + pic_wl)));

				if (p) pixo = 0x80; else pixo = 0;
				po = po >> 1;
				po |= pixo;
			}

			printf("0x%02X,", po);
			c++;
			if (c == 16){
				c = 0;
				printf("\n    ");
			}
		}
	}
	printf("\n");

	return 0;
}

//-----------------------------------------------------------------------------
// вывод линейки
// density - плотность =1 с пробелами или =0 без
//-----------------------------------------------------------------------------
void print_rule(size_t len, uint8_t density)
{
	size_t x;

    printf("\n");

    for (x=0; x<len / 10 + 1; x++){
		if (density) 
			printf("0 1 2 3 4 5 6 7 8 9 ");
		else
			printf("0123456789");
	}

	printf("\n");
}
