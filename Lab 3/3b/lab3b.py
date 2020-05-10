#!/usr/bin/python

"""
NAME: Shreya Raman
EMAIL: shreyaraman99@gmail.com
ID: 004923456
"""

from __future__ import print_function
import sys
import csv


class Superblock:
    def __init__(self, blockCount = 0, blocksPerGroup = 0, blockSize = 0, inodeCount = 0, inodesPerGroup = 0, inodeSize = 0, firstInode = 0, startInode = 0):
        self.blockCount = blockCount
        self.blockPerGroup = blocksPerGroup
        self.blockSize = blockSize        
        self.inodeCount = inodeCount
        self.inodePerGroup = inodesPerGroup
        self.inodeSize = inodeSize
        self.firstInode = firstInode
        self.startInode = startInode
    
    def startDataBlock(self):
        return self.startInode + (128 * self.inodeCount - 1) / self.blockSize + 1

class Block:
    def __init__(self, blockOffset = 0, blockLevel = 0, blockInode = 0):
        self.blockOffset = blockOffset
        self.blockLevel = blockLevel
        self.blockInode = blockInode

class Inode:
    def __init__(self, linksCount = 0, inodeNum = 0, inodeMode = 0):
        self.linksCount = linksCount
        self.links = 0        
        self.inodeNum = inodeNum
        self.inodeMode = inodeMode
        self.ptrs = []

class Dirent:
    def __init__(self, direntName, pInode, inodeNumber):
        self.direntName = direntName
        self.pInode = pInode
        self.inodeNumber = inodeNumber


superblock = Superblock()
freeInodeSet = set()
freeBlockSet = set()
direntSet = set()
inodeDict = dict()
blockDict = dict()
parentInodeDict = dict()


def parseInputFile():
    if len(sys.argv) != 2:
        print('Too many arguments!\nUsage: lab3b Filename', file=sys.stderr)
        sys.exit(1)

    try:
        inputFile = open(sys.argv[1], 'rb')
    except:
        print('File {} does not exist'.format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    csvreader = csv.reader(inputFile, delimiter=',')
    for row in csvreader:    
        if row[0] == 'GROUP':
            superblock.startInode = int(row[8])
            
        if row[0] == 'IFREE':
            freeInodeSet.add(int(row[1]))
        if row[0] == 'BFREE':
            freeBlockSet.add(int(row[1]))
            
        if row[0] == 'SUPERBLOCK':
            superblock.blockCount = int(row[1])
            superblock.inodeCount = int(row[2])
            superblock.blockSize = int(row[3])
            superblock.inodeSize = int(row[4])
            superblock.blockPerGroup = int(row[5])
            superblock.inodePerGroup = int(row[6])
            superblock.firstInode = int(row[7])

        if row[0] == 'INODE':
            inodeNumber = int(row[1])
            inode = Inode(int(row[6]), inodeNumber, int(row[3]))
            for i in range(15):
                blockNumber = int(row[12+i])
                inode.ptrs.append(blockNumber)
                level = 0
                offset = i
                if i > 11:
                    level = i - 11
                    offset = (level == 1) * 12 + (level == 2) * 268 + (level == 3) * 65804

                if blockNumber > 0:
                    block = Block(offset, level, inodeNumber) 
                    if blockNumber not in blockDict:
                        blockDict[blockNumber] = set()
                    blockDict[blockNumber].add(block)
            inodeDict[inode.inodeNum] = inode

        if row[0] == 'INDIRECT':
            inodeNumber = int(row[1])
            level = int(row[2]) - 1
            offset = int(row[3])
            blockNumber = int(row[5])

            block = Block(offset, level, inodeNumber)
            if blockNumber not in blockDict:
                blockDict[blockNumber] = set()
            blockDict[blockNumber].add(block)
                
        if row[0] == 'DIRENT':
            pInode = int(row[1])
            inodeNumber = int(row[3])
            direntName = row[6]
            dirent = Dirent(direntName, pInode, inodeNumber)
            direntSet.add(dirent)
            if direntName != '\'..\'' and direntName != '\'.\'':
                parentInodeDict[inodeNumber] = pInode

def dataBlock(blockNumber):
    if blockNumber < superblock.startDataBlock() or blockNumber > superblock.blockCount:
        return False
    return True


def blockFree(blockNumber):
    if not dataBlock(blockNumber):
        return False
    else:
        return blockNumber in freeBlockSet

def inodeFree(inodeNumber):
    if 2 < inodeNumber < superblock.firstInode:
        return False
    if inodeNumber > superblock.inodeCount:
        return False
    return inodeNumber in freeInodeSet


def scanDirents():
    global err
    keys = inodeDict.keys()
    for entry in direntSet:
        pInode = entry.pInode
        inodeNumber = entry.inodeNumber
        name = entry.direntName
        if entry.inodeNumber < 1 or entry.inodeNumber > superblock.inodeCount:
            print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(entry.pInode, entry.direntName, entry.inodeNumber))
            err = 2
            continue

        if entry.inodeNumber in inodeDict.keys():
            inodeDict[entry.inodeNumber].links += 1

        if name == '\'.\'':
            if pInode != inodeNumber:
                print("DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}".format(pInode, name, inodeNumber, pInode))
                err = 2
        elif name == '\'..\'':
            if inodeNumber == 2 or pInode == 2:
                gpInodeNum = 2
            else:
                gpInodeNum = parentInodeDict[pInode]

            if gpInodeNum != inodeNumber:
                print("DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}".format(pInode, name, inodeNumber, gpInodeNum))
                err = 2

        elif inodeFree(entry.inodeNumber):
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(entry.pInode, entry.direntName, entry.inodeNumber))
            err = 2

        elif entry.inodeNumber not in keys and entry.inodeNumber > superblock.firstInode:
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(entry.pInode, entry.direntName, entry.inodeNumber))
            err = 2

        elif inodeNumber in keys and inodeDict[inodeNumber].inodeMode <= 0:
            print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(entry.pInode, entry.direntName, entry.inodeNumber))
            err = 2

    inodes = inodeDict.values()
    for inode in inodes:
        if inode.inodeMode > 0 and inode.links != inode.linksCount:
            print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(inode.inodeNum, inode.links, inode.linksCount))
            err = 2

def scanInodes():
    keys = inodeDict.keys()
    global err
    for k in keys:
        i = inodeDict[k]
        if i.inodeMode <= 0:
            if not inodeFree(k):
                print("UNALLOCATED INODE {} NOT ON FREELIST".format(k))
                err = 2
        elif i.linksCount > 0:
            if inodeFree(k):
                print("ALLOCATED INODE {} ON FREELIST".format(k))
                err = 2
    for k in range(superblock.firstInode, 1+superblock.inodeCount):
        if not inodeFree(k) and k not in keys:
            print("UNALLOCATED INODE {} NOT ON FREELIST".format(k))
            err = 2
    return

def scanBlocks():
    keys = blockDict.keys()
    global err
    for k in keys:
        cur = blockDict[k]
        inum = list(cur)[0].blockInode
        info = 'BLOCK'
        if list(blockDict[k])[0].blockLevel == 1:
            info = 'INDIRECT BLOCK'
        if list(blockDict[k])[0].blockLevel == 2:
            info = 'DOUBLE INDIRECT BLOCK'
        if list(blockDict[k])[0].blockLevel == 3:
            info = 'TRIPLE INDIRECT BLOCK'

        if k < 0 or k >= superblock.blockCount:
            print("INVALID {} {} IN INODE {} AT OFFSET {}".format(info, k, inum, list(cur)[0].blockOffset))
            err = 2
        elif len(cur) > 1:
            for block in cur:
                info = 'BLOCK'
                if block.blockLevel == 1:
                    info = 'INDIRECT BLOCK'
                if block.blockLevel == 2:
                    info = 'DOUBLE INDIRECT BLOCK'
                if block.blockLevel == 3:
                    info = 'TRIPLE INDIRECT BLOCK'
                print("DUPLICATE {} {} IN INODE {} AT OFFSET {}".format(info, k, block.blockInode, block.blockOffset))
                err = 2
        elif blockFree(k):
            print("ALLOCATED BLOCK {} ON FREELIST".format(k))
            err = 2
        elif k < superblock.startDataBlock():
            print("RESERVED {} {} IN INODE {} AT OFFSET {}".format(info, k, inum, list(cur)[0].blockOffset))
            err = 2

    for bn in range(superblock.startDataBlock(), superblock.blockCount):
        if blockFree(bn) or bn in keys:
            continue
        else:
            print("UNREFERENCED BLOCK {}".format(bn))
            err = 2
    return


if __name__ == "__main__":

    parseInputFile()
    scanInodes()
    scanBlocks()
    scanDirents()

    sys.exit(err)
