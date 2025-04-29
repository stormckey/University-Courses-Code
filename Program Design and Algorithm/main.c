#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

/*
定义数组大小
*/

#define si 2000000

/*
定义结构q_block，用于传入参数给快排-并行
*/

typedef struct{
    int begin;
    int end;
    int *a;
    int time;
} q_block;

/*
定义四个全局数组，定义四个是因为随机生成的数据放入四组内
保证四个函数处理的数据是同一组
*/

int data1[si];
int data2[si];
int data3[si];
int data4[si];

/*
申明函数，从上到下分别是，归并-串行（si），归并-串行（si/4），
归并-并行，快排-并行，快排-串行
*/

void sort_g(int *a);
void *sort_gg(void *a);
void sort_gn(int *a);
void *sort_q(void *args);
int sort_qn(int a[],int begin,int end);

/*
定义获取时间的函数
*/

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
  time_t clock;
  struct tm tm;
  SYSTEMTIME wtm;
  GetLocalTime(&wtm);
  tm.tm_year   = wtm.wYear - 1900;
  tm.tm_mon   = wtm.wMonth - 1;
  tm.tm_mday   = wtm.wDay;
  tm.tm_hour   = wtm.wHour;
  tm.tm_min   = wtm.wMinute;
  tm.tm_sec   = wtm.wSecond;
  tm. tm_isdst  = -1;
  clock = mktime(&tm);
  tp->tv_sec = clock;
  tp->tv_usec = wtm.wMilliseconds * 1000;
  return (0);
}
#endif

double get_time()
{
#ifndef CLOCK
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec + t.tv_usec/1000000.0;
#else
    return (double)clock()/CLOCKS_PER_SEC;
#endif
}

/*
下面是主函数
*/

int main(int argc,char* argv[]){
    double begin,end;

    /*
    进行32轮循环，生成32组数据
    */    
    
    for(int m=0;m<32;m++){

        /*
        生成随机种子，并且生成四组数据
        */

        srand(time(0));
        for (int i=0; i<si; i++ ) {
            data1[i] =data2[i]=data3[i]=data4[i]= rand()+rand()*32767;
        }

        /*
        下面是并行-归并
        */

        begin=get_time();
        sort_gn(data1);
        end=get_time();
        printf("%lf\t",end-begin);

        /*
        下面是串行-归并
        */

        begin=get_time();
        sort_g(data2);
        end=get_time();
        printf("%lf\t",end-begin);

        /*
        下面是并行-快排
        */
        
        begin=get_time();
        q_block first;
        first.a=data3;
        first.begin=0;
        first.end=si-1;
        first.time=0;
        sort_q(&first);
        end=get_time();
        printf("%lf\t",end-begin);

        /*
        下面是串行-快排
        */
        
        begin=get_time();
        sort_qn(data4,0,si-1);
        end=get_time();
        printf("%lf\t",end-begin);


        printf("\n");
    }
}

/*
下面是普通的归并-串行，不过这里是针对si大小的排序
*/

void sort_g(int *a){
    int *b=(int *)malloc(si*sizeof(int));
    for(int i=2;i<=si*2;i*=2){
        for(int k=0;k<si;k+=i){
            int begin=k,end=k+i;
            if(end>si){
                end=si;
            }
            int middle=begin+i/2;
            int p1=begin,p2=middle;
            int p3=0;
            while(p1<middle&&p2<end){
                if(a[p1]<=a[p2]){
                    b[p3]=a[p1];
                    p1++;
                    p3++;
                }else{
                    b[p3]=a[p2];
                    p2++;
                    p3++;
                }
            }
            while(p1<middle){
                a[--p2]=a[--middle];
            }
            while(p3>0){
                a[--p2]=b[--p3];
            }
        }
    }
    free(b);
}

/*
下面是普通的归并-串行，但针对的数组大小是si/4，
用于归并-并行中每个线程单独调用
*/

void *sort_gg(void *p){
    int *a=(int *)p;
    int n=si/4;
    int *b=(int *)malloc(n*sizeof(int));
    for(int i=2;i<=n*2;i*=2){
        for(int k=0;k<n;k+=i){
            int begin=k,end=k+i;
            if(end>n){
                end=n;
            }
            int middle=begin+i/2;
            int p1=begin,p2=middle;
            int p3=0;
            while(p1<middle&&p2<end){
                if(a[p1]<=a[p2]){
                    b[p3]=a[p1];
                    p1++;
                    p3++;
                }else{
                    b[p3]=a[p2];
                    p2++;
                    p3++;
                }
            }
            while(p1<middle){
                a[--p2]=a[--middle];
            }
            while(p3>0){
                a[--p2]=b[--p3];
            }
        }
    }
    free(b);
}

/*
下面是归并-并行
*/

void sort_gn(int *a){

    /*
    创建四个线程，并把数组分为四分传入，各自进行排序
    */

    pthread_t a1,a2,a3,a4;
    pthread_create(&a1,NULL,sort_gg,a);
    pthread_create(&a2,NULL,sort_gg,&(a[si/4]));
    pthread_create(&a3,NULL,sort_gg,&(a[si/2]));
    pthread_create(&a4,NULL,sort_gg,&(a[3*si/4]));
	pthread_join(a1,NULL);
    pthread_join(a2,NULL);
    pthread_join(a3,NULL);
    pthread_join(a4,NULL);

    /*
    再次进行合并，把四个排好序的数组合并起来
    */

    int *b=(int *)malloc(si*sizeof(int));
    for(int i=si/4;i<=si*2;i*=2){
        for(int k=0;k<si;k+=i){
            int begin=k,end=k+i;
            if(end>si){
                end=si;
            }
            int middle=begin+i/2;
            int p1=begin,p2=middle;
            int p3=0;
            while(p1<middle&&p2<end){
                if(a[p1]<=a[p2]){
                    b[p3]=a[p1];
                    p1++;
                    p3++;
                }else{
                    b[p3]=a[p2];
                    p2++;
                    p3++;
                }
            }
            while(p1<middle){
                a[--p2]=a[--middle];
            }
            while(p3>0){
                a[--p2]=b[--p3];
            }
        }
    }
    free(b);
}

/*
下面是普通的快排-串行
*/

int sort_qn(int a[],int begin,int end){
    if(begin>=end){
        return 0;
    }
    int pivot=a[end];
    int i=begin,j=end-1;
    while(j!=i){
        while(a[i]<=pivot&&i<j){
            i++;
        }
        while(a[j]>=pivot&&i<j){
            j--;
        }
        if(i<j){
            int t=a[i];
            a[i]=a[j];
            a[j]=t;
        }
    }
    a[end]=a[i];
    a[i]=pivot;
    sort_qn(a,begin,i-1);
    sort_qn(a,i+1,end);
}

/*
下面是快排-并行
*/

void *sort_q(void*args){

    /*
    将传来的void*转化成可用的参数
    */

    q_block *o=(q_block*) args;
    int *a=o->a;
    int begin=o->begin;
    int end=o->end;
    int time=o->time;

    /*
    base case
    */

    if(begin>=end){
        return 0;
    }
    
    /*
    然后进行快速排序，参数中定义了time，每往下传递一层，time+1
    */

    int pivot=a[end];
    int i=begin,j=end-1;
    while(j!=i){
        while(a[i]<=pivot&&i<j){
            i++;
        }
        while(a[j]>=pivot&&i<j){
            j--;
        }
        if(i<j){
            int t=a[i];
            a[i]=a[j];
            a[j]=t;
        }
    }
    a[end]=a[i];
    a[i]=pivot;
    time++;

    /*
    构建q_block* 传入下一层递归中
    */

    q_block l[3];
    l[1].begin=begin;
    l[1].a=a;
    l[1].end=i-1;
    l[1].time=time;
    l[2].a=a;
    l[2].begin=i+1;
    l[2].end=end;
    l[2].time=time;

    /*
    当time等于2时，把下一层的函数分别放入不同的线程中，
    此时一共会放入四个线程中进行递归
    time不等于2时，正常进行递归
    */

    if(time==2){
        pthread_t y1,y2;
        pthread_create(&y1,NULL,sort_q,&l[1]);
        pthread_create(&y2,NULL,sort_q,&l[2]);
        pthread_join(y1,NULL);
        pthread_join(y2,NULL);
    }else{
        sort_q(&l[2]);
        sort_q(&l[1]);
    }
}

