/* Original comment follows :


  *
  * UAE - The Un*x Amiga Emulator
  *
  * memory management
  *
  * Copyright 1995 Bernd Schmidt
  *

* Original filenname memory.h
   
  28.10.2005, Rex Schilasky

  Thanks to the great UAE.

*/

#include "sysconfig.h"
#include "sysdeps.h"

#define ROMTYPE_KICK 1
#define ROMTYPE_KICKCD32 2
#define ROMTYPE_EXTCD32 4
#define ROMTYPE_EXTCDTV 8
#define ROMTYPE_A2091BOOT 16
#define ROMTYPE_A4091BOOT 32
#define ROMTYPE_AR 64
#define ROMTYPE_SUPERIV 128
#define ROMTYPE_KEY 256
#define ROMTYPE_ARCADIABIOS 512
#define ROMTYPE_ARCADIAGAME 1024
#define ROMTYPE_HRTMON 2048
#define ROMTYPE_NORDIC 4096
#define ROMTYPE_XPOWER 8192
#define ROMTYPE_CD32CART 16384
#define ROMTYPE_EVEN 131072
#define ROMTYPE_ODD 262144
#define ROMTYPE_BYTESWAP 524288
#define ROMTYPE_SCRAMBLED 1048576

struct romdata {
    char *name;
    int ver, rev;
    int subver, subrev;
    char *model;
    uae_u32 size;
    int id;
    int cpu;
    int cloanto;
    int type;
    int group;
    int title;
    uae_u32 crc32;
    uae_u32 sha1[5];
    char *configname;
};

struct romdata *scan_single_rom(FILE *f, const char* dir, const char* dlldir);
