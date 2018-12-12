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

//#include "main.h"

#define stat xv6_stat
#include "../include/stat.h"
#include "../include/types.h"
#include "../include/fs.h"
#undef stat
using namespace std;


void inodeType(char* fs, uint ninodes){
    struct dinode * inode;
    short type;
    for (int i = 0; i < ninodes; i++)
    {
        inode = (struct dinode *)(fs+(BSIZE*2)+(sizeof(dinode)*i));
        type = inode->type;
        //cout << "here" << inode->type <<endl;
        if (type != T_DIR && type != T_FILE && type !=T_DEV && type != 0)
        {
            cerr << "ERROR: bad inode." << endl;
            exit(1);
        }

    }

}

void rootDir(char* fs, char* dataStart){
    struct dinode * root;
    char name;
    uint addrs[DIRSIZ];
    int i=0;
    struct dirent * dir;
    root = (struct dinode *)(fs+BSIZE+BSIZE+sizeof(dinode));
    cout << ";adkf " << root->addrs[0] << endl;

    if (root->type != 0)
    {
        //addrs = root->addrs;
        dir = (struct dirent *)(dataStart+root->addrs[i]);
        cout << "here " << dataStart << endl;
        while (strcmp(dir->name, "..")!= 0 && root->addrs[i]!= 0)
        {
            i++;
            cout << "name " << dir->name << endl;
            dir = (struct dirent *)(dataStart+root->addrs[i]);
        }
        cout << ";akldj" << endl;
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
    char * dataStart;
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
    if (r!=0) cout << "bad" <<endl;
    size = st.st_size;
    cout << "size " << size << endl;
    fs = (char*) mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    cout << "fs " <<fs<< endl;
    if (fs==MAP_FAILED)cout << "baddd "<<endl;
    sb = (struct superblock *)(fs + BSIZE);
    numiNodeBlocks = sb->ninodes /IPB;
    numBitMapBlocks = sb->size/BPB+1;
    cout << "size " << numBitMapBlocks << endl;
    cout << "inodes " << sb->size << endl;
    cout << "blocks " << sb->nblocks << endl;
    inodes = (struct dinode *) (fs + (BSIZE*2));
    dataStart = (char *)fs+BSIZE+BSIZE + numiNodeBlocks*BSIZE+numBitMapBlocks*BSIZE;
    cout << "types " << T_DIR <<" "<<T_FILE <<" "<<T_DEV << endl;
    cout << "size of " << inodes+2 << endl;
    inodeType(fs, sb->ninodes);
    cout << "finished inode type" << endl;
    rootDir(fs, dataStart);
    cout << "finished rootdir" << endl;
    if (munmap(fs,size)==-1) cout << "error" << endl;
    closed = close(fd);
    return 0;
}
