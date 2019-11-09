#include <iostream>
#include <string>
#include <string.h>
#include <vector>
 
using namespace std;

typedef unsigned char u8;	//1字节
typedef unsigned short u16;	//2字节
typedef unsigned int u32;	//4字节
 
 
int  BytsPerSec;	//每扇区字节数
int  SecPerClus;	//每簇扇区数
int  RsvdSecCnt;	//Boot记录占用的扇区数
int  NumFATs;	//FAT表个数
int  RootEntCnt;	//根目录最大文件数
int  FATSz;	//FAT扇区数
 
 
#pragma pack (1) /*指定按1字节对齐*/
 
//偏移11个字节
struct BPB {	
	u16  BPB_BytsPerSec;	//每扇区字节数
	u8   BPB_SecPerClus;	//每簇扇区数
	u16  BPB_RsvdSecCnt;	//Boot记录占用的扇区数
	u8   BPB_NumFATs;	//FAT表个数
	u16  BPB_RootEntCnt;	//根目录最大文件数
	u16  BPB_TotSec16;
	u8   BPB_Media;
	u16  BPB_FATSz16;	//FAT扇区数
	u16  BPB_SecPerTrk;
	u16  BPB_NumHeads;
	u32  BPB_HiddSec;
	u32  BPB_TotSec32;	//如果BPB_FATSz16为0，该值为FAT扇区数
};
//BPB至此结束，长度25字节
 
//根目录条目
struct RootEntry {	
	char DIR_Name[11];
	u8   DIR_Attr;		//文件属性
	char reserved[10];
	u16  DIR_WrtTime;
	u16  DIR_WrtDate;
	u16  DIR_FstClus;	//开始簇号
	u32  DIR_FileSize;
};
//根目录条目结束，32字节
 
#pragma pack () /*取消指定对齐，恢复缺省对齐*/

void cat_fat12(string path);
void ls_fat12(string path);
void fillBPB(FILE* fat12 , struct BPB* bpb_ptr);

int main(){
	FILE* fat12;
	fat12 = fopen("a.img","rb");
	struct BPB bpb;
	struct BPB* bpb_ptr = &bpb;
	fillBPB(fat12,bpb_ptr);
	//初始化各个全局变量
	BytsPerSec = bpb_ptr->BPB_BytsPerSec;
	SecPerClus = bpb_ptr->BPB_SecPerClus;
	RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
	NumFATs = bpb_ptr->BPB_NumFATs;
	RootEntCnt = bpb_ptr->BPB_RootEntCnt;
	if (bpb_ptr->BPB_FATSz16 != 0) {
		FATSz = bpb_ptr->BPB_FATSz16;
	} else {
		FATSz = bpb_ptr->BPB_TotSec32;
	}
 
	struct RootEntry rootEntry;
	struct RootEntry* rootEntry_ptr = &rootEntry;

	//cout<<BytsPerSec<<" "<<SecPerClus<<" "<<RsvdSecCnt<<" "<<NumFATs<<" "<<RootEntCnt<<" "<<FATSz<<endl;
	while(1){
		char inp[100]={0};
		string command,path,para;
 		cout<<">";
		cin.getline(inp,100);
		char *t=inp;
		char *token=strtok(t," ");
		command=token;
		while(token!=NULL){
			token=strtok(NULL," ");
			if(token!=NULL){
				if(token[0]=='-')
					para=token+1;
				else
					path=token;
			}
		}
		if(command=="quit"){
			return 0;
		}else if(command=="cat"){
			cat_fat12(path);
		}else if(command=="ls"){
			ls_fat12(path);
		}else{
			cout<<"非法命令"<<endl;
		}
	}
	fclose(fat12);
	return 0;
}

void cat_fat12(string path){
	cout<<path<<endl;
}
void ls_fat12(string path){
	cout<<path<<endl;
}
void fillBPB(FILE* fat12 , struct BPB* bpb_ptr) {
	int check;
 
	//BPB从偏移11个字节处开始
	check = fseek(fat12,11,SEEK_SET);
	if (check == -1) 
		printf("fseek in fillBPB failed!");
 
	//BPB长度为25字节
	check = fread(bpb_ptr,1,25,fat12);
	if (check != 25)
		printf("fread in fillBPB failed!");
}