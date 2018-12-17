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

uint isDir(char * fs, uint inum)
{
    struct dinode* inode;
    inode = getinode(fs, inum);
    return (inode->type==T_DIR);
}

// 1
void inodeType(char* fs, struct superblock * sb){
    struct dinode * inode;
    short type;
    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs,i);
/*
        cout << "inode number " << i << endl;
        cout << "inode type " << inode->type << endl;
        cout << "inode # links " << inode->nlink << endl;
        cout << "inode address " << inode->addrs << endl;
        cout << "inode address 1 " << inode->addrs[0]  << endl;
        cout << "=============================" << endl;
*/
        type = inode->type;
        //cout << "here" << inode->type <<endl;
        if (type < 0 || type >3)
        {
            cout << type << endl;
            cerr << "ERROR: bad inode." << endl;
            exit(1);
        }

    }

}
//2
void inUseInode(char* fs, struct superblock * sb)
{
    struct dinode * inode;
    uint * indirect;
    short type;
    uint daddr,iaddr,ninodes,dbStart, dbEnd;
    ninodes = sb->ninodes;
    cout << "dkfs" << sb->size << endl;
    dbStart = (sb->ninodes/IPB)+2 + (sb->size/BPB+1);
    cout << "start " << dbStart << endl;
    dbEnd = sb->size;
    cout << "end " << dbEnd << endl;
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
            cout << "this" << inode->size << endl;

            if (inode->size/BSIZE >= NDIRECT)
            {
                cout << "here" << i << endl;
                if (inode->addrs[NDIRECT] > 0)
                {

                    indirect = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                    for (int k = 0; k < NINDIRECT; k++)
                    {
                      cout << indirect[k] << endl;
                        if(indirect[k]==0)break;
                        if (indirect[k] < dbStart || indirect[k] > dbEnd)
                        {
                            cout << indirect[k] << endl;
                            cerr << "ERROR: bad indirect address in inode" << endl;
                            exit(1);
                        }
                    }
                }
            }
        }
    }
}

//3
void rootDir(char* fs, struct superblock * sb){
    struct dinode * root;
    char* name;
    uint addrs[DIRSIZ];
    int i=0;
    struct dirent * dir;
    root = getinode(fs, 1);
    //cout << ";adkf " << root<< endl;
    uint start = root->addrs[0];
    //cout << "Start " << start << endl;
    /*
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
        cerr << "ERROR: root directory does not exist" << endl;
        exit(1);
    }
}

//4
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
            //cout << "dir " << i << endl;
            j = 0;
            found = 0;
            dir = getdir(fs, inode->addrs[0], j);
            //cout << "fj " << dir->name << endl;
            while (j <= NDIRECT && found < 2)
            {
                //cout << "blah " << dir->name << endl;
                if (strcmp(dir->name,".")==0 && dir->inum==i)
                {
                    //cout << "self" << endl;
                    found++;
                    //cout << "found " << found << endl;
                }
                else if (strcmp(dir->name,"..")==0)
                {
                    //cout << "parent" << endl;
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
    //cout << "blocknum " << blocknum/32 << endl;
    word = (uint)(bitmapArray[(blocknum / 32)]); //gets word within BitMap
    //cout << "word " << word << endl;
    pos = blocknum % 32;
    //cout << "pos " << pos << endl;
    //shift = 31 - pos;
    word = word >>pos;
    //cout << "word1 " << word << endl;
    return (word & 0x1);
}

//5
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
                if (inode->addrs[j] > 0)
                {
                    alloc = isAllocated(fs, inode->addrs[j], ninodes);
                    if (alloc < 1)
                    {
                        cerr << "ERROR: address used by inode but marked free in bitmap." << endl;
                        exit(1);
                    }
                }
            }
            if (inode->addrs[NDIRECT] > 0 && inode->size/BSIZE>NDIRECT)
            {
                indirect = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                for (int k = 0; k < NINDIRECT; k++)
                {
                    alloc = isAllocated(fs, indirect[k], ninodes);
                    if (alloc < 1)
                    {
                        cerr << "ERROR: address used by inode but marked free in bitmap." << endl;
                        exit(1);
                    }
                }
            }
        }
    }
}

//6
void bitmapMarked(char *fs, struct superblock * sb)
{
    uint size,alloc,ninodes,addr,found,x;
    struct dinode * inode;
    short type;
    uint* indirect;
    size = sb->size;
    ninodes = (sb->ninodes/IPB)+3;
    for (int i = ninodes; i < size; i++)
    {
        cout << "no? " << i << endl;
        alloc = isAllocated(fs, i, sb->ninodes);
        if (alloc)
        {
            found = 0;
            for (int j = 0; j < ninodes; j++)
            {
                inode = getinode(fs, j);
                cout << "j " << j << " " << found<< endl;
                x=0;
                while (inode->addrs[x]>0 && x < NDIRECT) //check direct
                {
                    cout << inode->addrs[x] <<" "<<i <<endl;
                    cout << "found " <<found << endl;
                    if (inode->addrs[x]==i) found = 1;
                    x++;
                }
                if (found < 1 && inode->size>0)      //check indirect
                {
                    indirect = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                    x=0;
                    while (indirect[x]>0 && x< NINDIRECT)
                    {
                        if (indirect[x]==i) found = 1;
                        x++;
                    }
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

//7
void usedOnceDirect (char* fs, struct superblock *sb)
{
    struct dinode * inode;
    uint addr, notfound;
    short type;
    uint used[sb->size];
    for (int i = 0; i < sb->size; i++)
    {
        used[i] = 0;
    }
    //cout<<"usedOnceDirect" << endl;
    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs, i);
        if (inode->size > 0)
        {
            type = inode->type;
            if (type == T_DIR || type == T_FILE || type== T_DEV)
            {
                for (int j = 0; j < NDIRECT; j++)
                {
                    //cout << "========== " << j << " ===========" << endl;
                    if (inode->addrs[j]==0) break;
                    if (used[inode->addrs[j]]==1)
                    {
                        cerr << "ERROR: direct address used more than once." << endl;
                        exit(1);
                    }
                    used[inode->addrs[j]] = 1;
                }
            }
        }
    }
}

//8
void usedOnceIndirect(char* fs, struct superblock* sb)
{
    struct dinode * inode;
    uint addr, notfound;
    uint * indirect;
    short type;
    uint used[sb->size];
    for (int i = 0; i < sb->size; i++)
    {
        used[i] = 0;
    }
    //cout<< "usedOnceIndirect" << endl;
    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs, i);
        if (inode->size > 0)
        {
            type = inode->type;
            if (type == T_DIR || type == T_FILE || type== T_DEV)
            {
                indirect = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                for (int j = 0; j < NINDIRECT; j++)
                {
                    if (indirect[j]==0) break;
                    if (used[indirect[j]]==1)
                    {
                        cerr << "ERROR: indirect address used more than once." << endl;
                        exit(1);
                    }
                    used[indirect[j]] = 1;
                }
            }
        }
    }
}


//9
void inodeInDirectory(char * fs, struct superblock* sb)
{
    struct dinode * inode;
    struct dinode* inode2;
    struct dirent* dir;
    uint notfound,j,k;
    cout << sb->ninodes << endl;
    for (int i = 1; i < sb->ninodes; i++)
    {
        inode = getinode(fs, i);
        if (inode->type == 0) break;
        cout << "fdlks" << endl;
        notfound = 1;
        j = 1;
        while (notfound || j < sb->ninodes)
        {
            if (i != j)
            {
                inode2 = getinode(fs, j);
                if (inode2->type==T_DIR)
                {
                    //cout << "2" << endl;
                    k = 0;
                    dir = getdir(fs,inode2->addrs[0],k);
                    while (k < inode2->size/sizeof(struct dirent*)|| notfound) //??rihgt/?
                    {
                        //cout << "dir " << dir->name << endl;
                        if (dir->inum==i) notfound = 0;
                        k++;
                        dir = getdir(fs,j,k);
                    }
                    cout << "n " << notfound << endl;

                }
            }
                j++;

        }
        if (notfound)
        {
            cerr << "ERROR: inode marked use but not found in a directory." << endl;
            exit(1);
        }
    }
}

//10
void inodeinUse(char*fs, struct superblock * sb)
{
    struct dinode* inode;
    struct dinode* inode2;
    struct dirent * dir;
    uint* indirect;
    uint j,alloc;
    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs, i);
        if (isDir(fs, i))
        {
            j = 0;
            dir = getdir(fs, inode->addrs[0], j);
            while (dir->inum > 0)
            {
                inode2 = getinode(fs, dir->inum);
                if (inode2->type < 1 || inode->type > 3)
                {
                    cerr << "ERROR: inode referred to in directory but marked free." << endl;
                    exit(1);
                }

                j++;
                dir = getdir(fs, inode->addrs[0], j);
            }

            if (inode->size/sizeof(struct dirent*) > NDIRECT)
            {
                j = 0;
                indirect  = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                dir = getdir(fs, indirect[0], j);
                while (dir->inum > 0)
                {
                    inode2 = getinode(fs, dir->inum);
                    if (inode2->type < 1 || inode->type > 3)
                    {
                        cerr << "ERROR: inode referred to in directory but marked free." << endl;
                        exit(1);
                    }

                    j++;
                    dir = getdir(fs, indirect[0], j);
                }
            }

        }
    }
}



//11
void linkCount(char* fs, struct superblock * sb)
{
    struct dinode * inode;
    struct dinode * inode2;
    struct dirent * dir;
    uint links,k,looking;

    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs, i);
        if (inode->type == T_FILE || inode->type== T_DEV)
        {
            links=0;
            for (int j = 0; j < sb->ninodes; j++)
            {
                if (i != j)
                {
                    inode2 = getinode(fs, j);
                    if (inode2->type == T_DIR)
                    {
                        k = 0;
                        dir = getdir(fs, inode2->addrs[0],k);
                        while (dir->inum != 0 )
                        {
                            if (dir->inum == i) links++;
                            k++;
                            dir = getdir(fs, inode2->addrs[0],k);
                        }
                    }
                }
            }
            if (links != inode->nlink)
            {
                cout << "inode " << i << endl;
                cout << "links " << links << endl;
                cout << "inode nlinks " << inode->nlink << endl;
                cerr << "ERROR: bad reference count for file." << endl;
                exit(1);
            }
        }
    }
}



void noExtraLink(char * fs, struct superblock * sb)
{
    struct dinode * inode;
    struct dirent * dir;
    uint * indirect;
    uint j;
    uint seen[sb->ninodes];
    for (int i = 0; i < sb->ninodes; i++)
    {
        seen[i] = 0;
    }
    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs, i);
        if (isDir(fs, i))
        {
            j = 0;
            dir = getdir(fs, inode->addrs[0], j);
            while (dir->inum > 0 && j < inode->size/sizeof(struct dirent*))
            {
                if (strcmp(dir->name, "..")!=0 && strcmp(dir->name, ".")!=0)
                {
                    if (isDir(fs, dir->inum))
                    {
                        if (seen[dir->inum]==1)
                        {
                            cerr << "ERROR: directory appears more than once in file system." << endl;
                            exit(1);
                        }
                        seen[dir->inum] = 1;
                    }
                }
                j++;
                dir = getdir(fs, inode->addrs[0], j);
            }

            if (inode->size/sizeof(struct dirent*) > NDIRECT)
            {
                indirect = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                cout << "large " << endl;
                j = 0;
                dir = getdir(fs, indirect[0], j);
                while (dir->inum > 0 && j < inode->size/sizeof(struct dirent*))
                {
                    cout << "h " << endl;
                    if (strcmp(dir->name, "..")!=0 && strcmp(dir->name, ".")!=0)
                    {
                        cout << "i " << endl;
                        if (isDir(fs, dir->inum))
                        {
                            cout << "j " << endl;
                            if (seen[dir->inum]==1)
                            {
                                cerr << "ERROR: directory appears more than once in file system." << endl;
                                exit(1);
                            }
                            seen[dir->inum] = 1;
                        }
                    }
                    j++;
                    dir = getdir(fs, indirect[0], j);
                }
            }

        }
    }
}















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

    //inodeType(fs, sb); //1
    //inUseInode(fs,sb); //2
    //rootDir(fs, sb); //3
    //selfAndParent(fs,sb->ninodes); //4
    //inBitMap(fs, sb->ninodes); //5
    //bitmapMarked(fs,sb); //6
    //usedOnceDirect(fs, sb); //7
    //usedOnceIndirect(fs, sb); //8
    inodeinUse(fs,sb); //10
    //inodeInDirectory(fs, sb); //9
    //linkCount(fs, sb); //11
    //noExtraLink(fs, sb); //12
    if (munmap(fs,size)==-1) cout << "error" << endl;
    closed = close(fd);
    return 0;
}
