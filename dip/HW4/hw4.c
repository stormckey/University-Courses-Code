#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
//declare
typedef struct BMP{
    FILE* file;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
} BMP;
int write_rgbdata(BMP,unsigned char*);//write rgb data into the bmp
int initial_bmp(BMP);//write the file header into the file 
BMP create_bmp();//read the first bmp in D://test.bmp
int translate(unsigned char *,unsigned char *,BMP,BMP,int,int);
int mirror(BMP,BMP,unsigned char *,unsigned char *,int ,int);
int rotate(BMP,BMP,unsigned char *,unsigned char *,int,int,double,double);
int scale(BMP,BMP,unsigned char *, unsigned char*,int,int,double);
int shear(BMP,BMP,unsigned char *,unsigned char * ,int,int);
unsigned char* fill_rgb_array(BMP,unsigned char*);//read bmp data from the 24color file and transfer it into gray number array

int main(){
    BMP rawbmp = create_bmp();
    printf("%d %d\n",rawbmp.bi.biHeight,rawbmp.bi.biWidth);
    unsigned char * rgbdata= malloc(sizeof(unsigned char)*3*rawbmp.bi.biWidth*rawbmp.bi.biHeight);// to store the gray number for gray bmp
    fill_rgb_array(rawbmp,rgbdata);//fill in the array
    FILE * tranfile = fopen("D:\\tran.bmp","wb");
    BMP tranbmp={tranfile,rawbmp.bf,rawbmp.bi};
    tranbmp.bi.biWidth *= 2;
    tranbmp.bi.biHeight *= 2;
    tranbmp.bi.biSizeImage = (tranbmp.bi.biWidth+3)/4*4*tranbmp.bi.biHeight;
    tranbmp.bf.bfSize = tranbmp.bf.bfOffBits + tranbmp.bi.biHeight * tranbmp.bi.biWidth;
    initial_bmp(tranbmp);
    unsigned char * tranrgb = malloc(sizeof(unsigned char)*tranbmp.bi.biHeight*tranbmp.bi.biWidth*3);
    unsigned char white = 255;
    for(int i=0;i<tranbmp.bi.biWidth*tranbmp.bi.biHeight*3;i++){
        tranrgb[i]=white;
    }
    translate(tranrgb,rgbdata,rawbmp,tranbmp,rawbmp.bi.biWidth*3,0);
    write_rgbdata(tranbmp,tranrgb);
    FILE * rotafile = fopen ("D:\\rota.bmp","wb");
    BMP rotabmp = {rotafile,tranbmp.bf,tranbmp.bi};
    initial_bmp(rotabmp);
    unsigned char * rotargb = tranrgb;
    for(int i=0;i<tranbmp.bi.biWidth*tranbmp.bi.biHeight*3;i++){
        rotargb[i]=white;
    }
    rotate(rotabmp,rawbmp,rotargb,rgbdata,rawbmp.bi.biWidth/4*3,rawbmp.bi.biHeight/2,0.5,0.866025);
    write_rgbdata(rotabmp,rotargb);
    FILE * scalfile = fopen("D:\\scal.bmp","wb");
    BMP scalbmp = {scalfile,tranbmp.bf,tranbmp.bi};
    initial_bmp(scalbmp);
    unsigned char * scalrgb = rotargb;
    for(int i=0;i<tranbmp.bi.biWidth*tranbmp.bi.biHeight*3;i++){
        scalrgb[i]=white;
    }
    scale(scalbmp,rawbmp,scalrgb,rgbdata,0,0,1.5);
    write_rgbdata(scalbmp,scalrgb);
    FILE * sheafile = fopen("D:\\shea.bmp","wb");
    BMP sheabmp = {sheafile,tranbmp.bf,tranbmp.bi};
    initial_bmp(sheabmp);
    unsigned char * sheargb = tranrgb;
    for(int i=0;i<tranbmp.bi.biWidth*tranbmp.bi.biHeight*3;i++){
        sheargb[i]=white;
    }
    shear(sheabmp,rawbmp,sheargb,rgbdata,0,0);
    write_rgbdata(sheabmp,sheargb);
    FILE * mirrfile = fopen("D:\\mirr.bmp","wb");
    BMP mirrbmp = {mirrfile,tranbmp.bf,tranbmp.bi};
    initial_bmp(mirrbmp);
    unsigned char * mirrrgb = tranrgb;
    for(int i=0;i<tranbmp.bi.biWidth*tranbmp.bi.biHeight*3;i++){
        mirrrgb[i]=white;
    }
    mirror(mirrbmp,rawbmp,mirrrgb,rgbdata,0,0);
    write_rgbdata(mirrbmp,mirrrgb);
    free(tranrgb);
    free(rgbdata);
}

//function
int mirror(BMP newbmp,BMP oldbmp,unsigned char *mirrrgb,unsigned char *rgbdata,int x,int y){
    int newoff = newbmp.bi.biWidth*3;
    int oldoff = oldbmp.bi.biWidth*3;
    for(int i=0;i<oldbmp.bi.biHeight;i++){
        for(int j=0;j<oldbmp.bi.biWidth;j++){
            *(mirrrgb + (i+y)*newoff + (j+x)*3) = *(rgbdata + (oldbmp.bi.biHeight-i)*oldoff + (oldbmp.bi.biWidth - j)*3);
            *(mirrrgb + (i+y)*newoff + (j+x)*3+1) = *(rgbdata + (oldbmp.bi.biHeight-i)*oldoff + (oldbmp.bi.biWidth - j)*3+1);
            *(mirrrgb + (i+y)*newoff + (j+x)*3+2) = *(rgbdata + (oldbmp.bi.biHeight-i)*oldoff + (oldbmp.bi.biWidth - j)*3+2);
        }
    }
}
int shear(BMP newbmp,BMP oldbmp,unsigned char * sheargb,unsigned char * rgbdata,int x,int y){
    int newoff = newbmp.bi.biWidth*3;
    int oldoff = oldbmp.bi.biWidth*3;
    for(int i=0;i<oldbmp.bi.biHeight;i++){
        for(int j=0;j<oldbmp.bi.biWidth;j++){
            *(sheargb + (i+y)*newoff + (j+x+i)*3) = *(rgbdata + i*oldoff + j*3);
            *(sheargb + (i+y)*newoff + (j+x+i)*3+1) = *(rgbdata + i*oldoff + j*3+1);
            *(sheargb + (i+y)*newoff + (j+x+i)*3+2) = *(rgbdata + i*oldoff + j*3+2);
        }
    }
}
int scale(BMP newbmp,BMP oldbmp,unsigned char* scalrgb,unsigned char *rgbdata,int x, int y ,double ratio){
    int newoff = newbmp.bi.biWidth*3;
    int oldoff = oldbmp.bi.biWidth*3;
    if(ratio < 1){

    }else{
        for(int i=0;i<oldbmp.bi.biHeight;i++){
            for(int j=0;j<oldbmp.bi.biWidth;j++){
                *(scalrgb +(int)(i*ratio + y)*newoff +(int)(j*ratio+x)*3) = *(rgbdata + i*oldoff + j*3);
                *(scalrgb +(int)(i*ratio + y)*newoff +(int)(j*ratio+x)*3+1) = *(rgbdata + i*oldoff + j*3+1);
                *(scalrgb +(int)(i*ratio + y)*newoff +(int)(j*ratio+x)*3+2) = *(rgbdata + i*oldoff + j*3+2);
            }
        }
    }
}
int rotate(BMP newbmp,BMP oldbmp,unsigned char * rotargb,unsigned char * rgbdata,int x,int y,double sin,double cos){
    int newoff = newbmp.bi.biWidth*3;
    int oldoff = oldbmp.bi.biWidth*3;
    for(int i=0;i<oldbmp.bi.biHeight;i++){
        for(int j=0;j<oldbmp.bi.biWidth;j++){
            int newx = j*cos - i*sin;
            int newy = j*sin + i*cos;
            *(rotargb + (newy+y)*newoff + (newx + x)*3) = *(rgbdata +i*oldoff + j*3);
            *(rotargb + (newy+y)*newoff + (newx + x)*3+1) = *(rgbdata +i*oldoff + j*3+1);
            *(rotargb + (newy+y)*newoff + (newx + x)*3+2) = *(rgbdata +i*oldoff + j*3+2);
        }
    }
}
int translate(unsigned char *tranrgb,unsigned char * rgbdata,BMP oldbmp,BMP newbmp,int x,int y){
    int newoff = newbmp.bi.biWidth*3;
    int oldoff = oldbmp.bi.biWidth*3;
    // printf("%d %d %d %d",off,newbmp.bi.biHeight,oldbmp.bi.biWidth,oldbmp.bi.biHeight);
    for(int i=0;i<oldbmp.bi.biHeight;i++){
        for(int j = 0; j<oldbmp.bi.biWidth*3;j++){
            *(tranrgb + (i+y)*newoff + j + x) = *(rgbdata + i*oldoff + j);
        }
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
unsigned char* fill_rgb_array(BMP rawbmp,unsigned char* rgbdata){
    int width = rawbmp.bi.biWidth*3;
    int adjusted_width = (width+3)/4*4;
    unsigned char dustbin;
    int count=0;
    for(int i=0; i< rawbmp.bi.biWidth * rawbmp.bi.biHeight; i++){
        fread(&rgbdata[3*i],1,1,rawbmp.file);// read the  rgb number
        fread(&rgbdata[3*i+1],1,1,rawbmp.file);
        fread(&rgbdata[3*i+2],1,1,rawbmp.file);
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
