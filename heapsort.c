#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

#define MAXREC PAGE_SIZE/RECORD_SIZE

int heapsize=0;
int totalpagenum;
int totalrecordnum;

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓸 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉, 페이지 단위로 읽거나 써야 합니다.

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	memset(pagebuf, 0xFF, PAGE_SIZE);
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);

}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);

}

//
// 주어진 레코드 파일에서 레코드를 읽어 heap을 만들어 나간다. Heap은 배열을 이용하여 저장되며, 
// heap의 생성은 Chap9에서 제시한 알고리즘을 따른다. 레코드를 읽을 때 페이지 단위를 사용한다는 것에 주의해야 한다.
//
void buildHeap(FILE *inputfp, char **heaparray)
{
	char pagebuf[PAGE_SIZE];
	char recordbuf[RECORD_SIZE];
	char parentbuf[RECORD_SIZE];
	char *ptr;
	long long sn;
	long long psn;
	char tmp[RECORD_SIZE];
	int i,j,k;

	readPage(inputfp, pagebuf, 0);
	totalpagenum = pagebuf[0];
	totalrecordnum = pagebuf[4];

	for(i=1; i<totalpagenum; i++){
		readPage(inputfp, pagebuf, i);
		for(j=0; j<MAXREC; j++){
			if(pagebuf[j*RECORD_SIZE] == (char)0xFF)
				break;
			strncpy(recordbuf, pagebuf+(j*RECORD_SIZE), RECORD_SIZE);
			sn = atoll(recordbuf);
			
			k = ++heapsize;
			heaparray[k] = (char *)malloc(RECORD_SIZE*(sizeof(char)));
			while(k!=1){
				psn = atoll(heaparray[k/2]);
				if(sn > psn)
					break;
				strcpy(heaparray[k], heaparray[k/2]);
				k/=2;
			}
			strcpy(heaparray[k], recordbuf);
		}
	}

	
}

//
// 완성한 heap을 이용하여 주민번호를 기준으로 오름차순으로 레코드를 정렬하여 새로운 레코드 파일에 저장한다.
// Heap을 이용한 정렬은 Chap9에서 제시한 알고리즘을 이용한다.
// 레코드를 순서대로 저장할 때도 페이지 단위를 사용한다.
//
void makeSortedFile(FILE *outputfp, char **heaparray)
{
	char pagebuf[PAGE_SIZE];
	char recordbuf[RECORD_SIZE];
	char lastheap[RECORD_SIZE];
	int parent = 1;
	int child = 2;
	int i,j;
	
	for(i=1; i<totalpagenum; i++){
		memset(pagebuf, 0xFF, PAGE_SIZE);
		for(j=0; j<MAXREC; j++){
			parent = 1;
			child = 2;
			strcpy(pagebuf+(j*RECORD_SIZE), heaparray[parent]);
			if(((i-1)*MAXREC + (j+1)) > totalrecordnum)
				break;
			strcpy(lastheap, heaparray[heapsize]);
			free(heaparray[heapsize--]);
			while(child <= heapsize){
				if(child < heapsize && atoll(heaparray[child]) > atoll(heaparray[child+1]))
					child++;
	
				if(atoll(lastheap) <= atoll(heaparray[child]))
					break;

				strcpy(heaparray[parent], heaparray[child]);
				parent = child;
				child *= 2;
			}

			strcpy(heaparray[parent], lastheap);
		}
		writePage(outputfp, pagebuf, i);
	}

}

int main(int argc, char *argv[])
{
	FILE *inputfp;	// 입력 레코드 파일의 파일 포인터
	FILE *outputfp;	// 정렬된 레코드 파일의 파일 포인터
	char *heaparray[RECORD_SIZE];
	char pagebuf[PAGE_SIZE];

	if(argc != 4){
		fprintf(stderr, "usage : %s s <input record file name> <output record file name>\n", argv[0]);
		exit(1);
	}

	if((inputfp = fopen(argv[2], "r")) == NULL){
		fprintf(stderr, "fopen error for %s\n", argv[2]);
		exit(1);
	}

	outputfp = fopen(argv[3], "w+");

	buildHeap(inputfp, heaparray);

	readPage(inputfp, pagebuf, 0);
	writePage(outputfp, pagebuf, 0);
	makeSortedFile(outputfp, heaparray);

	fclose(inputfp);
	fclose(outputfp);
	return 1;
}
