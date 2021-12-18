/* Original comment follows :


  * UAE - The Un*x Amiga Emulator
  *
  * Memory management
  *
  * (c) 1995 Bernd Schmidt


* Original filenname memory.c, extracted 5 functions/strcutures 
  and adapt them for RomFind.
  
  28.10.2005, Rex Schilasky

  Thanks to the great UAE.

*/

#include "stdio.h"
#include "windows.h"
#include "io.h"
#include "stdlib.h"
#include "memory.h"
#include "string.h"

#include "sysconfig.h"
#include "sysdeps.h"
#include "crc32.h"

#include "rmemory.h"


#define ALTROM(id,grp,num,size,flags,crc32,a,b,c,d,e) \
    { "X", 0, 0, 0, 0, 0, size, id, 0, 0, flags, (grp << 16) | num, 0, crc32, a, b, c, d, e },

static struct romdata roms[] = {
    { "Cloanto Amiga Forever ROM key", 0, 0, 0, 0, 0, 2069, 0, 0, 1, ROMTYPE_KEY, 0, 0,
	0x869ae1b1, 0x801bbab3,0x2e3d3738,0x6dd1636d,0x4f1d6fa7,0xe21d5874 },
    { "Cloanto Amiga Forever 2006 ROM key", 0, 0, 0, 0, 0, 750, 48, 0, 1, ROMTYPE_KEY, 0, 0,
	0xb01c4b56, 0xbba8e5cd,0x118b8d92,0xafed5693,0x5eeb9770,0x2a662d8f },

    { "KS ROM v1.0 (A1000)(NTSC)", 1, 0, 1, 0, "A1000\0", 262144, 1, 0, 0, ROMTYPE_KICK, 0, 0,
	0x299790ff, 0x00C15406,0xBEB4B8AB,0x1A16AA66,0xC05860E1,0xA7C1AD79 },
    { "KS ROM v1.1 (A1000)(NTSC)", 1, 1, 31, 34, "A1000\0", 262144, 2, 0, 0, ROMTYPE_KICK, 0, 0,
	0xd060572a, 0x4192C505,0xD130F446,0xB2ADA6BD,0xC91DAE73,0x0ACAFB4C},
    { "KS ROM v1.1 (A1000)(PAL)", 1, 1, 31, 34, "A1000\0", 262144, 3, 0, 0, ROMTYPE_KICK, 0, 0,
	0xec86dae2, 0x16DF8B5F,0xD524C5A1,0xC7584B24,0x57AC15AF,0xF9E3AD6D },
    { "KS ROM v1.2 (A1000)", 1, 2, 33, 166, "A1000\0", 262144, 4, 0, 0, ROMTYPE_KICK, 0, 0,
	0x9ed783d0, 0x6A7BFB5D,0xBD6B8F17,0x9F03DA84,0xD8D95282,0x67B6273B },
    { "KS ROM v1.2 (A500,A1000,A2000)", 1, 2, 33, 180, "A500\0A1000\0A2000\0", 262144, 5, 0, 0, ROMTYPE_KICK, 0, 0,
	0xa6ce1636, 0x11F9E62C,0xF299F721,0x84835B7B,0x2A70A163,0x33FC0D88 },
    { "KS ROM v1.3 (A500,A1000,A2000)", 1, 3, 34, 5, "A500\0A1000\0A2000\0", 262144, 6, 0, 0, ROMTYPE_KICK, 0, 0,
	0xc4f0f55f, 0x891E9A54,0x7772FE0C,0x6C19B610,0xBAF8BC4E,0xA7FCB785 },
    { "KS ROM v1.3 (A3000)(SK)", 1, 3, 34, 5, "A3000\0", 262144, 32, 0, 0, ROMTYPE_KICK, 0, 0,
	0xe0f37258, 0xC39BD909,0x4D4E5F4E,0x28C1411F,0x30869504,0x06062E87 },
    { "KS ROM v1.4 (A3000)", 1, 4, 36, 16, "A3000\0", 524288, 59, 3, 0, ROMTYPE_KICK, 0, 0,
	0xbc0ec13f, 0xF76316BF,0x36DFF14B,0x20FA349E,0xD02E4B11,0xDD932B07 },

    { "KS ROM v2.04 (A500+)", 2, 4, 37, 175, "A500+\0", 524288, 7, 0, 0, ROMTYPE_KICK, 0, 0,
	0xc3bdb240, 0xC5839F5C,0xB98A7A89,0x47065C3E,0xD2F14F5F,0x42E334A1 },
    { "KS ROM v2.05 (A600)", 2, 5, 37, 299, "A600\0", 524288, 8, 0, 0, ROMTYPE_KICK, 0, 0,
	0x83028fb5, 0x87508DE8,0x34DC7EB4,0x7359CEDE,0x72D2E3C8,0xA2E5D8DB },
    { "KS ROM v2.05 (A600HD)", 2, 5, 37, 300, "A600HD\0A600\0", 524288, 9, 0, 0, ROMTYPE_KICK, 0, 0,
	0x64466c2a, 0xF72D8914,0x8DAC39C6,0x96E30B10,0x859EBC85,0x9226637B },
    { "KS ROM v2.05 (A600HD)", 2, 5, 37, 350, "A600HD\0A600\0", 524288, 10, 0, 0, ROMTYPE_KICK, 0, 0,
	0x43b0df7b, 0x02843C42,0x53BBD29A,0xBA535B0A,0xA3BD9A85,0x034ECDE4 },
    { "KS ROM v2.04 (A3000)", 2, 4, 37, 132, "A3000\0", 524288, 71, 3, 0, ROMTYPE_KICK, 0, 0,
	0x234a7233, 0xd82ebb59,0xafc53540,0xddf2d718,0x7ecf239b,0x7ea91590 },
    ALTROM(71, 1, 1, 262144, ROMTYPE_EVEN, 0x7db1332b,0x48f14b31,0x279da675,0x7848df6f,0xeb531881,0x8f8f576c)
    ALTROM(71, 1, 2, 262144, ROMTYPE_ODD , 0xa245dbdf,0x83bab8e9,0x5d378b55,0xb0c6ae65,0x61385a96,0xf638598f)

    { "KS ROM v3.0 (A1200)", 3, 0, 39, 106, "A1200\0", 524288, 11, 0, 0, ROMTYPE_KICK, 0, 0,
	0x6c9b07d2, 0x70033828,0x182FFFC7,0xED106E53,0x73A8B89D,0xDA76FAA5 },
    { "KS ROM v3.0 (A4000)", 3, 0, 39, 106, "A4000\0", 524288, 12, 2 | 4, 0, ROMTYPE_KICK, 0, 0,
	0x9e6ac152, 0xF0B4E9E2,0x9E12218C,0x2D5BD702,0x0E4E7852,0x97D91FD7 },
    { "KS ROM v3.1 (A4000)", 3, 1, 40, 70, "A4000\0", 524288, 13, 2 | 4, 0, ROMTYPE_KICK, 0, 0,
	0x2b4566f1, 0x81c631dd,0x096bbb31,0xd2af9029,0x9c76b774,0xdb74076c },
    { "KS ROM v3.1 (A500,A600,A2000)", 3, 1, 40, 63, "A500\0A600\0A2000\0", 524288, 14, 0, 0, ROMTYPE_KICK, 0, 0,
	0xfc24ae0d, 0x3B7F1493,0xB27E2128,0x30F989F2,0x6CA76C02,0x049F09CA },
    { "KS ROM v3.1 (A1200)", 3, 1, 40, 68, "A1200\0", 524288, 15, 1, 0, ROMTYPE_KICK, 0, 0,
	0x1483a091, 0xE2154572,0x3FE8374E,0x91342617,0x604F1B3D,0x703094F1 },
    { "KS ROM v3.1 (A3000)", 3, 1, 40, 68, "A3000\0", 524288, 61, 2, 0, ROMTYPE_KICK, 0, 0,
	0xefb239cc, 0xF8E210D7,0x2B4C4853,0xE0C9B85D,0x223BA20E,0x3D1B36EE },
    { "KS ROM v3.1 (A4000)(Cloanto)", 3, 1, 40, 68, "A4000\0", 524288, 31, 2 | 4, 1, ROMTYPE_KICK, 0, 0,
	0x43b6dd22, 0xC3C48116,0x0866E60D,0x085E436A,0x24DB3617,0xFF60B5F9 },
    { "KS ROM v3.1 (A4000)", 3, 1, 40, 68, "A4000\0", 524288, 16, 2 | 4, 0, ROMTYPE_KICK, 0, 0,
	0xd6bae334, 0x5FE04842,0xD04A4897,0x20F0F4BB,0x0E469481,0x99406F49 },
    { "KS ROM v3.1 (A4000T)", 3, 1, 40, 70, "A4000T\0", 524288, 17, 2 | 4, 0, ROMTYPE_KICK, 0, 0,
	0x75932c3a, 0xB0EC8B84,0xD6768321,0xE01209F1,0x1E6248F2,0xF5281A21 },
    { "KS ROM v3.X (A4000)(Cloanto)", 3, 10, 45, 57, "A4000\0", 524288, 46, 2 | 4, 0, ROMTYPE_KICK, 0, 0,
	0x08b69382, 0x81D3AEA3,0x0DB7FBBB,0x4AFEE41C,0x21C5ED66,0x2B70CA53 },

    { "KS ROM v3.X (A4000)(Cloanto AF2008)", 3, 10, 45, 57, "A4000\0", 524288, 46, 2 | 4, 0, ROMTYPE_KICK, 0, 0,
	0x08b69382, 0x3cbfc9e1,0xfe396360,0x157bd161,0xde74fc90,0x1abee7ec },

    { "CD32 KS ROM v3.1", 3, 1, 40, 60, "CD32\0", 524288, 18, 1, 0, ROMTYPE_KICKCD32, 0, 0,
	0x1e62d4a5, 0x3525BE88,0x87F79B59,0x29E017B4,0x2380A79E,0xDFEE542D },
    { "CD32 extended ROM", 3, 1, 40, 60, "CD32\0", 524288, 19, 1, 0, ROMTYPE_EXTCD32, 0, 0,
	0x87746be2, 0x5BEF3D62,0x8CE59CC0,0x2A66E6E4,0xAE0DA48F,0x60E78F7F },
    { "CD32 ROM (KS + extended)", 3, 1, 40, 60, "CD32\0", 2 * 524288, 64, 1, 0, ROMTYPE_KICKCD32 | ROMTYPE_EXTCD32, 0, 0,
	0xd3837ae4, 0x06807db3,0x18163745,0x5f4d4658,0x2d9972af,0xec8956d9 },
    { "CD32 MPEG Cartridge ROM", 3, 1, 40, 30, "CD32\0", 262144, 72, 1, 0, ROMTYPE_CD32CART, 0, 0,
	0xc35c37bf, 0x03ca81c7,0xa7b259cf,0x64bc9582,0x863eca0f,0x6529f435 },

    { "CDTV extended ROM v1.00", 1, 0, 1, 0, "CDTV\0", 262144, 20, 0, 0, ROMTYPE_EXTCDTV, 0, 0,
	0x42baa124, 0x7BA40FFA,0x17E500ED,0x9FED041F,0x3424BD81,0xD9C907BE },
    ALTROM(20, 1, 1, 131072, ROMTYPE_EVEN, 0x791cb14b,0x277a1778,0x92449635,0x3ffe56be,0x68063d2a,0x334360e4)
    ALTROM(20, 1, 2, 131072, ROMTYPE_ODD,  0xaccbbc2e,0x41b06d16,0x79c6e693,0x3c3378b7,0x626025f7,0x641ebc5c)
    { "CDTV extended ROM v2.07", 2, 7, 2, 7, "CDTV\0", 262144, 22, 0, 0, ROMTYPE_EXTCDTV, 0, 0,
	0xceae68d2, 0x5BC114BB,0xA29F60A6,0x14A31174,0x5B3E2464,0xBFA06846 },
    { "CDTV extended ROM v2.30", 2, 30, 2, 30, "CDTV\0", 262144, 21, 0, 0, ROMTYPE_EXTCDTV, 0, 0,
	0x30b54232, 0xED7E461D,0x1FFF3CDA,0x321631AE,0x42B80E3C,0xD4FA5EBB },

    { "A1000 bootstrap ROM", 0, 0, 0, 0, "A1000\0", 8192, 23, 0, 0, ROMTYPE_KICK, 0, 0,
	0x62f11c04, 0xC87F9FAD,0xA4EE4E69,0xF3CCA0C3,0x6193BE82,0x2B9F5FE6 },
    { "A1000 bootstrap ROM", 0, 0, 0, 0, "A1000\0", 65536, 24, 0, 0, ROMTYPE_KICK, 0, 0,
	0x0b1ad2d0, 0xBA93B8B8,0x5CA0D83A,0x68225CC3,0x3B95050D,0x72D2FDD7 },
    ALTROM(23, 1, 1, 65536,           0, 0x0b1ad2d0,0xBA93B8B8,0x5CA0D83A,0x68225CC3,0x3B95050D,0x72D2FDD7)
    ALTROM(23, 2, 1, 4096, ROMTYPE_EVEN, 0x42553bc4,0x8855a97f,0x7a44e3f6,0x2d1c88d9,0x38fee1f4,0xc606af5b)
    ALTROM(23, 2, 2, 4096, ROMTYPE_ODD , 0x8e5b9a37,0xd10f1564,0xb99f5ffe,0x108fa042,0x362e877f,0x569de2c3)

    { "KS ROM v3.1.4 (A1200)", 3, 1, 40, 68, "A1200\0", 524288, 15, 1, 0, ROMTYPE_KICK, 0, 0,
  0x1483a091,0x6355a9ed,0x5dc84042,0x2f9b7330,0x8a91be0d,0x0bb506bd },
    { "KS ROM v3.1.4 (A4000)", 3, 1, 40, 68, "A4000\0", 524288, 15, 1, 0, ROMTYPE_KICK, 0, 0,
  0x1483a091,0xbad0ae38,0x8442db02,0xeaeb6a53,0x63ef43eb,0x4e308ae6 },
    { "KS ROM v3.1.4 (A4000T)", 3, 1, 40, 68, "A4000\0", 524288, 15, 1, 0, ROMTYPE_KICK, 0, 0,
  0x1483a091,0x938a60a1,0xd2c0d411,0xf64bb27e,0x5af40258,0xb8decbf3 },
  
    { NULL }
};

int decode_cloanto_rom_do (unsigned char *mem, int size, int real_size, unsigned char *key, int keysize)
{
  long cnt, t;
  for (t = cnt = 0; cnt < size; cnt++, t = (t + 1) % keysize)  {
    mem[cnt] ^= key[t];
    if (real_size == cnt + 1)
      t = keysize - 1;
  }
  if ((mem[2] == 0x4e && mem[3] == 0xf9) || (mem[0] == 0x11 && (mem[1] == 0x11 || mem[1] == 0x14)))
    return 1;
  for (t = cnt = 0; cnt < size; cnt++, t = (t + 1) % keysize)  {
    mem[cnt] ^= key[t];
    if (real_size == cnt + 1)
      t = keysize - 1;
  }
  return 0;
}

typedef DWORD (STDAPICALLTYPE *PFN_GetKey)(LPVOID lpvBuffer, DWORD dwSize);
unsigned char* cloanto_load_keyfile(int* pSize, const char* dlldir)
{
  void* keybuf = NULL;
  HMODULE h;
  PFN_GetKey pfnGetKey;
  DWORD size;

  h = LoadLibraryA("amigaforever.dll");
  if (!h)
  {
    char path[MAX_PATH] = {0};
    strcat(path, dlldir);
    strcat(path, "\\amigaforever.dll");
    h = LoadLibraryA(path);
  }
  if (!h) return NULL;
  
  pfnGetKey = (PFN_GetKey)GetProcAddress(h, "GetKey");
  if(pfnGetKey) 
  {
    size = pfnGetKey(NULL, 0);
    *pSize = size;
    if(size > 0) 
    {
      keybuf = malloc(size);
      if(pfnGetKey(keybuf, size) != size) 
      {
        free(keybuf);
        keybuf = NULL;
      }
    }
  }

  FreeLibrary (h);
  return((unsigned char*)keybuf);
}

int decode_cloanto_rom (unsigned char *mem, int size, int real_size, const char* dir, const char* dlldir)
{
  int ret = 0;
  int keysize;

  unsigned char* p = cloanto_load_keyfile(&keysize, dlldir);
  if(p != NULL)
  {
    ret = decode_cloanto_rom_do (mem, size, real_size, p, keysize);
    free(p);
  }
  if(!ret)
  {
    FILE*         fp = NULL;
    int           fd = 0;
    unsigned char* p = NULL;

    char rom_path[_MAX_PATH] = "";
    strcat(rom_path, dir);
    strcat(rom_path, "\\");
    strcat(rom_path, "rom.key");
    fp = fopen(rom_path, "rb");
    if (!fp) {
      return 0;
    }
    fd = _fileno(fp);
    keysize = _filelength(fd);
    p = (unsigned char*)malloc(keysize);
    if(fread(p, 1, keysize, fp))
    {
      ret = decode_cloanto_rom_do (mem, size, real_size, p, keysize);
    }
    
    free(p);
    fclose(fp);
  }

#ifdef _DEBUG
  {
    FILE* dbf = fopen("_encoded.rom", "wb");
    if(dbf)
    {
      fwrite(mem, 1, size, dbf);
      fclose(dbf);
    }
  }
#endif

  return ret;
}

static __forceinline int notcrc32(uae_u32 crc32)
{
    if (crc32 == 0xffffffff || crc32 == 0x00000000)
	return 1;
    return 0;
}

static int cmpsha1(uae_u8 *s1, struct romdata *rd)
{
  int i;

  for (i = 0; i < SHA1_SIZE / 4; i++) {
    uae_u32 v1 = (s1[0] << 24) | (s1[1] << 16) | (s1[2] << 8) | (s1[3] << 0);
    uae_u32 v2 = rd->sha1[i];
    if (v1 != v2)
      return -1;
    s1 += 4;
  }
  return 0;
}

static struct romdata *checkromdata(uae_u8 *sha1, int size, uae_u32 mask)
{
  int i = 0;
  while (roms[i].name) {
    if (!notcrc32(roms[i].crc32) && roms[i].size >= size) {
      if (roms[i].type & mask) {
        if (!cmpsha1(sha1, &roms[i]))
          return &roms[i];
      }
    }
    i++;
  }
  return NULL;
}

struct romdata *getromdatabydata (unsigned char *rom, int size, const char* dir, const char* dlldir)
{
  uae_u8 sha1[SHA1_SIZE];
  uae_u8 tmp[4];
  uae_u8 *tmpbuf = NULL;
  struct romdata *ret = NULL;
  int    cloanto = 0;

  if (size > 11 && !memcmp (rom, "AMIROMTYPE1", 11)) {
    uae_u8 *tmpbuf = (uae_u8*)malloc (size);
    int tmpsize = size - 11;
    memcpy (tmpbuf, rom + 11, tmpsize);
    decode_cloanto_rom (tmpbuf, tmpsize, tmpsize, dir, dlldir);
    rom = tmpbuf;
    size = tmpsize;
    cloanto = 1;
  }
  get_sha1 (rom, size, sha1);
  ret = checkromdata(sha1, size, -1);
  if (!ret) {
    get_sha1 (rom, size / 2, sha1);
    ret = checkromdata (sha1, size / 2, -1);
    if (!ret) {
      /* ignore AR IO-port range until we have full dump */
      memcpy (tmp, rom, 4);
      memset (rom, 0, 4);
      get_sha1 (rom, size, sha1);
      ret = checkromdata (sha1, size, ROMTYPE_AR);
      memcpy (rom, tmp, 4);
    }
  }
  free (tmpbuf);
  
  // if we had decoded the rom file 
  // we do mark as AF rom in any case
  if(ret && cloanto)
  {
    ret->cloanto = 1;
  }
  
  return ret;
}

struct romdata *scan_single_rom(FILE *f, const char* dir, const char* dlldir)
{
  unsigned char  *rombuf;
  int             size;
  struct romdata *rd = 0;
  
  fseek (f, 0, SEEK_END);
  size = ftell (f);
  fseek (f, 0, SEEK_SET);
  if (size > 600000) return 0;

  rombuf = (unsigned char*)calloc (size, 1);
  if(!rombuf) return 0;
  fread (rombuf, 1, size, f);
  
  rd = getromdatabydata (rombuf, size, dir, dlldir);

  free (rombuf);
  
  return rd;
}
