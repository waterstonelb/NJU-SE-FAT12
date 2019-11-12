#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int

unsigned char FAT_STORE[100000] = {0};
int root_entry_num = 512 * 19;
int data_num = 512 * 33;
int fat_num = 512;
unsigned char *FAT = FAT_STORE + 512;
unsigned char *Root_Entry = FAT_STORE + 512 * 19;
unsigned char *Data = FAT_STORE + 512 * 33;

typedef struct _FILE_HEADER FILE_HEADER;
typedef struct _FILE_HEADER *PFILE_HEADER;

struct _FILE_HEADER
{
    BYTE DIR_Name[11];
    BYTE DIR_Attr;
    BYTE Reserved[10];
    WORD DIR_WrtTime;
    WORD DIR_WrtDate;
    WORD DIR_FstClus;
    DWORD DIR_FileSize;
} __attribute__((packed)) _FILE_HEADER;

int input(char *inp, char *path, char *command, char *para);

void asm_print(const char*, int);

void my_print(unsigned char *p, int len, int color);

unsigned int fat_next(unsigned char *fat, unsigned int clus);

PFILE_HEADER find_path(unsigned char *root, char path[][11], int len);

int my_cat(PFILE_HEADER entry, unsigned char *fat, unsigned char *data);

int ls_l(char *path, int *res);

int my_ls(PFILE_HEADER root, char para, char *path);

int main()
{

    FILE *fat12 = fopen("ref.img", "r");
    fread(FAT_STORE, 1, 100000, fat12);
    while (1)
    {
        my_print("> ",2,0);
        char inp[100] = {0}, t;
        int len = 100;
        fgets(inp,len,stdin);

        if(strlen(inp) == 0 || !strcmp(inp, "exit\n")){
            return 0;
        }

        inp[strlen(inp)-1]=0;
        char path[100] = {0}, command[10] = {0}, para = 0;

        int RES_INPUT = input(inp, path, command, &para);
        if (RES_INPUT == 1)
        {
            my_print("Invalid Input!\n", 15, 0);
            continue;
        }
        else if (RES_INPUT == 2)
        {
            my_print("Paramter error!\n", 16, 0);
            continue;
        }
        char pp[20][11] = {0};
        char *token = strtok(path, "/");
        len = 0;
        while (token != NULL)
        {
            strcpy(pp[len++], token);
            token = strtok(NULL, "/");
        }
        PFILE_HEADER entry = find_path(Root_Entry, pp, len);
        char *p = strcmp(path,"/")==0 ? path + 1 : path;
        if(strcmp(command,"cat")==0){
            my_cat(entry, FAT, Data);
        }else if(strcmp(command,"ls")==0){
            my_ls(entry, para, p);
        }else{
            my_print("Invalid input!\n",14,0);
        }
    }

    return 0;
}

int input(char *inp, char *path, char *command, char *para)
{
    char *token = strtok(inp, " ");
    int count = 0,hot=0;
    while (token)
    {
        if (count == 0)
        {
            if (strcmp(token, "ls") == 0 || strcmp(token, "cat") == 0)
                strcpy(command, token);
            else
                return 1;
        }
        else if (*token == '-' && count != 0)
        {
            int i = 1;
            for (; *(token + i) == 'l'; i++);
            if (i == 1 || *(token + i) != 0 || strcmp(command, "ls") != 0)
                return 2;
            else
                *para = *(token + 1);
        }
        else{
            strcpy(path, token);hot++;}
        count++;
        token = strtok(NULL, " ");
    }
    return count == 0||hot!=1;
}

void my_print(unsigned char *p, int len, int color)
{
    char buf[100];
    for (int i = 0; i < len; i++)
    {
        if (color == 16)
            sprintf(buf, "\x1b[31m%c\x1b[0m", *(p + i));
        else
            sprintf(buf, "%c", *(p + i));
        asm_print(buf, strlen(buf));
    }
}
unsigned int fat_next(unsigned char *fat, unsigned int clus)
{
    int offset = (int)clus * 1.5;
    WORD *desc = (WORD *)(fat + offset);
    WORD res = *desc;
    res = clus % 2 == 0 ? res & 0x0fff : res >> 4;
    return (unsigned int)res;
}

PFILE_HEADER find_path(unsigned char *root, char path[][11], int len)
{
    unsigned char *t = root;
    PFILE_HEADER pFileHeader = (PFILE_HEADER)t;
    for (int n = 0; n < len; n++)
    {
        char *token = *(path + n);
        while (1)
        {
            if (*(BYTE *)pFileHeader == 0 || *(BYTE *)pFileHeader == 0xe5)
                return NULL;
            BYTE ttt = *(BYTE *)pFileHeader;
            char buffer[12] = {0};
            int i, len = strlen(token) - 1;
            for (i = 0; i < 11 && token[i] != '.'; i++)
                buffer[i] = token[i] == 0 ? ' ' : token[i];
            if (i != 11)
            {
                for (int j = 10; j >= i; j--)
                    buffer[j] = token[len] == '.' ? ' ' : token[len--];
            }
            char name[20] = {0};
            memcpy(name, pFileHeader->DIR_Name, 11);
            if (strcmp(buffer, name) == 0)
                break;
            ++pFileHeader;
        }
        if (pFileHeader->DIR_Attr == 16)
            pFileHeader = pFileHeader->DIR_FstClus == 0 ? (PFILE_HEADER)Root_Entry : (PFILE_HEADER)(Root_Entry + (pFileHeader->DIR_FstClus - 2 + 14) * 512);
        else if (n != len - 1)
            return NULL;
    }
    return pFileHeader;
}

int ls_l(char *path, int *res)
{
    char p[100] = {0};
    strcpy(p, path);
    char pp[20][11] = {0};
    char *token = strtok(p, "/");
    int len = 0;
    while (token != NULL)
    {
        strcpy(pp[len++], token);
        token = strtok(NULL, "/");
    }
    PFILE_HEADER entry = find_path(Root_Entry, pp, len);
    if (entry->DIR_Attr != 16)
    {
        res[0] = 2; //2表示文件
        res[1] = (int)entry->DIR_FileSize;
    }
    else
    {
        res[0] = 1;
        while (*(BYTE *)entry && *(BYTE *)entry != 0xe5)
        {
            if (entry->DIR_Attr == 16 && *(char *)entry != '.')
                res[1]++;
            else if (entry->DIR_Attr == 0)
                res[2]++;
            ++entry;
        }
    }
    return 0;
}

int my_cat(PFILE_HEADER entry, unsigned char *fat, unsigned char *data)
{
    if (entry == NULL || entry->DIR_Attr != 0)
    {
        my_print("Path is a directory or no such file\n", 36, 0);
        return 0;
    }
    int clus_nodes[100] = {0};
    int size = (int)entry->DIR_FileSize;
    clus_nodes[0] = (int)entry->DIR_FstClus;
    if (size > 512)
    {
        for (int i = 1;; i++)
        {
            int next = fat_next(fat, clus_nodes[i - 1]);
            if (next >= 255)
                break;
            clus_nodes[i] = next;
        }
    }
    for (int i = 0; clus_nodes[i] != 0; i++)
    {
        for (unsigned int j = 0; j < size && j < 512; j++)
        {
            char *c = data + j + (clus_nodes[i] - 2) * 512;
            my_print(c, 1, 0);
        }
        size -= 512;
    }
}
int my_ls(PFILE_HEADER root, char para, char *path)
{
    PFILE_HEADER entry = root;
    if (entry == NULL||entry->DIR_Attr!=16)
    {
        my_print("Path is not a directory or no such file\n", 40, 0);
        return 0;
    }
    char out[20][11] = {0};
    char p[20] = {0};
    strcpy(p, path);
    strcat(p, "/");
    my_print(p, strlen(p), 0);
    if (para == 'l')
    {
        int res[3] = {0};
        ls_l(path, res);
        char p1[10] = {0}, p2[10] = {0};
        sprintf(p1, "%d", res[1]);
        sprintf(p2, "%d", res[2]);
        my_print(" ", 1, 0);
        my_print(p1, strlen(p1), 0);
        if (res[0] == 1)
        {
            my_print(" ", 1, 0);
            my_print(p2, strlen(p2), 0);
        }
    }
    my_print(":\n", 2, 0);
    int n = 0;
    while (*(BYTE *)entry && *(BYTE *)entry != 0xe5)
    {
        char name[12] = {0};
        memcpy(name, entry->DIR_Name, 11);
        for (int i = 0; i < 12; i++)
            name[i] = name[i] != 32 ? name[i] : 0;
        if (name[8] != 0)
        {
            strcat(name, ".");
            strcat(name, name + 8);
        }
        if (entry->DIR_Attr == 16 && entry->DIR_Name[0] != 46)
            strcpy(out[n++], name);
        my_print(name, strlen(name), (int)entry->DIR_Attr);
        my_print(" ",1,0);
        if (para == 'l')
        {
            if (*(char *)entry != '.')
            {
                char tpath[100] = {0};
                int res[3] = {0};
                char p1[10] = {0}, p2[10] = {0};
                strcpy(tpath, path);
                strcat(tpath, "/");
                strcat(tpath, name);
                ls_l(tpath, res);
                sprintf(p1, "%d", res[1]);
                sprintf(p2, "%d", res[2]);
                my_print(" ", 1, 0);
                my_print(p1, strlen(p1), 0);
                if (res[0] == 1)
                {
                    my_print(" ", 1, 0);
                    my_print(p2, strlen(p2), 0);
                }
            }
            my_print("\n", 1, 0);
        }

        ++entry;
    }
    my_print("\n", 1, 0);
    for (int i = 0; i < n; i++)
    {
        char nnn[1][11] = {0};
        char ppp[100] = {0};
        strcpy(nnn[0], out[i]);
        strcpy(ppp, p);
        strcat(ppp, out[i]);
        PFILE_HEADER next = find_path((unsigned char *)root, nnn, 1);
        my_ls(next, para, ppp);
    }
}