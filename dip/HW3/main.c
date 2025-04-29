#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
//declare
int trans_yuv_rgb(double *,unsigned char*,int);
int find_closest(int);
unsigned char * equalization(int*,int,unsigned char* );
int log_the_data(double*,int ,unsigned char);
#define DEVIDE 8 //this defines how many pieces the picture will be cutted into ,which is followed by binarilazing
#define SCALE 9 //the area of the structuring element matrix
typedef struct BMP{
    FILE* file;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
} BMP;
int write_gray_data(BMP bmp,double* graydata);
int write_rgbdata(BMP,unsigned char*);
int * count_rgb(unsigned char*,int n);
int write_data(BMP bmp,unsigned char* black_white);//write the prepared bmp data into file
int initial_bmp(BMP);//write the file header into the file 
BMP create_bmp();//read the first bmp in D://test.bmp
unsigned char fill_rgb_yuv_array(BMP,unsigned char*,double*);//read bmp data from the 24color file and transfer it into gray number array

int main(){
    BMP rawbmp = create_bmp();
    unsigned char *rgbdata= malloc (sizeof(unsigned char)*3*rawbmp.bi.biWidth*rawbmp.bi.biHeight);// to store the gray number for gray bmp
    unsigned char max;
    double *yuv= malloc (sizeof(double)*3*rawbmp.bi.biWidth*rawbmp.bi.biHeight);
    max = fill_rgb_yuv_array(rawbmp,rgbdata,yuv);//fill in the array
    FILE * grayfile =fopen("D://gray.bmp","wb");
    BMP graybmp = {grayfile,rawbmp.bf,rawbmp.bi};
    graybmp.bf.bfOffBits += 256 *sizeof(RGBQUAD);
    graybmp.bf.bfSize = graybmp.bf.bfOffBits + (graybmp.bi.biWidth+3/4*4) *graybmp.bi.biHeight;
    graybmp.bi.biBitCount = 8;
    graybmp.bi.biSizeImage =  (graybmp.bi.biWidth+3/4*4) *graybmp.bi.biHeight;
    initial_bmp(graybmp);
    RGBQUAD rgbquad[256];
    for(int i=0; i<256; i++){
        rgbquad[i].rgbBlue = i;
        rgbquad[i].rgbGreen = i;
        rgbquad[i].rgbRed = i;
        rgbquad[i].rgbReserved = 0 ;
    }
    fwrite(&rgbquad,sizeof(rgbquad),1,grayfile);
    write_gray_data(graybmp,yuv);
    FILE * logfile = fopen("D:\\log.bmp","wb");
    BMP logbmp={logfile,rawbmp.bf,rawbmp.bi};
    logbmp.bf.bfOffBits += 256 *sizeof(RGBQUAD);
    logbmp.bf.bfSize = logbmp.bf.bfOffBits + (logbmp.bi.biWidth+3/4*4) *logbmp.bi.biHeight;
    logbmp.bi.biBitCount = 8;
    logbmp.bi.biSizeImage =  (logbmp.bi.biWidth+3/4*4) *logbmp.bi.biHeight;
    initial_bmp(logbmp);
    fwrite(&rgbquad,sizeof(rgbquad),1,logfile);
    log_the_data(yuv,logbmp.bi.biWidth*logbmp.bi.biHeight,max);
    write_gray_data(logbmp,yuv+1);

    // FILE * equfile = fopen("D:\\equbmp.bmp","wb");
    // BMP equbmp = {equfile,rawbmp.bf,rawbmp.bi};
    // equbmp.bf.bfOffBits += 256 *sizeof(RGBQUAD);
    // equbmp.bf.bfSize = equbmp.bf.bfOffBits + (equbmp.bi.biWidth+3/4*4) *equbmp.bi.biHeight;
    // equbmp.bi.biBitCount = 8;
    // equbmp.bi.biSizeImage =  (equbmp.bi.biWidth+3/4*4) *logbmp.bi.biHeight;
    // initial_bmp(equbmp);
    // int *rgbcount = count_rgb(rgbdata,equbmp.bi.biWidth*equbmp.bi.biHeight);
    // int cnt1=0,cnt2=0,cnt3=0;
    // for(int i=0;i<256;i++){
    //     printf("%d %d %d\n",*(rgbcount+3*i),*(rgbcount+3*i+1),*(rgbcount+3*i+2));
    //     cnt1 += *(rgbcount+3*i);
    //     cnt2+=*(rgbcount+3*i+1);
    
    //     cnt3 +=*(rgbcount+3*i+2);
    // }
    // printf("%d %d %d \n",cnt1,cnt2,cnt3);
    // unsigned char *equaled_rgbdata = equalization(rgbcount,equbmp.bi.biWidth*equbmp.bi.biHeight,rgbdata);
    // for(int i=0;i<equbmp.bi.biWidth*equbmp.bi.biHeight;i++){
    //     printf("%d\n",equaled_rgbdata[i]);
    // }
    // write_rgbdata(equbmp,equaled_rgbdata);
    free(rgbdata);
    free(yuv);
}

//function
int find_closest(int n){
    return (n +0.5)/1;
}
unsigned char *equalization(int *rgbcount,int total,unsigned char * rgbdata){
    double s_rgbcount[256][3]={0};
    double tot = (double) total;
    int adjusted_rgbcount[256][3] = {0};
    for(int i=0;i<256;i++){
        if(i==0){
            s_rgbcount[0][0]= *rgbcount/tot;
            s_rgbcount[0][1]= *(rgbcount+1)/tot;
            s_rgbcount[0][2]= *(rgbcount+2)/tot;
        }else{
            s_rgbcount[i][0] = s_rgbcount[i-1][0] + *(rgbcount+3*i)/tot*1.0;
            s_rgbcount[i][1] = s_rgbcount[i-1][1] + *(rgbcount+3*i+1)/tot*1.0;
            s_rgbcount[i][2] = s_rgbcount[i-1][2] + *(rgbcount+3*i+2)/tot*1.0;
        }
        // printf("%lf %lf %lf\n",s_rgbcount[i][0],s_rgbcount[i][1],s_rgbcount[i][2]);
    }
    for(int i=0;i<256;i++){
        for(int j=0;j<3;j++){
            adjusted_rgbcount[i][j] = find_closest(256*s_rgbcount[i][j]);
        }
    }
    unsigned char* final_rgbdata = malloc(sizeof(unsigned char)*3*total);
    for(int i=0;i<total;i++){
        final_rgbdata[3*i] = adjusted_rgbcount[rgbdata[3*i]][0];
        final_rgbdata[3*i+1] = adjusted_rgbcount[rgbdata[3*i+1]][0];
        final_rgbdata[3*i+2] = adjusted_rgbcount[rgbdata[3*i+2]][0];
    }
    return final_rgbdata;
}
int log_the_data(double *yuv,int n,unsigned char max){
    for(int i=0; i<n;i++){
        yuv[3*i+1] = 256*log(yuv[3*i]+1)/log(max+1);
    }
}
int write_rgbdata(BMP bmp,unsigned char* rgbdata){
    int width = bmp.bi.biWidth*3;
    int adjusted_width = (width+3)/4*4;
    int count=0;
    for(int i=0;i<bmp.bi.biWidth*bmp.bi.biHeight;i++){
        fwrite(&rgbdata[3*i],sizeof(unsigned char),1,bmp.file);
        fwrite(&rgbdata[3*i+1],sizeof(unsigned char),1,bmp.file);
        fwrite(&rgbdata[3*i+2],sizeof(unsigned char),1,bmp.file);
        count+=3;
        if (count == width){// read the 0s that just take up places 
            while(count++ != adjusted_width){
                fwrite(rgbdata,1,1,bmp.file);
            }
            count = 0;
        }
    }
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
int * count_rgb(unsigned char* rgbdata,int n){
    printf("%d\n",n);
    int *countrgb=malloc(sizeof(int)*3*n);
    for(int i=0;i<n;i++){
        *(countrgb+rgbdata[3*i]*3+0)=0;
        *(countrgb+rgbdata[3*i+1]*3+1)=0;
        *(countrgb+rgbdata[3*i+2]*3+2)=0;
    }
    for(int i=0;i<n;i++){
        *(countrgb+rgbdata[3*i]*3+0)+=1;
        *(countrgb+rgbdata[3*i+1]*3+1)+=1;
        *(countrgb+rgbdata[3*i+2]*3+2)+=1;
    }
    return countrgb;
}

unsigned char fill_rgb_yuv_array(BMP rawbmp,unsigned char* rgbdata, double * yuv){
    int max = 0;
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
        if(y > max) 
            max = y;
        count+=3;
        if (count == width){// read the 0s that just take up places 
            while(count++ != adjusted_width){
                fread(&dustbin,1,1,rawbmp.file);
            }
            count = 0;
        }
    }
    return max;
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
int trans_yuv_rgb(double *yuv,unsigned char*rgb,int n ){
    for( int i=0; i<n; i++){
    //use the reverse matrix to change YUV to RGB 
    rgb[3*i+2] = 1*yuv[3*i] + 0*yuv[3*i+1] + 1.140*yuv[3*i+2];
    rgb[3*i+1] = 1*yuv[3*i] + -0.395*yuv[3*i+1] + -0.58*yuv[3*i+2];
    rgb[3*i] = 1.002*yuv[3*i] + 2.036*yuv[3*i+1] + 0*yuv[3*i+2];
    }
}