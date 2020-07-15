#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"
//�ʿ��� ��� ��� ���ϰ� �Լ��� �߰��� �� ����

#define MAXREC PAGE_SIZE/RECORD_SIZE

int heapsize=0;
int totalpagenum;
int totalrecordnum;

// ���� ������� �����ϴ� ����� ���� �ٸ� �� ������ �ణ�� ������ �Ӵϴ�.
// ���ڵ� ������ ������ ������ ���� �����Ǳ� ������ ����� ���α׷����� ���ڵ� ���Ϸκ��� �����͸� �а� �� ����
// ������ ������ ����մϴ�. ���� �Ʒ��� �� �Լ��� �ʿ��մϴ�.
// 1. readPage(): �־��� ������ ��ȣ�� ������ �����͸� ���α׷� ������ �о�ͼ� pagebuf�� �����Ѵ�
// 2. writePage(): ���α׷� ���� pagebuf�� �����͸� �־��� ������ ��ȣ�� �����Ѵ�
// ���ڵ� ���Ͽ��� ������ ���ڵ带 �аų� ���ο� ���ڵ带 �� ����
// ��� I/O�� ���� �� �Լ��� ���� ȣ���ؾ� �մϴ�. ��, ������ ������ �аų� ��� �մϴ�.

//
// ������ ��ȣ�� �ش��ϴ� �������� �־��� ������ ���ۿ� �о �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	memset(pagebuf, 0xFF, PAGE_SIZE);
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);

}

//
// ������ ������ �����͸� �־��� ������ ��ȣ�� �ش��ϴ� ��ġ�� �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);

}

//
// �־��� ���ڵ� ���Ͽ��� ���ڵ带 �о� heap�� ����� ������. Heap�� �迭�� �̿��Ͽ� ����Ǹ�, 
// heap�� ������ Chap9���� ������ �˰����� ������. ���ڵ带 ���� �� ������ ������ ����Ѵٴ� �Ϳ� �����ؾ� �Ѵ�.
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
// �ϼ��� heap�� �̿��Ͽ� �ֹι�ȣ�� �������� ������������ ���ڵ带 �����Ͽ� ���ο� ���ڵ� ���Ͽ� �����Ѵ�.
// Heap�� �̿��� ������ Chap9���� ������ �˰����� �̿��Ѵ�.
// ���ڵ带 ������� ������ ���� ������ ������ ����Ѵ�.
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
	FILE *inputfp;	// �Է� ���ڵ� ������ ���� ������
	FILE *outputfp;	// ���ĵ� ���ڵ� ������ ���� ������
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
