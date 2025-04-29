//this program can only read a real color bmp file
//the test.bmp which is the source image must exist
//the program will create gray.bmp and new.bmp 
//the gray.bmp will contain the gray image 
//the new.bmp will contain the image whose Y is altered

#include <stdio.h>
#include <windows.h>

int main(){
    //the next three variables are for the source image
    FILE * oldbmp;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    //the source image should be stored in D:
    if( !(oldbmp = fopen("D:\\test.bmp","rb"))){
        printf("Not find file in D:\\test.bmp");
        return 0;
    }
    fread(&bf,sizeof(BITMAPFILEHEADER),1,oldbmp);//read the header of bmp
    fread(&bi,sizeof(BITMAPINFOHEADER),1,oldbmp);
    printf("%p %p",bf.bfType,bf.bfSize);
    // prepare to create the gray image
    BITMAPFILEHEADER graybf = bf;
    BITMAPINFOHEADER graybi = bi;
    //Because we use the Palette ,we need to change the value of Offset、SizeImage、BitCount、Size
    graybf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
    graybf.bfSize = (bf.bfSize -bf.bfOffBits ) * 8 / bi.biBitCount + graybf.bfOffBits;
    graybi.biBitCount = 8;
    graybi.biSizeImage = bi.biWidth * bi.biHeight;
    //Create the Palette for gray image 
    RGBQUAD rgbquad[256];
    for(int i=0; i<256; i++){
        rgbquad[i].rgbBlue = i;
        rgbquad[i].rgbGreen = i;
        rgbquad[i].rgbRed = i;
        rgbquad[i].rgbReserved = 0 ;
    }
    //Create the file and write the metadata
    FILE * grayf = fopen("D:\\gray.bmp","wb");
    fwrite(&graybf,sizeof(BITMAPFILEHEADER),1,grayf);
    fwrite(&graybi,sizeof(BITMAPINFOHEADER),1,grayf);
    fwrite(rgbquad,sizeof(rgbquad),1,grayf);
    //prepare to read the rgb from source image and calculate the YUV
    unsigned char red,green,blue,y;
    float yuv[bi.biSizeImage];
    for(int i =0; i< graybi.biSizeImage; i++){
        //remember that the color is stored in BGR
        fread(&blue,1,1,oldbmp);
        fread(&green,1,1,oldbmp);
        fread(&red,1,1,oldbmp);
        //y will store the luminance and be writen into gray image
        unsigned char y = 0.299*red + 0.587*green + 0.114*blue;
        yuv[3*i] = 0.299*red + 0.587*green + 0.114*blue ;
        yuv[3*i+1] = -0.147*red + -0.289*green + 0.435 *blue;
        yuv[3*i+2] = 0.615*red + -0.515*green + -0.1*blue;
        fwrite(&y,1,1,grayf);
    }
    //up to now , we have create the gray image and the YUV data array
    //prepare to create the image with altered Y
    BITMAPFILEHEADER newbf = bf;
    BITMAPINFOHEADER newbi = bi;
    unsigned char new_color[newbi.biSizeImage];
    for( int i=0; i<bi.biHeight*bi.biWidth; i++){
        //add 10 to every Y if Y is bigger than 255 ,set it 255 
        if((yuv[3*i] += 25 ) > 255){
            yuv[3*i] = 255;
        }
        //use the reverse matrix to change YUV to RGB 
        new_color[3*i+2] = 1*yuv[3*i] + 0*yuv[3*i+1] + 1.140*yuv[3*i+2];
        new_color[3*i+1] = 1*yuv[3*i] + -0.395*yuv[3*i+1] + -0.58*yuv[3*i+2];
        new_color[3*i] = 1.002*yuv[3*i] + 2.036*yuv[3*i+1] + 0*yuv[3*i+2];
    }
    //create the new file and write the data into it
    FILE * newbmp = fopen("D:\\new.bmp","wb");
    fwrite(&newbf,sizeof(BITMAPFILEHEADER),1,newbmp);
    fwrite(&newbi,sizeof(BITMAPINFOHEADER),1,newbmp);
    fwrite(&new_color,sizeof(new_color),1,newbmp);
}