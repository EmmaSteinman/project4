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


#define stat xv6_stat
#include "../include/stat.h"
#include "../include/types.h"
#include "../include/fs.h"
#undef stat
using namespace std;

//========================================================
// getinode
// - Returns the inum-th inode
//========================================================
struct dinode * getinode(char* fs, uint inum)
{
    struct dinode * inode;
    inode = (struct dinode *)(fs+(BSIZE*2)+(sizeof(struct dinode)*inum));
    return inode;
}
//========================================================
// getdir
// - Returns directory entry at given block and offset
//========================================================
struct dirent * getdir(char* fs, uint block, uint ent)
{
    struct dirent* dir;
    dir = (struct dirent *)(fs+(block*BSIZE)+sizeof(struct dirent)*ent);
    return dir;

}
//========================================================
// isAllocated
// - Checks if a given block number is allocated in the
// bitmap
//========================================================
uint isAllocated(char* fs, int blocknum, uint ninodes)
{
    uint* bitmapArray;
    uint bit,word,pos, shift;
    bitmapArray = (uint *) (fs + (BBLOCK(0,ninodes)*BSIZE));
    word = (uint)(bitmapArray[(blocknum / 32)]); //gets word within BitMap
    pos = blocknum % 32;
    word = word >>pos;
    return (word & 0x1);
}
//========================================================
// isDir
// - Checks if a given inode is a directory
//========================================================
uint isDir(char * fs, uint inum)
{
    struct dinode* inode;
    inode = getinode(fs, inum);
    return (inode->type==T_DIR);
}

//========================================================
// inodeType
// - 1
// - Determines if any inodes have an invalid type
//========================================================
void inodeType(char* fs, struct superblock * sb){
    struct dinode * inode;
    short type;

    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs,i);
        type = inode->type;
        if (type < 0 || type >3)
        {
            cerr << "ERROR: bad inode." << endl;
            exit(1);
        }
    }
}
//========================================================
// inUseInode
// - 2
// - Checks that each direct and indirect address points
// to a valid datablock
//========================================================
void inUseInode(char* fs, struct superblock * sb)
{
    struct dinode * inode;
    uint * indirect;
    short type;
    uint daddr,iaddr,ninodes,dbStart, dbEnd;
    ninodes = sb->ninodes;
    dbStart = (sb->ninodes/IPB)+2 + (sb->size/BPB+1);
    dbEnd = sb->size;

    for (int i = 0; i < ninodes; i++)
    {
        inode = getinode(fs, i);
        type = inode->type;
        if (type == T_DIR || type == T_FILE || type== T_DEV) //valid
        {
            for (int j = 0; j < NDIRECT; j++) //checking direct addresses
            {
                if (inode->addrs[j]==0) break;
                if (inode->addrs[j] < dbStart || inode->addrs[j]>dbEnd)
                {
                    cerr << "ERROR: bad direct address in inode" << endl;
                    exit(1);
                }
            }
            if (inode->size/BSIZE >= NDIRECT) //checking indirect addresses if necessary
            {
                if (inode->addrs[NDIRECT] < dbStart || inode->addrs[NDIRECT] > dbEnd)
                {
                    cerr << "ERROR: bad indirect address in inode" << endl;
                    exit(1);
                }

                indirect =  (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                for (int k = 0; k < NINDIRECT; k++)
                {

                    if (indirect[k]==0) break;
                    if (indirect[k] < dbStart || indirect[k] > dbEnd)
                    {
                        cerr << "ERROR: bad indirect address in inode" << endl;
                        exit(1);
                    }
                }
            }
        }
    }
}

//========================================================
// rootDir
// - 3
// - Checks that the root directory exists, the inode
// number is 1, and its parent is itself
//========================================================
void rootDir(char* fs, struct superblock * sb)
{
    struct dinode * root;
    char* name;
    uint addrs[DIRSIZ];
    int i=0;
    struct dirent * dir;
    root = getinode(fs, 1);
    uint start = root->addrs[0];

    if (root->type == T_DIR)
    {
        dir = getdir(fs, start, i);
        while (strcmp(dir->name, "..")!= 0 && i<=NDIRECT) //until you find ..
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

//========================================================
// selfAndParent
// - 4
// - Checks that each directory contains . and .. entries
// and that . points to itself
//========================================================
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
            j = 0;
            found = 0;
            dir = getdir(fs, inode->addrs[0], j);
            while (j <= NDIRECT && found < 2)
            {
                if (strcmp(dir->name,".")==0 && dir->inum==i) found++;
                else if (strcmp(dir->name,"..")==0) found++;
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

//========================================================
// inBitMap
// - 5
// - Validates that in-use inodes are marked correctly
//  in the bitmap
//========================================================
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
        if (type == T_DIR || type == T_FILE || type== T_DEV) //valid
        {
            for (int j = 0; j < NDIRECT; j++) //check direct addresses
            {
                if (inode->addrs[j] > 0)
                {
                    alloc = isAllocated(fs, inode->addrs[j], ninodes); //check if allocated
                    if (alloc < 1)
                    {
                        cerr << "ERROR: address used by inode but marked free in bitmap." << endl;
                        exit(1);
                    }
                }
            }
            if (inode->addrs[NDIRECT] > 0 && inode->size/BSIZE>NDIRECT) //check indirect addresses if necessary
            {
                indirect = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                for (int k = 0; k < NINDIRECT; k++)
                {
                    alloc = isAllocated(fs, indirect[k], ninodes); //check if allocated
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

//========================================================
// bitmapMarked
// - 6
// - Checks that each bit marked in use is actually used
//========================================================
void bitmapMarked(char *fs, struct superblock * sb)
{
    uint size,alloc,bitmapStart,addr,found,x,bitmap;
    struct dinode * inode;
    short type;
    uint* indirect;
    size = sb->size;
    bitmapStart = (sb->ninodes/IPB)+2;
    bitmap = sb->size /BPB+2;
    for (int i = bitmapStart+bitmap; i < size; i++) //boot,super,inode,and bitmap blocks always allocated
    {
        alloc = isAllocated(fs, i, sb->ninodes);
        if (alloc)
        {
            found = 0;
            for (int j = 0; j < sb->ninodes; j++) //search through inode addrs to find block
            {
                inode = getinode(fs, j);
                if (inode->type > 0 && inode->type <=3)
                {
                    x = 0;
                    while (x < NDIRECT) //check direct
                    {
                        if (inode->addrs[x]==i) found++;
                        x++;
                    }
                    if (inode->addrs[NDIRECT]==i)found++; //check indirect
                    if (inode->size/BSIZE >= NDIRECT)
                    {
                        indirect = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                        x=0;
                        while (indirect[x]>0)
                        {
                            if (indirect[x]==i) found++;
                            x++;
                        }
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

//========================================================
// usedOnceDirect
// - 7
// - Checks that direct blocks are only used once
//========================================================
void usedOnceDirect (char* fs, struct superblock *sb)
{
    struct dinode * inode;
    uint addr, notfound;
    short type;
    uint used[sb->size];

    for (int k = 0; k < sb->size; k++)
    {
        used[k] = 0;
    }

    for (int i = 0; i < sb->ninodes; i++) //check direct blocks of all inodes
    {
        inode = getinode(fs, i);
        if (inode->size > 0)
        {
            type = inode->type;
            if (type == T_DIR || type == T_FILE || type== T_DEV)
            {
                for (int j = 0; j < NDIRECT; j++)
                {
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

//========================================================
// usedOnceIndirect
// - 8
// - Checks that indirect blocks are only used once
//========================================================
void usedOnceIndirect(char* fs, struct superblock* sb)
{
    struct dinode * inode;
    uint addr, notfound;
    uint * indirect;
    short type;
    uint used[sb->size];

    for (int k = 0; k < sb->size; k++)
    {
        used[k] = 0;
    }
    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs, i);
        if (inode->size > 0)
        {
            type = inode->type;
            if (type == T_DIR || type == T_FILE || type== T_DEV)
            {
                indirect = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT])); //check indirect addresses
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

//========================================================
// inodeInDirectory
// - 9
// - Checks that inodes marked in-use are in a directory
//========================================================
void inodeInDirectory(char*fs, struct superblock * sb)
{
    struct dinode* inode;
    struct dinode* inode2;
    struct dirent * dir;
    uint k, ref;
    uint* indirect;

    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs, i);
        ref = 0;
        if (inode->type > 0) //marked in use
        {
            for (int j = 0; j < sb->ninodes; j++) //look through directories of other inodes
            {
                if (i != j)
                {
                    inode2 = getinode(fs, j);
                    if (isDir(fs, j))
                    {
                        k = 0;
                        dir = getdir(fs, inode2->addrs[0],k);
                        while (dir->inum > 0) //through dirents
                        {
                            if (dir->inum==i) ref++; //referenced
                            k++;
                            dir = getdir(fs, inode2->addrs[0],k);
                        }
                    }
                }
                if (inode2->size/sizeof(struct dirent*) > NDIRECT) //check indirects
                {
                    indirect = (uint *)(fs+(BSIZE*inode->addrs[NDIRECT]));
                    k = 0;
                    dir = getdir(fs, indirect[0], k);
                    while (dir->inum > 0)
                    {
                        if (dir->inum==i) ref++;
                        k++;
                        dir = getdir(fs, indirect[0],k);
                    }
                }
            }
            if (ref < 1)
            {
                cerr << "ERROR: inode marked use but not found in a directory." << endl;
                exit(1);
            }
        }
    }
}


//========================================================
// inodeinUse
// - 10
// - Checks that inodes referenced in a directory are
// marked in use
//========================================================
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
            while (dir->inum > 0)                   //check directory entries
            {
                inode2 = getinode(fs, dir->inum);
                if (inode2->type < 1 || inode->type > 3) //invalid or not in use
                {
                    cerr << "ERROR: inode referred to in directory but marked free." << endl;
                    exit(1);
                }
                j++;
                dir = getdir(fs, inode->addrs[0], j);
            }

            if (inode->size/sizeof(struct dirent*) > NDIRECT) //check indirect blocks for directories
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

//========================================================
// linkCount
// - 11
// -Checks the number of links for regular files match
// the number of times it is referenced
//========================================================
void linkCount(char* fs, struct superblock * sb)
{
    struct dinode * inode;
    struct dinode * inode2;
    struct dirent * dir;
    uint links,k,looking;

    for (int i = 0; i < sb->ninodes; i++)
    {
        inode = getinode(fs, i);
        if (inode->type == T_FILE || inode->type == T_DEV)
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
                cerr << "ERROR: bad reference count for file." << endl;
                exit(1);
            }
        }
    }
}

//========================================================
// noExtraLink
// - 12
// - Checks that directories are only referenced once
//========================================================
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
                j = 0;
                dir = getdir(fs, indirect[0], j);
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
    fd = open(argv[1],O_RDWR);
    if (fd == -1)
    {
        cerr << "image not found" << endl;
        exit(1);
    }
    r = fstat(fd, &st);
    size = st.st_size;
    fs = (char *) (mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0));
    sb = (struct superblock *)(fs + BSIZE);
    inodeType(fs, sb); //1
    inUseInode(fs,sb); //2
    rootDir(fs, sb); //3
    selfAndParent(fs,sb->ninodes); //4
    inBitMap(fs, sb->ninodes); //5
    bitmapMarked(fs,sb); //6
    usedOnceDirect(fs, sb); //7
    usedOnceIndirect(fs, sb); //8
    inodeinUse(fs,sb); //10
    inodeInDirectory(fs, sb); //9
    linkCount(fs, sb); //11
    noExtraLink(fs, sb); //12
    if (munmap(fs,size)==-1) cout << "error" << endl;
    closed = close(fd);
    return 0;
}
