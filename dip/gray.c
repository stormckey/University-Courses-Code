#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

typedef struct BMP{
    FILE* file;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
} BMP;
int write_gray_data(BMP bmp,double* graydata);
int adjust_gray_metadata(BMP*);
void fill_gray_palette(RGBQUAD*);
int write_rgbdata(BMP,unsigned char*);
int write_data(BMP bmp,unsigned char* black_white);//write the prepared bmp data into file
int initial_bmp(BMP);//write the file header into the file 
BMP create_bmp();//read the first bmp in D://test.bmp
int fill_rgb_yuv_array(BMP,unsigned char*,double*);//read bmp data from the 24color file and transfer it into gray number array

int main(){
    BMP rawbmp = create_bmp();
    unsigned char *rgbdata= malloc (sizeof(unsigned char)*3*rawbmp.bi.biWidth*rawbmp.bi.biHeight);// to store the gray number for gray bmp
    double *yuv= malloc (sizeof(double)*3*rawbmp.bi.biWidth*rawbmp.bi.biHeight);
    fill_rgb_yuv_array(rawbmp,rgbdata,yuv);//fill in the array
    FILE * grayfile =fopen("D://gray.bmp","wb");
    BMP graybmp = {grayfile,rawbmp.bf,rawbmp.bi};
    adjust_gray_metadata(&graybmp);
    initial_bmp(graybmp);
    RGBQUAD rgbquad[256];
    fill_gray_palette(rgbquad);
    fwrite(&rgbquad,sizeof(rgbquad),1,grayfile);
    write_gray_data(graybmp,yuv);
}
//func
int fill_rgb_yuv_array(BMP rawbmp,unsigned char* rgbdata, double * yuv){
    int width = rawbmp.bi.biWidth*3;
    int adjusted_width = (width+3)/4*4;
    unsigned char dustbin;
    int count=0;
    for(int i=0; i< rawbmp.bi.biWidth * rawbmp.bi.biHeight; i++){
        fread(&rgbdata[3*i],1,1,rawbmp.file);// read the  rgb number
        fread(&rgbdata[3*i+1],1,1,rawbmp.file);
        fread(&rgbdata[3*i+2],1,1,rawbmp.file);
        unsigned char y = 0.299*rgbdata[3*i+2] + 0.587*rgbdata[3*i+1] + 0.114*rgbdata[3*i];
        yuv[3*i] = 0.299*rgbdata[3*i+2] + 0.587*rgbdata[3*i+1] + 0.114*rgbdata[3*i] ;
        yuv[3*i+1] = -0.147*rgbdata[3*i+2] + -0.289*rgbdata[3*i+1] + 0.435 *rgbdata[3*i];
        yuv[3*i+2] = 0.615*rgbdata[3*i+2] + -0.515*rgbdata[3*i+1] + -0.1*rgbdata[3*i];
        count+=3;
        if (count == width){// read the 0s that just take up places 
            while(count++ != adjusted_width){
                fread(&dustbin,1,1,rawbmp.file);
            }
            count = 0;
        }
    }
}
BMP create_bmp(){
    BMP newbmp;
    FILE * bmp;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    if( !(bmp = fopen("D:\\test.bmp","rb"))){
        printf("Not find file in D:\\test.bmp");
        return newbmp;
    }
    fread(&bf,sizeof(BITMAPFILEHEADER),1,bmp);
    fread(&bi,sizeof(BITMAPINFOHEADER),1,bmp);
    newbmp.bf = bf;
    newbmp.bi = bi;
    newbmp.file = bmp;
    return newbmp;
}

int initial_bmp(BMP bmp){
    fwrite(&bmp.bf,sizeof(bmp.bf),1,bmp.file);
    fwrite(&bmp.bi,sizeof(bmp.bi),1,bmp.file);
}
int write_gray_data(BMP bmp,double* graydata){
    int width = bmp.bi.biWidth*3;
    int adjusted_width = (width+3)/4*4;
    int count=0;
    unsigned char y;
    for(int i=0;i<bmp.bi.biHeight*bmp.bi.biWidth;i++){
        y = (unsigned char)graydata[3*i];
        fwrite(&y,sizeof(unsigned char),1,bmp.file);
        count+=1;
        if (count == width){// read the 0s that just take up places 
            while(count++ != adjusted_width){
                fwrite(graydata,1,1,bmp.file);
            }
            count = 0;
        }
    }
}
int adjust_gray_metadata(BMP* Graybmp){
    Graybmp->bf.bfOffBits += 256 *sizeof(RGBQUAD);
    Graybmp->bf.bfSize = Graybmp->bf.bfOffBits + (Graybmp->bi.biWidth+3/4*4) *Graybmp->bi.biHeight;
    Graybmp->bi.biBitCount = 8;
    Graybmp->bi.biSizeImage =  (Graybmp->bi.biWidth+3/4*4) *Graybmp->bi.biHeight;
}
void fill_gray_palette(RGBQUAD* rgbquad){
    for(int i=0; i<256; i++){
        rgbquad[i].rgbBlue = i;
        rgbquad[i].rgbGreen = i;
        rgbquad[i].rgbRed = i;
        rgbquad[i].rgbReserved = 0 ;
    }
}