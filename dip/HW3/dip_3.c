#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
//declare
int find_closest(int);//find the closest gray number of the sk*256
unsigned char * equalization(int*,int,unsigned char* );//the main function to do the equalization
int log_the_data(unsigned char*,unsigned char*,int ,unsigned char *);// the generate the rgbdata after logrization
#define DEVIDE 8 //this defines how many pieces the picture will be cutted into ,which is followed by binarilazing
#define SCALE 9 //the area of the structuring element matrix
typedef struct BMP{
    FILE* file;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
} BMP;
int write_rgbdata(BMP,unsigned char*);//write rgb data into the bmp
int * count_rgb(unsigned char*,int n);//count the number of rgb 
int write_data(BMP bmp,unsigned char* black_white);//write the prepared bmp data into file
int initial_bmp(BMP);//write the file header into the file 
BMP create_bmp();//read the first bmp in D://test.bmp
unsigned char* fill_rgb_array(BMP,unsigned char*);//read bmp data from the 24color file and transfer it into gray number array

int main(){
    BMP rawbmp = create_bmp();
    unsigned char rgbdata[3*rawbmp.bi.biWidth*rawbmp.bi.biHeight];// to store the gray number for gray bmp
    unsigned char *max;// store the max intensity of rgb
    max = fill_rgb_array(rawbmp,rgbdata);//fill in the array
    FILE * logfile = fopen("D:\\log.bmp","wb");
    BMP logbmp={logfile,rawbmp.bf,rawbmp.bi};
    initial_bmp(logbmp);
    unsigned char *log_rgb=malloc(sizeof(unsigned char)*3*logbmp.bi.biWidth* logbmp.bi.biHeight);//store the rgbdata after logrization
    log_the_data(log_rgb,rgbdata,logbmp.bi.biWidth*logbmp.bi.biHeight,max);//do the logrization
    write_rgbdata(logbmp,log_rgb);

    FILE * equfile = fopen("D:\\equbmp.bmp","wb");
    BMP equbmp = {equfile,rawbmp.bf,rawbmp.bi};
    initial_bmp(equbmp);
    int *rgbcount = count_rgb(rgbdata,equbmp.bi.biWidth*equbmp.bi.biHeight);//make the histogram
    unsigned char *equaled_rgbdata = equalization(rgbcount,equbmp.bi.biWidth*equbmp.bi.biHeight,rgbdata);// do the equalization
    write_rgbdata(equbmp,equaled_rgbdata);
}

//function
int find_closest(int n){
    return (n +0.5)/1;//round off
}
unsigned char *equalization(int *rgbcount,int total,unsigned char * rgbdata){
    double s_rgbcount[256][3]={0};//store the sk
    double tot = (double) total;
    int adjusted_rgbcount[256][3] = {0};
    for(int i=0;i<256;i++){
        if(i==0){//initial
            s_rgbcount[0][0]= *rgbcount/tot;
            s_rgbcount[0][1]= *(rgbcount+1)/tot;
            s_rgbcount[0][2]= *(rgbcount+2)/tot;
        }else{//latter
            s_rgbcount[i][0] = s_rgbcount[i-1][0] + *(rgbcount+3*i)/tot*1.0;
            s_rgbcount[i][1] = s_rgbcount[i-1][1] + *(rgbcount+3*i+1)/tot*1.0;
            s_rgbcount[i][2] = s_rgbcount[i-1][2] + *(rgbcount+3*i+2)/tot*1.0;
        }
    }
    for(int i=0;i<256;i++){// find the adjusted intensity of rgb
        for(int j=0;j<3;j++){
            adjusted_rgbcount[i][j] = find_closest(256*s_rgbcount[i][j]);
        }
    }
    unsigned char* final_rgbdata = malloc(sizeof(unsigned char)*3*total);
    for(int i=0;i<total;i++){//adjust the rgbdata
        final_rgbdata[3*i] = adjusted_rgbcount[rgbdata[3*i]][0];
        final_rgbdata[3*i+1] = adjusted_rgbcount[rgbdata[3*i+1]][0];
        final_rgbdata[3*i+2] = adjusted_rgbcount[rgbdata[3*i+2]][0];
    }
    return final_rgbdata;
}
int log_the_data(unsigned char * logrgb,unsigned char* rgbdata,int n,unsigned char* max){
    for(int i=0; i<n;i++){
        logrgb[3*i] = 256*log(rgbdata[3*i]+1)/log(max[0]+1);
        logrgb[3*i+1] = 256*log(rgbdata[3*i+1]+1)/log(max[1]+1);
        logrgb[3*i+2] = 256*log(rgbdata[3*i+2]+1)/log(max[2]+1);
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
int * count_rgb(unsigned char* rgbdata,int n){
    int *countrgb=malloc(sizeof(int)*3*n);
    for(int i=0;i<n;i++){//initialize it to 0
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

unsigned char* fill_rgb_array(BMP rawbmp,unsigned char* rgbdata){
    unsigned char  redmax=0,greenmax=0,bluemax=0;
    unsigned char * max = malloc(sizeof(unsigned char)*3);
    int width = rawbmp.bi.biWidth*3;
    int adjusted_width = (width+3)/4*4;
    unsigned char dustbin;
    int count=0;
    for(int i=0; i< rawbmp.bi.biWidth * rawbmp.bi.biHeight; i++){
        fread(&rgbdata[3*i],1,1,rawbmp.file);// read the  rgb number
        fread(&rgbdata[3*i+1],1,1,rawbmp.file);
        fread(&rgbdata[3*i+2],1,1,rawbmp.file);
        if(rgbdata[3*i] >bluemax)// find the max rgb at the same time
            bluemax = rgbdata[3*i];
        if (rgbdata[3*i+1] > greenmax)
            greenmax = rgbdata[3*i+1];
        if (rgbdata[3*i+2] > redmax)
            redmax = rgbdata[3*i+2];
        count+=3;
        if (count == width){// read the 0s that just take up places 
            while(count++ != adjusted_width){
                fread(&dustbin,1,1,rawbmp.file);
            }
            count = 0;
        }
    }
    max[0] = bluemax;
    max[1] = greenmax;
    max[2] = redmax;
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
