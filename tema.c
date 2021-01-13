#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "util/bmp_header.h"

//Pixel structure
typedef struct{
    unsigned char Red, Green, Blue;
} PIXEL;

//Write fileheader in output file
void print_header(bmp_fileheader *pHeader, FILE *outFile) {
    fwrite(pHeader,sizeof(bmp_fileheader),1,outFile);
}

//Write infoheader in output file
void print_infoheader(bmp_infoheader *pInfoHeader, FILE *outFile,
			signed int new_size1, signed int new_size2) {
    fwrite(&pInfoHeader->biSize, sizeof(pInfoHeader->biSize),1,outFile);
    fwrite(&new_size1, sizeof(new_size1),1,outFile);
    fwrite(&new_size2, sizeof(new_size2),1,outFile);
    fwrite(&pInfoHeader->planes, sizeof(pInfoHeader->planes),1,outFile);
    fwrite(&pInfoHeader->bitPix, sizeof(pInfoHeader->bitPix),1,outFile);
    fwrite(&pInfoHeader->biCompression, sizeof(pInfoHeader->biCompression),1,outFile);
    fwrite(&pInfoHeader->biSizeImage, sizeof(pInfoHeader->biSizeImage),1,outFile);
    fwrite(&pInfoHeader->biXPelsPerMeter, sizeof(pInfoHeader->biXPelsPerMeter),1,outFile);
    fwrite(&pInfoHeader->biYPelsPerMeter, sizeof(pInfoHeader->biYPelsPerMeter),1,outFile);
    fwrite(&pInfoHeader->biClrUsed, sizeof(pInfoHeader->biClrUsed),1,outFile);
    fwrite(&pInfoHeader->biClrImportant, sizeof(pInfoHeader->biClrImportant),1,outFile);
}

//Truncates value
int overflow(int nr) {
    if (nr > 255) return 255;
    else if (nr < 0) return 0;
    return nr;
}

//Write pixel in output file
void write_pixel(PIXEL *pPixel, int Red, int Green, int Blue, FILE *outFile) {
    fwrite(&Red, sizeof(pPixel->Red), 1, outFile);
    fwrite(&Green, sizeof(pPixel->Green), 1, outFile);
    fwrite(&Blue, sizeof(pPixel->Blue), 1, outFile);
}

//Black&White method
void black_white(bmp_infoheader *pInfoHeader, FILE *outFile,
		PIXEL **pImage, PIXEL *pPixel) {

    int i, j, sum;
    int padding_out = (4 - (pInfoHeader->width * sizeof(PIXEL)) % 4) % 4;
    for(i = 0; i < pInfoHeader->height; i++){
		for(j = 0; j < pInfoHeader->width; j++){
			sum = pImage[i][j].Red + pImage[i][j].Green + pImage[i][j].Blue;
			sum /= 3;
			write_pixel(pPixel, sum, sum, sum, outFile);
        }
        static const char Padding[4]={0,0,0,0};
        fwrite(Padding, padding_out, 1, outFile);
    }
}

//Nocrop method
void nocrop(bmp_infoheader *pInfoHeader, FILE *outFile,
		PIXEL **pImage, PIXEL *pPixel) {

    int i, j, sum, inf, sup, ok=0;
    //calculate new dimension
    if (pInfoHeader->height < pInfoHeader->width) sum = pInfoHeader->width - pInfoHeader->height;
    else if (pInfoHeader->height > pInfoHeader->width) sum = pInfoHeader->height - pInfoHeader->width, ok=1;
    else sum=0;

    int padding_out = (4 - (pInfoHeader->width * sizeof(PIXEL)) % 4) % 4;
    inf = sum / 2;
    sup = sum / 2;
    //if values differ
    if (sum % 2) sup++;
    //width is bigger
    if (!ok) {
		print_infoheader(pInfoHeader,outFile,pInfoHeader->width,pInfoHeader->width);
		//Complete at bottom
		for(i = 0; i < sup; i++) {
            for(j = 0; j < pInfoHeader->width; j++){
				sum=255;
                write_pixel(pPixel, sum, sum, sum, outFile);
    		}
            static const char Padding[4]={0,0,0,0};
        	fwrite(Padding, padding_out, 1, outFile);
		}
		//Initial image
    	for(i = 0; i < pInfoHeader->height; i++){
            for(j = 0; j < pInfoHeader->width; j++)
				write_pixel(pPixel, pImage[i][j].Red, pImage[i][j].Green, pImage[i][j].Blue, outFile);
            static const char Padding[4]={0,0,0,0};
        	fwrite(Padding, padding_out, 1, outFile);
		}
		//Complete top
    	for(i = 0; i < inf; i++) {
            for(j = 0; j < pInfoHeader->width; j++){
				sum=255;
    			write_pixel(pPixel, sum, sum, sum, outFile);
            }
            static const char Padding[4]={0,0,0,0};
        	fwrite(Padding, padding_out, 1, outFile);
		}
    } else {
		//height is bigger
		print_infoheader(pInfoHeader,outFile,pInfoHeader->height,pInfoHeader->height);
		for(i = 0; i < pInfoHeader->height; i++){
			//Complete left
			for(j=0;j<inf;j++) {
				sum=255;
				write_pixel(pPixel, sum, sum, sum, outFile);
			}
			for(j = 0; j < pInfoHeader->width; j++){
				write_pixel(pPixel, pImage[i][j].Red, pImage[i][j].Green, pImage[i][j].Blue, outFile);
        	}
			//Complete right
			for(j=0;j<sup;j++) {
				sum=255;
				write_pixel(pPixel, sum, sum, sum, outFile);
			}
			static const char Padding[4]={0,0,0,0};
        	fwrite(Padding, padding_out, 1, outFile);
    	}
    }
}

//Filter method
void filter_image(bmp_infoheader *pInfoHeader, FILE *outFile,
		PIXEL **pImage, PIXEL *pPixel, int **filter, int filterSize) {

	int padding_out = (4 - (pInfoHeader->width * sizeof(PIXEL)) % 4) % 4;
	int sumRed, sumGreen, sumBlue, i, j, k, h, pos1, pos2;
    for (i = 0; i < pInfoHeader->height; i++) {
        for (j = 0; j < pInfoHeader->width; j++) {
            sumRed = 0;
            sumGreen = 0;
            sumBlue = 0;
            //Build a matrix smalled than 'filterSize'
            for (k = i - (filterSize / 2), pos1 = 0; pos1 < filterSize; k++, pos1++) {
                for (h = j - (filterSize / 2), pos2 = 0; pos2 < filterSize; h++, pos2++) {
                    //If pixel is in matrix
                    if (k >= 0 && k < pInfoHeader->height && h >= 0 && h < pInfoHeader->width) {
                        sumRed += (filter[pos1][pos2] * pImage[k][h].Red);
                        sumGreen += (filter[pos1][pos2] * pImage[k][h].Green);
                        sumBlue += (filter[pos1][pos2] * pImage[k][h].Blue);
                    }
                }
            }
            //Check if <0 or >255
            sumRed = overflow(sumRed);
            sumGreen = overflow(sumGreen);
            sumBlue = overflow(sumBlue);
            write_pixel(pPixel, sumRed, sumGreen, sumBlue, outFile);
            }
        static const char Padding[4]={0,0,0,0};
        fwrite(Padding, padding_out, 1, outFile);
    }
}

//Methods to calculate min/max
int min(int nr1, int nr2) {
	if (nr1 > nr2) return nr2;
	return nr1;
}

int max(int nr1, int nr2) {
	if (nr1 > nr2) return nr1;
	return nr2;
}

int min_max(char type, int nr1, int nr2) {
	if (type == 'm') return min(nr1, nr2);
	return max(nr1, nr2);
}

//Pooling method
void pooling_image(bmp_infoheader *pInfoHeader, FILE *outFile,
		PIXEL **pImage, PIXEL *pPixel, char type, int poolingSize) {
	int padding_out = (4 - (pInfoHeader->width * sizeof(PIXEL)) % 4) % 4;
	int typeRed, typeGreen, typeBlue, i, j, k, h, pos1, pos2;
    for (i = 0; i < pInfoHeader->height; i++) {
        for (j = 0; j < pInfoHeader->width; j++) {
            typeRed = pImage[i][j].Red;
            typeGreen = pImage[i][j].Green;
            typeBlue = pImage[i][j].Blue;
            //Build smaller matrix
            for (k = i - (poolingSize / 2), pos1 = 0; pos1 < poolingSize; k++, pos1++) {
                for (h = j - (poolingSize / 2), pos2 = 0; pos2 < poolingSize; h++, pos2++) {
                    //If pixel is part of matrix
                    if (k >= 0 && k < pInfoHeader->height && h >= 0 && h < pInfoHeader->width) {
                        //Calculate min/max
                        typeRed = min_max(type, typeRed, pImage[k][h].Red);
                        typeGreen = min_max(type, typeGreen, pImage[k][h].Green);
                        typeBlue = min_max(type, typeBlue, pImage[k][h].Blue);
                    } else {
                        typeRed = min_max(type, typeRed, 0);
                        typeGreen = min_max(type, typeGreen, 0);
                        typeBlue = min_max(type, typeBlue, 0);
                    }
                }
            }
            //Scriu pixelul in output file
            write_pixel(pPixel, typeRed, typeGreen, typeBlue, outFile);
            }
        //Scriu padding-ul in output file
        static const char Padding[4]={0,0,0,0};
        fwrite(Padding, padding_out, 1, outFile);
    }
}

//Read filter matrix
int **readMatrix(int **matrix, int matrixSize, FILE *input) {
	int i, j;
	matrix = malloc(matrixSize * matrixSize);
    for (i = 0; i < matrixSize; i++) {
        matrix[i] = calloc(matrixSize, sizeof(int));
    }
	for (i = 0; i < matrixSize; i++)
		for (j = 0; j < matrixSize; j++)
			fscanf(input,"%d",&matrix[i][j]);
    return matrix;
}

//Check file open
int fileCheck(FILE *file) {
	if(file == NULL) {
		printf("Can't open file.\n");
        return 1;
    }
	return 0;
}

int main() {
    FILE *inFile, *inFilter, *inPooling, *inCluster;
	FILE *outFile_black, *outFile_crop, *outFile_cluster, *outFile_filter, *outFile_pooling;

    bmp_fileheader *pHeader;
    bmp_infoheader *pInfoHeader;
    PIXEL *pPixel, **pImage;

    int i, j, k;
    char fName[200], fFilter[200], fPooling[200], fCluster[200];
	char outName1[200], outName2[200], outName3[200], outName4[200], outName5[200];

    for (k = 0; k <= 9; k++){

	//Create files
    sprintf(fName,"input/images/test%d.bmp",k);
	sprintf(fFilter,"input/filters/filter%d.txt",k);
	sprintf(fPooling,"input/pooling/pooling%d.txt",k);
	sprintf(fCluster,"input/clustering/cluster%d.txt",k);
    sprintf(outName1,"test%d_black_white.bmp",k);
    sprintf(outName2,"test%d_nocrop.bmp",k);
	sprintf(outName3,"test%d_filter.bmp",k);
	sprintf(outName4,"test%d_pooling.bmp",k);
	sprintf(outName5,"test%d_clustered.bmp",k);

    pHeader = (bmp_fileheader *)malloc(sizeof(bmp_fileheader));
    pInfoHeader = (bmp_infoheader *)malloc(sizeof(bmp_infoheader));
    pPixel = (PIXEL *)malloc(sizeof(PIXEL));

	//Open input files
    inFile = fopen(fName, "rb");
	inFilter = fopen(fFilter, "rt");
	inPooling = fopen(fPooling, "rt");
	inCluster = fopen(fCluster, "rt");

    //Check open
    if (fileCheck(inFile)) return 1;
    if (fileCheck(inFilter)) return 1;
    if (fileCheck(inPooling)) return 1;
    if (fileCheck(inCluster)) return 1;

	//Read filter input
	int filterSize;
	int **filter;
	fscanf(inFilter,"%d",&filterSize);
	filter = readMatrix(filter,filterSize, inFilter);

	//Read pooling input
	char type;
	int poolingSize;
	fread(&type,sizeof(char),1,inPooling);
	fscanf(inPooling,"%d",&poolingSize);
	int threshold;
	fscanf(inCluster,"%d",&threshold);

    //Open output files
    outFile_black = fopen(outName1, "wb");
    outFile_crop = fopen(outName2, "wb");
    outFile_filter = fopen(outName3, "wb");
    outFile_pooling = fopen(outName4, "wb");
    outFile_cluster = fopen(outName5, "wb");

    //Check open
    if (fileCheck(outFile_black)) return 1;
    if (fileCheck(outFile_crop)) return 1;
    if (fileCheck(outFile_filter)) return 1;
    if (fileCheck(outFile_pooling)) return 1;
    if (fileCheck(outFile_cluster)) return 1;

	//Read and write fileheader
    fread(pHeader,sizeof(bmp_fileheader),1,inFile);
    print_header(pHeader,outFile_black);
    print_header(pHeader,outFile_crop);
    print_header(pHeader,outFile_filter);
	print_header(pHeader,outFile_pooling);
	print_header(pHeader,outFile_cluster);

	//Read and write infoheader
    fread(pInfoHeader,sizeof(bmp_infoheader),1,inFile);
    print_infoheader(pInfoHeader,outFile_black,pInfoHeader->width,pInfoHeader->height);
	print_infoheader(pInfoHeader,outFile_filter,pInfoHeader->width,pInfoHeader->height);
	print_infoheader(pInfoHeader,outFile_pooling,pInfoHeader->width,pInfoHeader->height);
	print_infoheader(pInfoHeader,outFile_cluster,pInfoHeader->width,pInfoHeader->height);

    //Dynamic allocation
    pImage = (PIXEL **)malloc(pInfoHeader->width * pInfoHeader->height); //sizeof(PIXEL *)
    for(i = 0; i < pInfoHeader->height; i++){
		pImage[i] = (PIXEL *)malloc(sizeof(PIXEL) * pInfoHeader->width);
    }

	//Calculate padding
	int padding_in  = (4 - (pInfoHeader->width  * sizeof(PIXEL)) % 4) % 4;

	//Read image matrix
    for(i = 0; i < pInfoHeader->height; i++){
		for(j = 0; j < pInfoHeader->width; j++){
			fread(&pImage[i][j].Red, sizeof(pPixel->Red), 1, inFile);
			fread(&pImage[i][j].Green, sizeof(pPixel->Green), 1, inFile);
			fread(&pImage[i][j].Blue, sizeof(pPixel->Blue), 1, inFile);
        }
		fseek(inFile, padding_in, SEEK_CUR);
    }


    black_white(pInfoHeader, outFile_black, pImage, pPixel);

    nocrop(pInfoHeader, outFile_crop, pImage, pPixel);

	filter_image(pInfoHeader, outFile_filter, pImage, pPixel, filter, filterSize);

	pooling_image(pInfoHeader, outFile_pooling, pImage, pPixel, type, poolingSize);

    fclose(inFile);
	fclose(inFilter);
    fclose(outFile_black);
	fclose(outFile_filter);
	fclose(outFile_crop);
    free(pHeader);
    free(pInfoHeader);
    free(pPixel);
	for(i = 0; i < pInfoHeader->height; i++){
		free(pImage[i]);
    }
    free(pImage);
	for(i = 0; i < filterSize; i++){
		free(filter[i]);
    }
    free(filter);
	}
	
    return 0;
}
