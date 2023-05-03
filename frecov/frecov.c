#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN  0x02
#define ATTR_SYSTEM   0x04
#define ATTR_VOLUME_ID   0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE  0x20
#define ATTR_LONG_NAME_MASK \
(ATTR_READ_ONLY | \
ATTR_HIDDEN | \
ATTR_SYSTEM | \
ATTR_VOLUME_ID | \
ATTR_DIRECTORY | \
ATTR_ARCHIVE)
#define LAST_LONG_ENTRY 0x40



// #define test(a,b) ((a)&(b))


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// Copied from the manual
// BPB structure
struct fat32hdr {
  u8  BS_jmpBoot[3];
  u8  BS_OEMName[8];
  u16 BPB_BytsPerSec;
  u8  BPB_SecPerClus;
  u16 BPB_RsvdSecCnt;
  u8  BPB_NumFATs;
  u16 BPB_RootEntCnt;
  u16 BPB_TotSec16;
  u8  BPB_Media;
  u16 BPB_FATSz16;
  u16 BPB_SecPerTrk;
  u16 BPB_NumHeads;
  u32 BPB_HiddSec;
  u32 BPB_TotSec32;
  u32 BPB_FATSz32;
  u16 BPB_ExtFlags;
  u16 BPB_FSVer;
  u32 BPB_RootClus;
  u16 BPB_FSInfo;
  u16 BPB_BkBootSec;
  u8  BPB_Reserved[12];
  u8  BS_DrvNum;
  u8  BS_Reserved1;
  u8  BS_BootSig;
  u32 BS_VolID;
  u8  BS_VolLab[11];
  u8  BS_FilSysType[8];
  u8  __padding_1[420];
  u16 Signature_word;
} __attribute__((packed));// packed指原始的未优化的内存空间

union dent{
  struct {
  u8 LDIR_Ord; // last member(N) is N|LAST_LONG_ENTRY(0x40)
  u16 LDIR_Name1[5];
  u8 LDIR_Attr;
  u8 LDIR_Type; // zero
  u8 LDIR_Chksum; 
  u16 LDIR_Name2[6];
  u16 LDIR_FstClusLO; //zero
  u16 LDIR_Name3[2 ];
  }__attribute__((packed)) long_name_dent;
  struct {
  u8 DIR_Name[11];
  u8 DIR_Attr;
  u8 DIR_NTRes;
  u8 DIR_CrtTimeTenth;
  u16 DIR_CrtTime;
  u16 DIR_CrtDate;
  u16 DIR_LstAccDate;
  u16 DIR_FstClusHI;
  u16 DIR_WrtTime;
  u16 DIR_WrtDate;
  u16 DIR_FstClusLO;
  u32 DIR_FileSize;
  }__attribute__((packed)) origin_dent;
};

int is_long_name_dent(union dent *x){

  return x->long_name_dent.LDIR_Attr == (ATTR_READ_ONLY | \
    ATTR_HIDDEN | \
    ATTR_SYSTEM | \
    ATTR_VOLUME_ID);

}

int is_short_dent(union dent *x){
  return x->origin_dent.DIR_Attr == (ATTR_ARCHIVE | ATTR_DIRECTORY);
}






struct fatentry{
  u32 values;
}; // section 4

struct fat32hdr *hdr;
// enum fatentrystatus{
//   FREE = 1,
//   allocated,
//   reserved,
//   bad,
//   may_be_end_of_file,
//   end_of_file
// };

u32 BytsPerSec;
u32 BytesPerCluster;
u32 Maximum_Valid_Cluster_Number;
u32 FirstDataSector;
off_t size;
u32 FirstSectorofCluster(u32 cluster_num){
  return (cluster_num - 2)*(hdr->BPB_SecPerClus) + FirstDataSector;
}

void *map_disk(const char *fname);

void retrive_long(union dent* entry){

  int top=0;
  u16 name[256];
  // char expends[32];

  if(!((entry->long_name_dent.LDIR_Ord)&(LAST_LONG_ENTRY))) return;// not a long_name_dent

  int ori = (entry->long_name_dent.LDIR_Ord)&(~LAST_LONG_ENTRY); // original

  union dent * final_entry =  entry + (ori - 1);

  for(int i=1;i<=ori;i++){
    union dent * next_entry =  entry + (ori - i);
    for(int j=0;j<sizeof(next_entry->long_name_dent.LDIR_Name1)/sizeof(u16);j++){
      if(next_entry->long_name_dent.LDIR_Name1[j] != 0x0000){
        name[top++] = next_entry->long_name_dent.LDIR_Name1[j];
      }
      goto done;
    }
    for(int j=0;j<sizeof(next_entry->long_name_dent.LDIR_Name2)/sizeof(u16);j++){
      if(next_entry->long_name_dent.LDIR_Name1[j] != 0x0000){
        name[top++] = next_entry->long_name_dent.LDIR_Name1[j];
      }
      goto done;
    }
    for(int j=0;j<sizeof(next_entry->long_name_dent.LDIR_Name3)/sizeof(u16);j++){
      if(next_entry->long_name_dent.LDIR_Name1[j] != 0x0000){
        name[top++] = next_entry->long_name_dent.LDIR_Name1[j];
      }
      goto done;
    }
  }

done:
  name[top] = '\0';
  for(int i=0;i<top;i++){
    putchar(name[i]);
  }



}

void doit(u8 *whole_disk){

  off_t data_offect = FirstDataSector*BytesPerCluster;

  union dent* entry = (union dent*)(&whole_disk[data_offect]);
  
  while(((uintptr_t)entry-(uintptr_t)whole_disk)<size){
    if(is_long_name_dent(entry)){ // long entry
      retrive_long(entry);
    }else if(is_short_dent(entry)){

    }
  }
  
  


}



int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s fs-image\n", argv[0]);
    exit(1);
  }


  setbuf(stdout, NULL);

  assert(sizeof(struct fat32hdr) == 512); // defensive
  assert(sizeof(union dent) == 32);
  // map disk image to memory
  hdr = map_disk(argv[1]);

  BytsPerSec = hdr->BPB_BytsPerSec;
  BytesPerCluster = hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus;
  FirstDataSector = hdr->BPB_RsvdSecCnt + (hdr->BPB_NumFATs * hdr->BPB_FATSz32) + ((hdr->BPB_RootEntCnt *32 ) + (hdr->BPB_BytsPerSec -1))/hdr->BPB_BytsPerSec;
  Maximum_Valid_Cluster_Number =  (hdr->BPB_TotSec32 - (hdr->BPB_RsvdSecCnt)+(hdr->BPB_NumFATs*hdr->BPB_FATSz32))/hdr->BPB_SecPerClus;
  // TODO: frecov
  doit((u8 *)hdr);
  // file system traversal
  munmap(hdr, hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec);
}

void *map_disk(const char *fname) {
  int fd = open(fname, O_RDWR);

  if (fd < 0) {
    perror(fname);
    goto release;
  }

  size = lseek(fd, 0, SEEK_END);
  if (size == -1) {
    perror(fname);
    goto release;
  }

  struct fat32hdr *hdr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (hdr == (void *)-1) {
    goto release;
  }

  close(fd);

  if (hdr->Signature_word != 0xaa55 ||
      hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec != size) {
    fprintf(stderr, "%s: Not a FAT file image\n", fname);
    goto release;
  }
  return hdr;

release:
  if (fd > 0) {
    close(fd);
  }
  exit(1);
}
