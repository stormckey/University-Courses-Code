#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
#define pi 3.1415926

typedef struct BMP{
    FILE* file;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
} BMP;
int write_gray_data(BMP bmp,double* graydata);//write the gray intensity array into the file
int adjust_gray_metadata(BMP*); // change the metadata of the graybmp
double* find(double*,int,int,int);//find the address of the wanted member of a 2d array
void fill_gray_palette(RGBQUAD*);//fill the gray palette
double Gauss(double sigma,double mu,double x);//the Gauss function with the parameter of mu and sigma
void bilateral(double*,double,double,BMP,int);//do the bilateral filter 
int write_data(BMP bmp,unsigned char* black_white);//write the prepared bmp data into file
int initial_bmp(BMP);//write the file header into the file 
BMP create_bmp();//read the first bmp in D://test.bmp
int fill_rgb_yuv_array(BMP,unsigned char*,double*);//read bmp data from the 24color file and transfer it into gray number array

int main(int argc,char** argv){
    BMP rawbmp = create_bmp();//read the test.bmp
    unsigned char *rgbdata= malloc (sizeof(unsigned char)*3*rawbmp.bi.biWidth*rawbmp.bi.biHeight);// to store the gray number for gray bmp
    double *yuv= malloc (sizeof(double)*3*rawbmp.bi.biWidth*rawbmp.bi.biHeight);
    fill_rgb_yuv_array(rawbmp,rgbdata,yuv);//fill in the array
    FILE * grayfile =fopen("D://gray.bmp","wb");//the new file of gray bmp
    BMP graybmp = {grayfile,rawbmp.bf,rawbmp.bi};
    adjust_gray_metadata(&graybmp);
    initial_bmp(graybmp);
    RGBQUAD rgbquad[256];
    fill_gray_palette(rgbquad);
    fwrite(&rgbquad,sizeof(rgbquad),1,grayfile);// write in the palette
    write_gray_data(graybmp,yuv);//write the y component into the file
    fclose(grayfile);
    FILE* bilfile = fopen("D://bilateral.bmp","wb");
    BMP bilbmp = {bilfile,graybmp.bf,graybmp.bi};
    initial_bmp(bilbmp);
    fwrite(&rgbquad,sizeof(rgbquad),1,bilfile);//write in the palette
    double s = 0.0025*sqrt((pow(bilbmp.bi.biHeight,2)+pow(bilbmp.bi.biWidth,2)));//the s parameter
    double r = 20;//the r parameter
    int window=13;
    if(argc>1)     s = strtod(argv[1],0);//if s is given by the command line parameter
    if(argc>2)     r = strtod(argv[2],0);//if r is given by the command line parameter
    if(argc>3)      window = (int)strtod(argv[3],0);//if window is given by the command line parameter 
    bilateral(yuv,s,r,bilbmp,window);
    write_gray_data(bilbmp,yuv+1);//remember that the processed data is stored in u component
    free(yuv);
    free(rgbdata);
}
//func
int fill_rgb_yuv_array(BMP rawbmp,unsigned char* rgbdata, double * yuv){
    int width = rawbmp.bi.biWidth*3;
    int adjusted_width = (width+3)/4*4;//if we need to write 0 to take up places
    unsigned char dustbin;
    int count=0;
    for(int i=0; i< rawbmp.bi.biWidth * rawbmp.bi.biHeight; i++){
        fread(&rgbdata[3*i],1,1,rawbmp.file);// read the  rgb number
        fread(&rgbdata[3*i+1],1,1,rawbmp.file);
        fread(&rgbdata[3*i+2],1,1,rawbmp.file);
        unsigned char y = 0.299*rgbdata[3*i+2] + 0.587*rgbdata[3*i+1] + 0.114*rgbdata[3*i];//calculate yuv
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
BMP create_bmp(){//read and initial from test.bmp
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

int initial_bmp(BMP bmp){//initial the certain bmp
    fwrite(&bmp.bf,sizeof(bmp.bf),1,bmp.file);
    fwrite(&bmp.bi,sizeof(bmp.bi),1,bmp.file);
}
int write_gray_data(BMP bmp,double* graydata){
    int width = bmp.bi.biWidth;
    int adjusted_width = (width+3)/4*4;//if we need to write 0 to take up places
    int count=0;
    unsigned char y;
    for(int i=0;i<bmp.bi.biHeight*bmp.bi.biWidth;i++){
        y = (unsigned char)graydata[3*i];
        fwrite(&y,sizeof(unsigned char),1,bmp.file);
        count+=1;
        if (count == width){// write the 0s that just take up places 
            while(count++ != adjusted_width){
                fwrite(graydata,1,1,bmp.file);
            }
            count = 0;
        }
    }
}
int adjust_gray_metadata(BMP* Graybmp){// adjust the metadata to fit with the new gray bmp
    Graybmp->bf.bfOffBits += 256 *sizeof(RGBQUAD);
    Graybmp->bf.bfSize = Graybmp->bf.bfOffBits + (Graybmp->bi.biWidth+3/4*4) *Graybmp->bi.biHeight;
    Graybmp->bi.biBitCount = 8;
    Graybmp->bi.biSizeImage =  (Graybmp->bi.biWidth+3/4*4) *Graybmp->bi.biHeight;
}
void fill_gray_palette(RGBQUAD* rgbquad){//fill in the palette array
    for(int i=0; i<256; i++){
        rgbquad[i].rgbBlue = i;
        rgbquad[i].rgbGreen = i;
        rgbquad[i].rgbRed = i;
        rgbquad[i].rgbReserved = 0 ;
    }
}
void bilateral(double* yuv,double s,double r,BMP bilbmp,int window){
    double Gauss_s[window][window];
    double *dest = yuv+1;
    double wid = window;
    double half = (wid-1)/2;
    for(int i=0;i<window;i++){//the space factor in the convolution kernal
        for(int j=0;j<window;j++){
            Gauss_s[i][j] = Gauss(s,0,sqrt(pow(i-half,2)+pow(j-half,2)));
        }
    }
    for(int i=0;i<(window-1)/2;i++){// the edge where we cant do the convolution, we just copy the gray intensity
        for(int j=0;j<bilbmp.bi.biWidth;j++){
            *(find(dest,bilbmp.bi.biWidth,j,i)) = *(find(yuv,bilbmp.bi.biWidth,j,i));
            *(find(dest,bilbmp.bi.biWidth,j,bilbmp.bi.biHeight-1-i)) = *(find(yuv,bilbmp.bi.biWidth,j,bilbmp.bi.biHeight-1-i));
        }
    }
    for(int i=0;i<half;i++){//the edge where we cant do the convolution,we just copy the gray intensity
        for(int j=0;j<bilbmp.bi.biHeight;j++){
            *(find(dest,bilbmp.bi.biWidth,i,j)) = *(find(yuv,bilbmp.bi.biWidth,i,j));
            *(find(dest,bilbmp.bi.biWidth,bilbmp.bi.biWidth-1-i,j)) = *(find(yuv,bilbmp.bi.biWidth,bilbmp.bi.biWidth-1-i,j));
        }
    }
    for(int i= half;i<bilbmp.bi.biWidth-half;i++){//for every pixel to be processed
        for(int j =half;j<bilbmp.bi.biHeight-half;j++){
            int modify_xy[window];//the offset of x and y
            double final_Gauss[window][window];// the modified kernal take both intensity and distance into account
            double total = 0;//do the normallization
            for(int k=0;k<window;k++)   modify_xy[k] = k - (window-1)/2;//fill in the offset such as -2 -1 0 1 2
            for(int i1=0;i1<window;i1++){
                for(int j1 =0;j1<window;j1++){//for every neighbour
                    int modified_x = i+modify_xy[i1];
                    int modified_y = j+modify_xy[j1];//the coordinate of the neighbour
                    final_Gauss[i1][j1] = Gauss_s[i1][j1]*Gauss(r,0,*(find(yuv,bilbmp.bi.biWidth,modified_x,modified_y))-*(find(yuv,bilbmp.bi.biWidth,i,j)));//the final weight of this neighbour
                    total += final_Gauss[i1][j1];
                }
            }
            double new_intensity=0;
            for(int i1=0;i1<window;i1++){
                for(int j1=0;j1<window;j1++){
                    int modified_x = i+modify_xy[i1];
                    int modified_y = j+modify_xy[j1];
                    new_intensity += *(find(yuv,bilbmp.bi.biWidth,modified_x,modified_y))*final_Gauss[i1][j1];//do the convolution
                }
            }
            new_intensity /= total;//normalization
            *(find(dest,bilbmp.bi.biWidth,i,j)) = new_intensity;//assignment
        }
    }
}
double Gauss(double sigma,double mu,double x){//calculate the Gauss function with given parameter
    return  1/sqrt(2*pi)/sigma*exp(-(pow(x-mu,2))/2/pow(sigma,2));
}
double* find(double* array,int width,int x,int y){//find the address of a member in a 2d array
    return array+y*3*width+3*x;
}