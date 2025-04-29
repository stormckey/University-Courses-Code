#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

typedef struct BMP{
    FILE* file;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
} BMP;
int write_gray_data(BMP bmp,double* graydata);//write the gray data into graybmp, because we use the palette, we only need to write the y component 
int write_rgbdata(BMP,unsigned char*);
int write_data(BMP bmp,unsigned char* black_white);//write the prepared bmp data into file
int initial_bmp(BMP);//write the file header into the file 
BMP create_bmp();//read the first bmp in D://test.bmp
int fill_rgb_yuv_array(BMP,unsigned char*,double*);//read bmp data from the 24color file and transfer it into gray number array
int mean(double *yuv,BMP);//calculate the pixels after mean operation ,which will be stored in where former u component is stored 
int lap(double *yuv,BMP);//calculate the pixels after laplace opeartion ,which will be stored in where former u component id stored

int main(){
    BMP rawbmp = create_bmp();
    unsigned char *rgbdata= malloc (sizeof(unsigned char)*3*rawbmp.bi.biWidth*rawbmp.bi.biHeight);// to store the gray number for gray bmp
    double *yuv= malloc (sizeof(double)*3*rawbmp.bi.biWidth*rawbmp.bi.biHeight);
    fill_rgb_yuv_array(rawbmp,rgbdata,yuv);//fill in the array
    RGBQUAD rgbquad[256];//create the palette
    for(int i=0; i<256; i++){
        rgbquad[i].rgbBlue = i;
        rgbquad[i].rgbGreen = i;
        rgbquad[i].rgbRed = i;
        rgbquad[i].rgbReserved = 0 ;
    }
    FILE * grayfile =fopen("D://gray.bmp","wb");//create the gray bmpso that  we can compare the origin picture with the processed picture
    BMP graybmp = {grayfile,rawbmp.bf,rawbmp.bi};
    graybmp.bf.bfOffBits += 256 *sizeof(RGBQUAD);
    graybmp.bf.bfSize = graybmp.bf.bfOffBits + (graybmp.bi.biWidth+3/4*4) *graybmp.bi.biHeight;
    graybmp.bi.biBitCount = 8;
    graybmp.bi.biSizeImage =  (graybmp.bi.biWidth+3/4*4) *graybmp.bi.biHeight;
    initial_bmp(graybmp);
    fwrite(&rgbquad,sizeof(rgbquad),1,grayfile);
    write_gray_data(graybmp,yuv);
    FILE * meanfile = fopen("D://mean.bmp","wb");//create the mean bmp and do necessary initialization
    BMP meanbmp = {meanfile,graybmp.bf,graybmp.bi};
    initial_bmp(meanbmp);
    fwrite(&rgbquad,sizeof(rgbquad),1,meanfile);
    mean(yuv,meanbmp);//act the convolution and store the processed data in u component
    write_gray_data(meanbmp,yuv+1);
    FILE * lapfile = fopen("D://lap.bmp","wb");//create the modified bmp and do necessary initialization
    BMP lapbmp = {lapfile,graybmp.bf,graybmp.bi};
    initial_bmp(lapbmp);
    fwrite(&rgbquad,sizeof(rgbquad),1,lapfile);
    lap(yuv,lapbmp);//act the convolution and store the processed data in u component
    write_gray_data(lapbmp,yuv+1);
}
int lap(double*yuv,BMP lapbmp){
    int off[3] = {-1,0,1};// the offset in coordinate
    int mask[3][3] = {{0,-1,0},{-1,4,-1},{0,-1,0}};//the covolution kernel 
    double *off_yuv = yuv +1;//where we store the result
    for(int i=0;i<lapbmp.bi.biWidth;i++){//store the most top nd bottom edge ,where wo wont do any modification
        off_yuv[3*i] = yuv[3*i];
        off_yuv[3*i + (lapbmp.bi.biHeight-1)*lapbmp.bi.biWidth*3] = yuv[3*i + (lapbmp.bi.biHeight-1)*lapbmp.bi.biWidth*3];
    }
    for(int i=0;i<lapbmp.bi.biHeight;i++){//store the most left and right edge ,where we wont do any modification
        off_yuv[i*lapbmp.bi.biWidth*3] = yuv[i*lapbmp.bi.biWidth*3];
        off_yuv[(i+1)*lapbmp.bi.biWidth*3-3] = yuv[ (i+1)* lapbmp.bi.biWidth*3-3];
    }
    for(int i=1;i<lapbmp.bi.biWidth-1;i++){//traverse the matrix
        for(int j=1;j<lapbmp.bi.biHeight-1;j++){
            double sum = 0;
            for(int n1=0;n1<3;n1++){//traverse the nearby pixels
                for(int n2=0;n2<3;n2++){
                    sum += yuv[3*lapbmp.bi.biWidth*(j+off[n1])+ 3*(i+off[n2])]*mask[n1][n2];//covolution
                }
            }
            off_yuv[3*i+j*lapbmp.bi.biWidth*3] = sum;
            if (off_yuv[3*i+j*lapbmp.bi.biWidth*3] < 0) off_yuv[3*i+j*lapbmp.bi.biWidth*3] = 0;//adjust if it is less than 0
            off_yuv[3*i+j*lapbmp.bi.biWidth*3] += yuv[3*lapbmp.bi.biWidth*j + 3*i];
            off_yuv[3*i+j*lapbmp.bi.biWidth*3] = off_yuv[3*i+j*lapbmp.bi.biWidth*3]>255 ? 255 : off_yuv[3*i+j*lapbmp.bi.biWidth*3];//adjust if it is bigger than 255
        }
    }
}
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
        yuv[3*i] = 0.299*rgbdata[3*i+2] + 0.587*rgbdata[3*i+1] + 0.114*rgbdata[3*i] ;//claculate the yuv number
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

int initial_bmp(BMP bmp){//write file headers 
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
        if (count == width){// write the 0s that just take up places 
            while(count++ != adjusted_width){
                fwrite(graydata,1,1,bmp.file);
            }
            count = 0;
        }
    }
}
int mean(double * yuv,BMP meanbmp){
    int off[5] = {-2,-1,0,1,2};//the offset of coordinates
    double *off_yuv = yuv +1;//where we store our result
    for(int i=0;i<meanbmp.bi.biWidth;i++){//store the top two and bottom two edges 
        off_yuv[3*i] = yuv[3*i];
        off_yuv[3*i+meanbmp.bi.biWidth*3] = yuv[3*i+meanbmp.bi.biWidth*3];
        off_yuv[3*i + (meanbmp.bi.biHeight-2)*meanbmp.bi.biWidth*3] = yuv[3*i + (meanbmp.bi.biHeight-2)*meanbmp.bi.biWidth*3];
        off_yuv[3*i + (meanbmp.bi.biHeight-1)*meanbmp.bi.biWidth*3] = yuv[3*i + (meanbmp.bi.biHeight-1)*meanbmp.bi.biWidth*3];
    }
    for(int i=0;i<meanbmp.bi.biHeight;i++){//store the left two and right two edges
        off_yuv[i*meanbmp.bi.biWidth*3] = yuv[i*meanbmp.bi.biWidth*3];
        off_yuv[i*meanbmp.bi.biWidth*3+3] = yuv[i*meanbmp.bi.biWidth*3+3];
        off_yuv[(i+1)*meanbmp.bi.biWidth*3-6] = yuv[ (i+1)* meanbmp.bi.biWidth*3-6];
        off_yuv[(i+1)*meanbmp.bi.biWidth*3-3] = yuv[ (i+1)* meanbmp.bi.biWidth*3-3];
    }
    for(int i=2;i<meanbmp.bi.biWidth-2;i++){//traverse the matrix
        for(int j=2;j<meanbmp.bi.biHeight-2;j++){
            double sum = 0;
            for(int n1=0;n1<5;n1++){//traverse the nearby pixels
                for(int n2=0;n2<5;n2++){
                    sum += yuv[3*meanbmp.bi.biWidth*(j+off[n1])+ 3*(i+off[n2])];
                }
            }
            sum /= 25;// get the average 
            off_yuv[3*i+j*meanbmp.bi.biWidth*3] = sum;//store
        }
    }
}