#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <iterator>
#include <map>

//#include "main.h"

#define stat xv6_stat
#include "../include/stat.h"
#include "../include/types.h"
#include "../include/fs.h"
#undef stat
using namespace std;


struct dinode * getinode(char* fs, uint inum)
{
    struct dinode * inode;
    inode = (struct dinode *)(fs+(BSIZE*2)+(sizeof(struct dinode)*inum));
    return inode;
}

struct dirent * getdir(char* fs, uint block, uint ent)
{
    struct dirent* dir;
    dir = (struct dirent *)(fs+(block*BSIZE)+sizeof(struct dirent)*ent);
    return dir;

}

void inodeType(char* fs, uint ninodes){
    struct dinode * inode;
    short type;
    for (int i = 1; i < ninodes; i++)
    {
        inode = getinode(fs,i);
        cout << "inode "<< inode << endl;
        cout << "inode number " << i << endl;
        cout << "inode type " << inode->type << endl;
        cout << "inode # links " << inode->nlink << endl;
        cout << "inode address " << inode->addrs << endl;
        cout << "inode address 1 " << inode->addrs[0]  << endl;
        cout << "=============================" << endl;
        type = inode->type;
        //cout << "here" << inode->type <<endl;
        if (type != T_DIR && type != T_FILE && type !=T_DEV && type != 0)
        {
            cerr << "ERROR: bad inode." << endl;
            exit(1);
        }

    }

}

void inUseInode(char* fs, struct superblock * sb)
{
    struct dinode * inode;
    uint * indirect;
    short type;
    uint daddr,iaddr,ninodes;
    ninodes = sb->ninodes;
    for (int i = 0; i < ninodes; i++)
    {
        inode = getinode(fs, i);
        type = inode->type;
        if (type == T_DIR || type == T_FILE || type== T_DEV) //valid
        {
            for (int j = 0; j < NDIRECT; j++)
            {
                if (inode->addrs[j] >= sb->size)
                {
                    cerr << "ERROR: bad direct address in inode" << endl;
                    exit(1);
                }
            }

            if (inode->addrs[NDIRECT] > 0)
            {
                indirect = (uint *)(fs+(BSIZE*2)+(BSIZE*inode->addrs[NDIRECT]));
                for (int k = 0; k < NINDIRECT; k++)
                {
                    if (indirect[k] > sb->size)
                    {
                        cerr << "ERROR: bad direct address in inode" << endl;
                        exit(1);
                    }
                }
            }
        }
    }
}


void rootDir(char* fs, struct superblock * sb){
    struct dinode * root;
    char* name;
    uint addrs[DIRSIZ];
    int i=0;
    struct dirent * dir;
    root = getinode(fs, 1);
    cout << ";adkf " << root<< endl;
    uint start = root->addrs[0];
    cout << "Start " << start << endl;/*
    for (uint x = 0; x<sb->nblocks; x++)
    {
        dir = (struct dirent*)(fs+(start*BSIZE)+sizeof(struct dirent)*x);
        name = dir->name;
        cout << "block number " << start*BSIZE << endl;
        cout << "block number+x " << (void*)(fs+start*BSIZE+sizeof(struct dirent)*x) << endl;
        cout << "dirent " <<  dir<< endl;
        cout << "dir name " << name << endl;
        cout << "dir inum " << dir->inum << endl;
        cout << "======================" << endl;
    }*/

    if (root->type == T_DIR)
    {
        //addrs = root->addrs;
        dir = getdir(fs, start, i);
        //(struct dirent*)(fs+(start*BSIZE)+sizeof(struct dirent)*i);
        while (strcmp(dir->name, "..")!= 0 && i<=NDIRECT)
        {
            i++;
            dir = getdir(fs, start,i);
        }
        if (dir->inum != 1)
        {
            cerr << "ERROR: root directory does not exist" << endl;
            exit(1);
        }
    }
    else
    {
        cout << "here" << endl;
        cerr << "ERROR: root directory does not exist" << endl;
        exit(1);
    }
}

void selfAndParent(char* fs, uint ninodes)
{
    struct dinode * inode;
    char name;
    short type;
    struct dirent * dir;
    int j=0;
    int x;
    int found;
    for (int i = 0; i < ninodes; i++)
    {
        inode = getinode(fs, i);
        type = inode->type;
        if (type == T_DIR)
        {
            cout << "dir " << i << endl;
            j = 0;
            found = 0;
            dir = getdir(fs, inode->addrs[0], j);
            cout << "fj " << dir->name << endl;
            while (j <= NDIRECT && found < 2)
            {
                cout << "blah " << dir->name << endl;
                if (strcmp(dir->name,".")==0 && dir->inum==i)
                {
                    cout << "self" << endl;
                    found++;
                    cout << "found " << found << endl;
                }
                else if (strcmp(dir->name,"..")==0)
                {
                    cout << "parent" << endl;
                    found++;
                }
                j++;
                dir = (struct dirent *)(fs+inode->addrs[0]*BSIZE+sizeof(struct dirent)*j);
            }
            if (found < 2)
            {
                cerr << "ERROR: directory not properly formatted" << endl;
                exit(1);
            }
        }

    }

}

uint isAllocated(char* fs, int blocknum, uint ninodes)
{
    uint* bitmapArray;
    uint bit,word,pos, shift;
    bitmapArray = (uint *) (fs + (BBLOCK(0,ninodes)*BSIZE));
    cout << "blocknum " << blocknum/32 << endl;
    word = (uint)(bitmapArray[(blocknum / 32)]); //gets word within BitMap
    cout << "word " << word << endl;
    pos = blocknum % 32;
    cout << "pos " << pos << endl;
    shift = 31 - pos;
    word = word >>shift;
    cout << "word1 " << word << endl;
    return (word & 0x1);
}

void inBitMap(char * fs, uint ninodes)
{
    struct dinode * inode;
    uint * indirect;
    short type;
    uint alloc;
    for (int i = 0; i < ninodes; i++)
    {
        inode = getinode(fs, i);
        type = inode->type;
        if (type == T_DIR || type == T_FILE || type== T_DEV)
        {
            for (int j = 0; j < NDIRECT; j++)
            {
                cout << "=============== " << j <<" ==============="  << endl;
                cout << "kkkkkkk " << inode->addrs[j] << endl;
                if (inode->addrs[j] > 0)
                {
                    alloc = isAllocated(fs, inode->addrs[j], ninodes);
                    if (alloc < 1)
                    {
                        cout << "here " << inode->addrs[j] << endl;
                        cerr << "ERROR: address used by inode but marked free in bitmap." << endl;
                        exit(1);
                    }
                }
            }
            if (inode->addrs[NDIRECT] > 0)
            {
                indirect = (uint *)(fs+(BSIZE*2)+(BSIZE*inode->addrs[NDIRECT]));
                for (int k = 0; k < NINDIRECT; k++)
                {
                    alloc = isAllocated(fs, indirect[k], ninodes);
                    if (alloc < 1)
                    {
                        cout << "here2" << endl;
                        cerr << "ERROR: address used by inode but marked free in bitmap." << endl;
                        exit(1);
                    }
                }
            }


        }
    }
}

void bitmapMarked(char *fs, struct superblock * sb)
{
    uint size,alloc,ninodes,addr,found,x;
    struct dinode * inode;
    short type;
    size = sb->size;
    ninodes = (sb->ninodes/IPB)+2;
    for (int i = ninodes; i < size; i++)
    {
        alloc = isAllocated(fs, i, ninodes);
        if (alloc)
        {
            for (int j = 1; j < ninodes; j++)
            {
                inode = getinode(fs, j);
                while (inode->addrs[x]>0)
                {
                    if (inode->addrs[x]==i) found = 1;
                    x++;

                }
            }
            if (found < 1)
            {
                cerr << "ERROR: bitmap marks block in use but it is not in use." << endl;
                exit(1);
            }
        }
    }
}
/*
void usedOnce (char* fs, struct superblock *sb)
{
    struct dinode * inode;
    iterator found;
    uint addr;
    short type;
    map<uint,uint> used;
    for (int i = 1; i < ninodes; i++)
    {
        inode = (struct dinode *)(fs+(BSIZE*2)+(sizeof(struct dinode)*i));
        type = inode->type;
        //cout << "here" << inode->type <<endl;
        if (type == T_DIR || type == T_FILE || type== T_DEV)
        {
            addr = inode->addrs[0];
            found = used.find(addr);
            if (found != used.end())
            {
                cerr << "ERROR: direct address used more than once." << endl;
                exit(1);
            }
            else
            {
                used[addr] = 1;
            }
        }

    }
}
*/
int main (int argc,char *argv[])
{
    struct superblock *sb;
    char * fs;
    int fd;
    int r;
    int closed;
	struct stat st;
    size_t size;
    struct dinode * inodes;
    struct dirent * dataStart;
    uint numiNodeBlocks, numBitMapBlocks;
    cout << argv[0] << endl;
    cout << argv[1] << endl;
    fd = open(argv[1],O_RDWR);
    if (fd == -1)
    {
        cout << "bad file" << endl; //fix error message
        return -1;//probably don't do this
    }
    r = fstat(fd, &st);
    if (r!=0) cout << "bad" << endl;
    size = st.st_size;
    cout << "size " << size << endl;
    fs = (char *) (mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0));
    cout << "fs " <<(void*)fs<< endl;
    if (fs==MAP_FAILED)cout << "baddd "<<endl;
    sb = (struct superblock *)(fs + BSIZE);
    numiNodeBlocks = sb->ninodes /IPB;
    numBitMapBlocks = sb->size/BPB+1;
    cout << "size " << numiNodeBlocks << endl;
    cout << "inodes " << sb->size << endl;
    cout << "blocks " << sb->nblocks << endl;
    cout << "types " << T_DIR <<" "<<T_FILE <<" "<<T_DEV << endl;
    //inodeType(fs, sb->ninodes);
    cout << "finished inode type" << endl;
    //rootDir(fs, sb);
    cout << "finished rootdir" << endl;
    //inBitMap(fs, sb->ninodes);
    //cout << "done " << isAllocated(fs,31,sb->ninodes) << endl;
    inUseInode(fs,sb);
    //selfAndParent(fs,sb->ninodes);
    //bitmapMarked(fs,sb);
    if (munmap(fs,size)==-1) cout << "error" << endl;
    closed = close(fd);
    return 0;
}
