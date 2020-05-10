/* Name: Shreya Raman
 * Email: shreyaraman99@gmail.com
 * ID: 004923456
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "ext2_fs.h"

struct ext2_super_block superblock;
int diskFD;
unsigned int blockSize;

unsigned long blockOffSet(unsigned int block) {
  return 1024 + (block - 1) * blockSize;
}

void directEntry(unsigned int pInode, unsigned int numBlock) {
  struct ext2_dir_entry dirEntry;
  unsigned long offset = blockOffSet(numBlock);
  unsigned int nb = 0;
  while (nb < blockSize) {
    memset(dirEntry.name, 0, 256);
    pread(diskFD, &dirEntry, sizeof(dirEntry), offset + nb);
    if (dirEntry.inode != 0) {
      memset(&dirEntry.name[dirEntry.name_len], 0, 256 - dirEntry.name_len);
      fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", pInode, nb, dirEntry.inode, dirEntry.rec_len, dirEntry.name_len, dirEntry.name);
    }
    nb += dirEntry.rec_len;
  }
}

void convertTime(time_t GMT, char* b) {
  time_t epoch = GMT;
  struct tm sec = *gmtime(&epoch);
  strftime(b, 80, "%m/%d/%y %H:%M:%S", &sec);
}

void readInode(unsigned int table, unsigned int idx, unsigned int inum) {
  struct ext2_inode inode;
  unsigned long offset = blockOffSet(table)+idx*sizeof(inode);
  pread(diskFD, &inode, sizeof(inode), offset);

  if(inode.i_links_count == 0 || inode.i_mode == 0) {
    return;
  }

  char ftype = '?';
  uint16_t val = (inode.i_mode >> 12) << 12;
  if (val == 0xa000) {
    ftype = 's';
  } else if (val == 0x8000) {
    ftype = 'f';
  } else if (val == 0x4000) {
    ftype = 'd';
  }

  unsigned int numBlock = 2 * (inode.i_blocks / (2 << superblock.s_log_block_size));

  fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,", inum, ftype, inode.i_mode & 0xFFF, inode.i_uid, inode.i_gid, inode.i_links_count);

  char ctime[20];
  char mtime[20];
  char atime[20];
  convertTime(inode.i_ctime, ctime);
  convertTime(inode.i_mtime, mtime);
  convertTime(inode.i_atime, atime);
  fprintf(stdout, "%s,%s,%s,", ctime, mtime, atime);
  fprintf(stdout, "%d,%d", inode.i_size, numBlock);

  int i;
  for (i = 0; i < 15; i++)
    fprintf(stdout, ",%d", inode.i_block[i]);
  fprintf(stdout, "\n");

  for (i = 0; i < 12; i++) {
    if (inode.i_block[i] != 0 && ftype == 'd') {
      directEntry(inum, inode.i_block[i]);
    }
  }

  if (inode.i_block[12] != 0) {
    uint32_t *ptrs = malloc(blockSize);
    uint32_t ptrCount = blockSize / sizeof(uint32_t);

    unsigned long off = blockOffSet(inode.i_block[12]);
    pread(diskFD, ptrs, blockSize, off);

    unsigned int j;
    for (j = 0; j < ptrCount; j++) {
      if (ptrs[j] != 0) {
	if (ftype == 'd') {
	  directEntry(inum, ptrs[j]);
	}
	fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inum, 1, j+12, inode.i_block[12], ptrs[j]);
      }
    }
    free(ptrs);
  }

  if (inode.i_block[13] != 0) {
    uint32_t *iptrs = malloc(blockSize);
    uint32_t ptrCount = blockSize / sizeof(uint32_t);

    unsigned long ioff = blockOffSet(inode.i_block[13]);
    pread(diskFD, iptrs, blockSize, ioff);
    unsigned int j;
    for(j = 0; j < ptrCount; j++) {
      if (iptrs[j] != 0) {
	fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inum, 2, 256 + j + 12, inode.i_block[13], iptrs[j]);

	uint32_t *bptrs = malloc(blockSize);
	unsigned long inoff = blockOffSet(iptrs[j]);
	pread(diskFD, bptrs, blockSize, inoff);
	unsigned int k;
	for (k = 0; k < ptrCount; k++) {
	  if (bptrs[k] != 0) {
	    if (ftype == 'd') {
	      directEntry(inum, bptrs[k]);
	    }
	    fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",inum,1,256+12+k,iptrs[j],bptrs[k]);
	  }
	}
	free(bptrs);
      }
    }
    free(iptrs);
  }

  if (inode.i_block[14] != 0) {
    uint32_t * inptrs = malloc(blockSize);
    uint32_t ptrCount = blockSize / sizeof(uint32_t);

    unsigned long doff = blockOffSet(inode.i_block[14]);
    pread(diskFD, inptrs, blockSize, doff);

    unsigned int j;
    for (j = 0; j < ptrCount; j++) {
      if (inptrs[j] != 0) {
	fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inum,3,256+12+j+65536,inode.i_block[14], inptrs[j]);

	uint32_t *ip = malloc(blockSize);
	unsigned long indoff = blockOffSet(inptrs[j]);
	pread(diskFD, ip, blockSize, indoff);
	unsigned int k;
	for (k = 0; k < ptrCount; k++) {
	  if (ip[k] != 0) {
	    fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inum,2,256+12+k+65536, inptrs[j], ip[k]);
	    uint32_t *bpt = malloc(blockSize);
	    unsigned long noff = blockOffSet(ip[k]);
	    pread(diskFD, bpt, blockSize, noff);
	    unsigned int m;
	    for(m = 0; m < ptrCount; m++) {
	      if (bpt[m] != 0) {
		if (ftype == 'd') {
		  directEntry(inum, bpt[m]);
		}
		fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",inum,1,256+12+m+65536,ip[k],bpt[m]);
	      }
	    }
	    free(bpt);
	  }
	}
	free(ip);
      }
    }
    free(inptrs);
  }
}

void printInode(int g, int b, int table) {
  int nb = superblock.s_inodes_per_group / 8;
  char * by = (char*) malloc(nb);

  unsigned int cur = g * superblock.s_inodes_per_group + 1;
  unsigned int st = cur;
  unsigned long offset = blockOffSet(b);
  pread(diskFD, by, nb, offset);

  int i,j;
  for (i = 0; i < nb; i++) {
    char t = by[i];
    for (j = 0; j < 8; j++) {
      int isUsed = 1 & t;
      if (isUsed)
        readInode(table, cur-st, cur);
      else
        fprintf(stdout, "IFREE,%d\n", cur);
      t >>= 1;
      cur++;
    }
  }
  free(by);
}

void freeBlocks(int c, unsigned int block) {
  char * bytes = (char*)malloc(blockSize);
  unsigned long off = blockOffSet(block);
  unsigned int curr = superblock.s_first_data_block + c * superblock.s_blocks_per_group;
  pread(diskFD, bytes, blockSize, off);

  unsigned int i;
  unsigned int j;
  for (i = 0; i < blockSize; i++) {
    char b = bytes[i];
    for (j = 0; j < 8; j++) {
      int isUsed = 1 & b;
      if (!isUsed) {
        fprintf(stdout, "BFREE,%d\n", curr);
      }
      b >>= 1;
      curr++;
    }
  }
  free(bytes);
}

void groups(int group, int tot) {
  struct ext2_group_desc groupDesc;
  uint32_t blockDesc = 0;
  if(blockSize == 1024)
    blockDesc = 2;
  else
    blockDesc = 1;

  unsigned long offset = blockSize * blockDesc + 32*group;
  pread(diskFD, &groupDesc, sizeof(groupDesc), offset);
  unsigned int blocksPerGroup = superblock.s_blocks_per_group;
  if(group == tot - 1)
    blocksPerGroup = superblock.s_blocks_count - (superblock.s_blocks_per_group * (tot-1));
  unsigned int inodesPerGroup = superblock.s_inodes_per_group;
  if(group == tot-1)
    inodesPerGroup = superblock.s_inodes_count - (superblock.s_inodes_per_group * (tot-1));
  fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", group, blocksPerGroup, inodesPerGroup, groupDesc.bg_free_blocks_count, groupDesc.bg_free_inodes_count, groupDesc.bg_block_bitmap, groupDesc.bg_inode_bitmap, groupDesc.bg_inode_table);
  unsigned int bitMap = groupDesc.bg_block_bitmap;
  freeBlocks(group, bitMap);
  unsigned int ibitMap = groupDesc.bg_inode_bitmap;
  unsigned int itable = groupDesc.bg_inode_table;
  printInode(group, ibitMap, itable);
}

void superblockCSV() {
  pread(diskFD, &superblock, sizeof(superblock), 1024);
  fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", superblock.s_blocks_count, superblock.s_inodes_count,blockSize, superblock.s_inode_size, superblock.s_blocks_per_group, superblock.s_inodes_per_group, superblock.s_first_ino);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Error: Invalid Arguments\n");
    exit(1);
  }
  
  struct option opts[] = {{0,0,0,0}};

  if (getopt_long(argc, argv, "", opts, NULL) != -1) {
    fprintf(stderr, "Error: Invalid Arguments\n");
    exit(1);
  }

  if ((diskFD = open(argv[1], O_RDONLY)) == -1) {
    fprintf(stderr, "Error: Couldn't open file\n");
    exit(1);
  }
  
  blockSize = EXT2_MIN_BLOCK_SIZE << superblock.s_log_block_size;
  superblockCSV();
  int ngroup = superblock.s_blocks_count / superblock.s_blocks_per_group;
  if ((double) ngroup < (double) superblock.s_blocks_count / superblock.s_blocks_per_group) {
    ngroup++;
  }
  int i;
  for (i = 0; i < ngroup; i++) {
    groups(i, ngroup);
  }
  return 0;
}
