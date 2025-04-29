//include 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
//declare
#define DEVIDE 8 //this defines how many pieces the picture will be cutted into ,which is followed by binarilazing
#define SCALE 9 //the area of the structuring element matrix
typedef struct BMP{
    FILE* file;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
} BMP;
typedef struct MIN_MAX{
    int min;
    int max;
}MIN_MAX; // to store the max and min num of gray number
typedef struct THR{
    int threshold;
    double sigma;
}THR; // to store each threshold together with sigma
int write_data(BMP bmp,unsigned char* black_white);//write the prepared bmp data into file
int test_overlap_0(unsigned char *m1,unsigned char *m2,int row, int column,int off1, int off2);// to check if the 0s of picture has overlapped the structure element
int test_overlap_1(unsigned char* m1,unsigned char*m2,int row,int column,int off1,int off2);//to check if the 1s of the picture has overlapped the structure element
int initial_bmp(BMP);//write the file header into the file 
int erosion(unsigned char* eromatrix,unsigned char * black_white,unsigned char * structure_element,BMP erobmp);
int dilation(unsigned char* dilmatrix,unsigned char * black_white,unsigned char * structure_element,BMP dilbmp);
BMP create_bmp();//read the first bmp in D://test.bmp
MIN_MAX min_max(unsigned char *,int);//find the max and min bound of the array
int fill_gray_array(BMP,unsigned char*);//read bmp data from the 24color file and transfer it into gray number array
int write_gray_palette(BMP);//write gray palette into the file 
unsigned char* copy_matrix(unsigned char *,int ,int,int);// copy part of a big matrix and store it into a small matrix
int transfer(unsigned char *,int,int,int,int,unsigned char *,BMP );//transfer the gray number array into binary number array

int main(){
    BMP rawbmp = create_bmp();
    unsigned char gray[rawbmp.bi.biWidth * rawbmp.bi.biHeight];// to store the gray number for gray bmp
    fill_gray_array(rawbmp,gray);//fill in the array
    FILE * bifile = fopen("D:\\binary.bmp","wb");
    BMP bibmp={bifile,rawbmp.bf,rawbmp.bi};
    bibmp.bf.bfOffBits = 14 + 40 + 256*sizeof(RGBQUAD);//modify the file header for new file
    int new_width = (bibmp.bi.biWidth +3)/4*4;
    bibmp.bf.bfSize = bibmp.bf.bfOffBits + new_width*bibmp.bi.biHeight;
    bibmp.bi.biBitCount = 8;
    bibmp.bi.biSizeImage = new_width*bibmp.bi.biHeight;
    initial_bmp(bibmp);
    write_gray_palette(bibmp);
    unsigned char black_white[bibmp.bi.biWidth*bibmp.bi.biHeight];//to store the binary number array
    int a=bibmp.bi.biWidth/DEVIDE, b=bibmp.bi.biHeight/DEVIDE; //cut the picture
    int x=-a;
    for(int i=0; i<DEVIDE;i++){
        x+=a;
        int y=-b;
        for(int j=0;j<DEVIDE;j++){
            y+=b;
            if(i==DEVIDE-1)//adjust the width and height if edge piece is met
                a+=bibmp.bi.biWidth%DEVIDE;
            if(j==DEVIDE-1)
                b+=bibmp.bi.biHeight%DEVIDE;
            unsigned char * copymatrix=copy_matrix(gray+y*bibmp.bi.biWidth+x,a,b,bibmp.bi.biWidth);//copy one piece of the big matrix
            transfer(copymatrix,a,b,x,y,black_white,bibmp);//transfer it into binary number and save in black_white array
            a=bibmp.bi.biWidth/DEVIDE;//cover the adjustment
            b=bibmp.bi.biHeight/DEVIDE;
            free(copymatrix);
        }
    }
    write_data(bibmp,black_white);//the binarilized bmp is completed

    unsigned char structure_element[SCALE] = {0,1,0,   1,1,1,   0,1,0};// the structure element
    FILE * erofile = fopen("D:\\erosion.bmp","wb");
    BMP erobmp={erofile,bibmp.bf,bibmp.bi};
    initial_bmp(erobmp);
    write_gray_palette(erobmp);
    unsigned char *eromatrix = malloc(erobmp.bi.biWidth*erobmp.bi.biHeight);// to store the binary number array of erosed bmp
    erosion(eromatrix,black_white,structure_element,erobmp);
    write_data(erobmp,eromatrix);
    free(eromatrix);


    FILE * dilfile = fopen("D:\\dilation.bmp","wb");
    BMP dilbmp={dilfile,bibmp.bf,bibmp.bi};
    initial_bmp(dilbmp);
    write_gray_palette(dilbmp);
    unsigned char *dilmatrix= malloc (dilbmp.bi.biWidth*dilbmp.bi.biHeight);// to store the binary number array of dilated file
    dilation(dilmatrix,black_white,structure_element,dilbmp);
    write_data(dilbmp,dilmatrix);
    free(dilmatrix);


    FILE * openfile = fopen("D:\\open.bmp","wb");
    BMP openbmp={openfile,bibmp.bf,bibmp.bi};
    initial_bmp(openbmp);
    write_gray_palette(openbmp);
    unsigned char *openmatrix= malloc (openbmp.bi.biWidth*openbmp.bi.biHeight);
    unsigned char * tempmatrix = malloc(openbmp.bi.biWidth*openbmp.bi.biHeight);
    erosion(tempmatrix,black_white,structure_element,openbmp);  //the erosed data is stored in tempmatrix
    dilation(openmatrix,tempmatrix,structure_element,openbmp); // dilate the erosed data
    write_data(openbmp,openmatrix);
    free(openmatrix);

    FILE * closefile = fopen("D:\\close.bmp","wb");
    BMP closebmp={closefile,bibmp.bf,bibmp.bi};
    initial_bmp(closebmp);
    write_gray_palette(closebmp);
    unsigned char *closematrix= malloc (closebmp.bi.biWidth*closebmp.bi.biHeight);
    dilation(tempmatrix,black_white,structure_element,closebmp); // the dilated data is stored in tempmatrix
    erosion(closematrix,tempmatrix,structure_element,closebmp); // erose the dilated data
    write_data(closebmp,closematrix);
    free(closematrix);
    free(tempmatrix);
}

//function
unsigned char* copy_matrix(unsigned char * gray, int a,int b,int width){
    unsigned char * new =malloc(a*b);
    for(int i=0;i<a;i++){
        for(int j=0; j<b;j++){
            *(new+j*a+i) = *(gray+i+j*width);
        }
    }
    return new;
}

int transfer(unsigned char *copymatrix,int a,int b,int x,int y,unsigned char*black_white,BMP bibmp){
    MIN_MAX range = min_max(copymatrix,a*b); //the range of thresholds
    THR best ={0,0};
    for(int i=range.min;i<=range.max;i++){
        THR temp;
        temp.threshold = i;
        double f_num=0,b_num=0,f_sum=0,b_sum=0;//count the data of the threshold
        for(int j=0;j<a*b;j++){
            if (copymatrix[j]>i){
                b_num ++;
                b_sum += copymatrix[j];
            }else {
                f_num ++;
                f_sum += copymatrix[j];
            }
        }
    temp.sigma = b_num*f_num/(b_num+f_num)/(b_num+f_num)*(f_sum/f_num-b_sum/b_num)*(f_sum/f_num-b_sum/b_num);// the current sigma
    if(temp.sigma > best.sigma)
        best = temp;
    }// best will store the max sigma
    black_white += x + y*bibmp.bi.biWidth;//change the matrix into binary number using the best threshold
    for(int i=0;i<a;i++){
        for(int j=0;j<b;j++){
            if(*(copymatrix+i+j*a)>best.threshold)
                *(black_white + i +j*bibmp.bi.biWidth) = 255;
            else 
                *(black_white + i + j *bibmp.bi.biWidth) = 0; 
        }
    }
}

int fill_gray_array(BMP rawbmp,unsigned char* gray){
    unsigned char  red,green,blue;
    int width = rawbmp.bi.biWidth*3;
    int adjusted_width = (width+3)/4*4;
    int count=0;
    for(int i=0; i< rawbmp.bi.biWidth * rawbmp.bi.biHeight; i++){
        fread(&blue,1,1,rawbmp.file);// read the  rgb number
        fread(&green,1,1,rawbmp.file);
        fread(&red,1,1,rawbmp.file);
        gray[i] = 0.299*red + 0.587*green + 0.114*blue ; //calculate the gray number 
        count+=3;
        if (count == width){// read the 0s that just take up places 
            while(count++ != adjusted_width){
                fread(&red,1,1,rawbmp.file);
            }
            count = 0;
        }
    }
}

int test_overlap_1(unsigned char *m1,unsigned char *m2,int row, int column,int off1, int off2){// check if there is a 1 overlap in structure element and the bmp
    for(int i=0;i<row*column;i++){
        if(*(m1+i%column+i/column*off1) * *(m2+i%column+i/column*off2))
            return 1;
    }
    return 0;
}

int test_overlap_0(unsigned char *m1,unsigned char *m2,int row, int column,int off1, int off2){//check if there is a 0 overlap in structure element and the bmp
    for(int i=0;i<row*column;i++){
        if(!(*(m1+i%column+i/column*off1) * *(m2+i%column+i/column*off2))&&*(m2+i%column+i/column*off2))
            return 1;
    }
    return 0;
}

int write_gray_palette(BMP bibmp){
    RGBQUAD rgbquad[256];
    for(int i=0; i<256; i++){
        rgbquad[i].rgbBlue = i;
        rgbquad[i].rgbGreen = i;
        rgbquad[i].rgbRed = i;
        rgbquad[i].rgbReserved = 0 ;
    }
    fwrite(rgbquad,sizeof(rgbquad),1,bibmp.file);
}

MIN_MAX min_max(unsigned char *arr, int n){
    int bottom = 255,up = 0;
    for(int i=0;i<n;i++){
        if(*(arr+i)< bottom)
            bottom = *(arr+i);
        if(*(arr+i) > up)
            up = *(arr+i);
    }
    MIN_MAX result ={bottom,up};
    return result;
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

int erosion(unsigned char *eromatrix,unsigned char * black_white,unsigned char* structure_element,BMP erobmp){
    for(int i=0;i<erobmp.bi.biWidth;i++){// copy the places that wont be erosed
        *(eromatrix + i) = *(black_white +i);
        *(eromatrix + i + (erobmp.bi.biHeight-1)*erobmp.bi.biWidth) = *(black_white + i +(erobmp.bi.biHeight-1)*erobmp.bi.biWidth);
    }
    for(int i=0;i<erobmp.bi.biHeight;i++){//copy the left places that wont be erosed
        *(eromatrix + i*erobmp.bi.biWidth) = *(black_white + i*erobmp.bi.biWidth);
        *(eromatrix + i*erobmp.bi.biWidth+erobmp.bi.biWidth - 1) = *(black_white + i*erobmp.bi.biWidth + erobmp.bi.biWidth - 1);
    }
    for(int i=0;i<erobmp.bi.biWidth-3;i++){//erosion
        for(int j=0;j<erobmp.bi.biHeight-3;j++){
            if(test_overlap_1(black_white+i+j*erobmp.bi.biWidth,structure_element,3,3,erobmp.bi.biWidth,3))
                *(eromatrix + i+1 + (j+1)*erobmp.bi.biWidth) = 255;
            else    
                *(eromatrix + i+1 + (j+1)*erobmp.bi.biWidth) = 0;
        }
    }
}
int dilation(unsigned char* dilmatrix,unsigned char * black_white,unsigned char * structure_element,BMP dilbmp){
    for(int i=0;i<dilbmp.bi.biWidth;i++){//copy the places that wont be dilated
        *(dilmatrix + i) = *(black_white +i);
        *(dilmatrix + i + (dilbmp.bi.biHeight-1)*dilbmp.bi.biWidth) = *(black_white + i +(dilbmp.bi.biHeight-1)*dilbmp.bi.biWidth);
    }
    for(int i=0;i<dilbmp.bi.biHeight;i++){//copy the left places that wont be dilated
        *(dilmatrix + i*dilbmp.bi.biWidth) = *(black_white + i*dilbmp.bi.biWidth);
        *(dilmatrix + i*dilbmp.bi.biWidth+dilbmp.bi.biWidth - 1) = *(black_white + i*dilbmp.bi.biWidth + dilbmp.bi.biWidth - 1);
    }
    for(int i=0;i<dilbmp.bi.biWidth-3;i++){//dilation
        for(int j=0;j<dilbmp.bi.biHeight-3;j++){
            if(test_overlap_0(black_white+i+j*dilbmp.bi.biWidth,structure_element,3,3,dilbmp.bi.biWidth,3))
                *(dilmatrix + i+1 + (j+1)*dilbmp.bi.biWidth) = 0;
            else    
                *(dilmatrix + i+1 + (j+1)*dilbmp.bi.biWidth) = 255;
        }
    }
}
int write_data(BMP bmp,unsigned char* black_white){
    int new_width = (bmp.bi.biWidth+3)/4*4;
    int count=0;
    unsigned char zero = 0;
    for(int i=0; i<bmp.bi.biSizeImage; i++){
        fwrite(&black_white[i],1,1,bmp.file);
        count++;
        if(count == bmp.bi.biWidth){// put 0s that take up places
            while(count++ != new_width){
                fwrite(&zero,1,1,bmp.file);
            }
            count=0;
        }
    }
}