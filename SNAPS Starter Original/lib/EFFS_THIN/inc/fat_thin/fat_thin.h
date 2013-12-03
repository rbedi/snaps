#ifndef _FAT_THIN_H_
#define _FAT_THIN_H_

/****************************************************************************
 *
 *            Copyright (c) 2003 by HCC Embedded 
 *
 * This software is copyrighted by and is the sole property of 
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,  
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1132
 * Victor Hugo Utca 11-15
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


#ifndef NULL
#define NULL (void *)0
#endif

/****************************************************************************
 *
 * some definition
 *
 ***************************************************************************/


#define F_SECTOR_SIZE 512
#define F_MAXNAME 8		/* 8 byte name */
#define F_MAXEXT  3		/* 3 byte extension */


typedef struct {
unsigned char sector_per_cluster;
unsigned char number_of_FATs;
unsigned char media_descriptor;

#if (F_FORMATTING && F_WRITING)
unsigned char jump_code[3];
unsigned char OEM_name [8];
unsigned short bytes_per_sector;

unsigned short reserved_sectors;
unsigned short max_root_entry;
unsigned short number_of_sectors_less32;/* <32M */
unsigned short sector_per_FAT;
unsigned short sector_per_Track;
unsigned short number_of_heads;
unsigned long number_of_hidden_sectors;
unsigned long number_of_sectors;

unsigned short logical_drive_num;
unsigned char extended_signature;
unsigned long serial_number;
unsigned char FAT_name[8];
unsigned char executable_marker[2];
#endif
#if (F_FORMATTING || F_GETLABEL)
unsigned char volume_name[11];
#endif

#if F_FAT32
/*only on fat32*/
unsigned long rootcluster;
#if (F_FORMATTING && F_WRITING)
unsigned long sector_per_FAT32;
unsigned short extflags;
unsigned short fsversion;
unsigned short fsinfo;
unsigned short bkbootsec;
unsigned char reserved[12];
#endif
#endif
/*fat12-fat16-fat32*/
} F_BOOTRECORD;


typedef struct {
unsigned short number_of_cylinders;
unsigned short sector_per_track;
unsigned short number_of_heads;
unsigned long number_of_sectors;
} F_PHY;


#if (!F_LONGFILENAME)

typedef struct {
#if F_DIRECTORIES
	char path [F_MAXPATH];		/*  /directory1/dir2/  */
#endif
	char filename[F_MAXNAME];	/* filename */
	char fileext [F_MAXEXT];  	/* extension */
} F_NAME;

#else


typedef struct {
#if F_DIRECTORIES
	char path  [F_MAXPATH];		/*  /directory1/dir2/  */
#endif
	char lname [F_MAXLNAME];	/* long file name  */
} F_NAME;

#endif /* F_LONGFILENAME  */

typedef struct {
	unsigned long cluster;
	unsigned long sector;
	unsigned long sectorend;
	unsigned long pos;
} F_POS;

typedef struct {
	char filename[F_MAXPATH];	/*file name+ext*/
	char name[F_MAXNAME];		/*file name*/
	char ext[F_MAXEXT];		/*file extension*/
	unsigned char attr;		/*attribute of the file*/

	unsigned short ctime;		/*creation time*/
	unsigned short cdate;		/*creation date*/
	unsigned long cluster;

	long filesize;					/*length of file*/

	F_NAME findfsname;		   	/*find properties*/
	F_POS pos;
} F_FIND; 


#if F_LONGFILENAME 

typedef struct {
   char name[261]; /*with zero term*/
   unsigned char ord;
   unsigned char chksum;
   unsigned char state; 
   unsigned long start;
   unsigned long end;
} F_LFNINT;

enum {
F_LFNSTATE_LFN,  /*lfn is useable*/
F_LFNSTATE_SFN,  /*lfn is useable, contains short filename*/
F_LFNSTATE_NEXT, /*lfn need more entry*/
F_LFNSTATE_INV   /*invalid lfn*/
};

#endif  /*F_LONGFILENAME*/

typedef struct {
unsigned char name[F_MAXNAME];	/* 8+3 */
unsigned char ext[F_MAXEXT];	
unsigned char attr;					/* 00ADVSHR */

unsigned char ntres;      /* FAT32 only  */
unsigned char crttimetenth;    /* FAT32 only  */
unsigned char crttime[2];   /* FAT32 only  */
unsigned char crtdate[2];   /* FAT32 only */
unsigned char lastaccessdate[2];/* FAT32 only */

unsigned char clusterhi[2]; /* FAT32 only */
unsigned char ctime[2];
unsigned char cdate[2];
unsigned char clusterlo[2]; /* fat12,fat16,fat32 */
unsigned char filesize[4];

} F_DIRENTRY;

#define NTRES_LOW 0x08 /*lower case name*/

#define F_ATTR_ARC      0x20
#define F_ATTR_DIR      0x10
#define F_ATTR_VOLUME   0x08
#define F_ATTR_SYSTEM   0x04
#define F_ATTR_HIDDEN   0x02
#define F_ATTR_READONLY 0x01

#if F_LONGFILENAME 

typedef struct {
unsigned char ord;

unsigned char lfn_1;
unsigned char lfnhi_1;

unsigned char lfn_2;
unsigned char lfnhi_2;

unsigned char lfn_3;
unsigned char lfnhi_3;

unsigned char lfn_4;
unsigned char lfnhi_4;

unsigned char lfn_5;
unsigned char lfnhi_5;

unsigned char attr;	/*00ADVSHR*/
unsigned char type;  /*always 0*/

unsigned char chksum;

unsigned char lfn_6;
unsigned char lfnhi_6;

unsigned char lfn_7;
unsigned char lfnhi_7;

unsigned char lfn_8;
unsigned char lfnhi_8;

unsigned char lfn_9;
unsigned char lfnhi_9;

unsigned char lfn_10;
unsigned char lfnhi_10;

unsigned char lfn_11;
unsigned char lfnhi_11;

unsigned char clusterlo[2]; /*fat12,fat16,fat32*/

unsigned char lfn_12;
unsigned char lfnhi_12;

unsigned char lfn_13;
unsigned char lfnhi_13;

} F_LFN;

#define F_ATTR_LFN      (F_ATTR_VOLUME|F_ATTR_SYSTEM|F_ATTR_HIDDEN|F_ATTR_READONLY)

#endif /*F_LONGFILENAME */

#define F_CLUSTER_FREE	   ((unsigned long)0x00000000)
#define F_CLUSTER_RESERVED ((unsigned long)0x0ffffff0)
#define F_CLUSTER_BAD	   ((unsigned long)0x0ffffff7)
#define F_CLUSTER_LAST	   ((unsigned long)0x0ffffff8)
#define F_CLUSTER_LASTF32R ((unsigned long)0x0fffffff)

#define F_ST_MISSING   0x00000001
#define F_ST_CHANGED   0x00000002
#define F_ST_WRPROTECT 0x00000004

typedef struct {
	unsigned long sector;  /*start sector*/
	unsigned long num;	   /*number of sectors*/
} F_SECTOR;


typedef struct {
	unsigned long abspos;
	unsigned long filesize;
	unsigned long startcluster;
	unsigned long prevcluster;
	unsigned long relpos;
	unsigned char modified;
	unsigned char mode;
	unsigned char _tdata[F_SECTOR_SIZE];
	F_POS pos;
	F_POS dirpos;
} F_FILE;

#if F_GETFREESPACE_QUICK || (FATBITFIELD_ENABLE && (F_FAT16 || F_FAT32))
typedef struct {
  unsigned long free;
  unsigned long used;
  unsigned long bad;
} F_CSPACE;
#endif

typedef struct {
  unsigned char state;

  F_BOOTRECORD bootrecord;

  F_SECTOR firstfat;

  F_SECTOR root;
  F_SECTOR _tdata;

  unsigned char fat[F_SECTOR_SIZE];
  unsigned long fatsector;

#if F_WRITING
  unsigned char fatmodified;
  unsigned long lastalloccluster;
#endif

  unsigned char direntry[F_SECTOR_SIZE];
  unsigned long direntrysector;

#if F_DIRECTORIES
  char cwd[F_MAXPATH];		/*current working folder in this volume*/
#endif
  unsigned char mediatype;
  unsigned long maxcluster;

#if F_GETFREESPACE_QUICK || (FATBITFIELD_ENABLE && (F_FAT16 || F_FAT32))
  F_CSPACE cspace;
  unsigned char cspace_valid;
#endif
#if FATBITFIELD_ENABLE && (F_FAT16 || F_FAT32)
  unsigned char *fatbitfield;
#endif
} F_VOLUME;

#if F_FAT32
enum {
F_UNKNOWN_MEDIA,
F_FAT12_MEDIA,
F_FAT16_MEDIA,
F_FAT32_MEDIA
};
#else
enum {
F_UNKNOWN_MEDIA,
F_FAT12_MEDIA,
F_FAT16_MEDIA
};
#endif



#define F_FILE_CLOSE	0x00
#define F_FILE_RD	0x01
#define F_FILE_WR	0x02
#define F_FILE_A	0x04
#define F_FILE_RDP	0x08
#define F_FILE_WRP	0x10
#define F_FILE_AP	0x20

#define F_FILE_CL	0x80	/* cluster list write mode */

enum {
/*  0 */ F_STATE_NONE,
/*  1 */ F_STATE_NEEDMOUNT,
/*  2 */ F_STATE_WORKING
};

enum {
/*  0 */ F_NO_ERROR,
/*  1 */ F_ERR_RESERVED_1,
/*  2 */ F_ERR_NOTFORMATTED,
/*  3 */ F_ERR_INVALIDDIR,  
/*  4 */ F_ERR_INVALIDNAME,
/*  5 */ F_ERR_NOTFOUND,
/*  6 */ F_ERR_DUPLICATED,
/*  7 */ F_ERR_NOMOREENTRY,
/*  8 */ F_ERR_NOTOPEN,
/*  9 */ F_ERR_EOF,
/* 10 */ F_ERR_RESERVED_2,
/* 11 */ F_ERR_NOTUSEABLE,
/* 12 */ F_ERR_LOCKED,
/* 13 */ F_ERR_ACCESSDENIED,
/* 14 */ F_ERR_NOTEMPTY,
/* 15 */ F_ERR_INITFUNC,
/* 16 */ F_ERR_CARDREMOVED,
/* 17 */ F_ERR_ONDRIVE,
/* 18 */ F_ERR_INVALIDSECTOR,
/* 19 */ F_ERR_READ,
/* 20 */ F_ERR_WRITE,
/* 21 */ F_ERR_INVALIDMEDIA,
/* 22 */ F_ERR_BUSY,
/* 23 */ F_ERR_WRITEPROTECT,
/* 24 */ F_ERR_INVFATTYPE,
/* 25 */ F_ERR_MEDIATOOSMALL,
/* 26 */ F_ERR_MEDIATOOLARGE,
/* 27 */ F_ERR_NOTSUPPSECTORSIZE,
/* 28 */ F_ERR_ALLOCATION
};

/* definitions for ctime */

#define F_CTIME_SEC_SHIFT		0
#define F_CTIME_SEC_MASK	0x001f	/*0-30 in 2seconds*/
#define F_CTIME_MIN_SHIFT		5
#define F_CTIME_MIN_MASK	0x07e0	/*0-59 */
#define F_CTIME_HOUR_SHIFT		11
#define F_CTIME_HOUR_MASK	0xf800	/*0-23*/

/* definitions for cdate */

#define F_CDATE_DAY_SHIFT		0
#define F_CDATE_DAY_MASK	0x001f	/*0-31*/
#define F_CDATE_MONTH_SHIFT	5
#define F_CDATE_MONTH_MASK	0x01e0	/*1-12*/
#define F_CDATE_YEAR_SHIFT		9
#define F_CDATE_YEAR_MASK	0xfe00	/*0-119 (1980+value)*/

typedef struct {
   unsigned long total;
   unsigned long free;
   unsigned long used;
   unsigned long bad;
} F_SPACE;

enum {
F_SEEK_SET, /*Beginning of file*/
F_SEEK_CUR, /*Current position of file pointer*/
F_SEEK_END  /*End of file*/
};

#if F_GETVERSION
char *fn_getversion(void);
#define f_getversion fn_getversion
#endif

unsigned char fn_initvolume(void);
#define f_initvolume fn_initvolume
unsigned char fn_delvolume(void);
#define f_delvolume fn_delvolume

#if (F_FORMATTING && F_WRITING)
unsigned char fn_hardformat(unsigned char fattype);
unsigned char fn_format(unsigned char fattype);
#define f_hardformat(fattype) fn_hardformat(fattype)
#define f_format(fattype) fn_format(fattype)
#endif

unsigned char fn_getcwd(char *buffer, unsigned char maxlen, char root );
#define f_getcwd(buffer,maxlen) fn_getcwd(buffer,maxlen,1)

#if F_GETFREESPACE
unsigned char fn_getfreespace(F_SPACE *pspace);  
#define f_getfreespace fn_getfreespace
#endif


#if F_CHDIR
unsigned char fn_chdir(const char *dirname);
#define f_chdir(dirname) fn_chdir(dirname)
#endif
#if (F_MKDIR && F_WRITING)
unsigned char fn_mkdir(const char *dirname);
#define f_mkdir(dirname) fn_mkdir(dirname)
#endif
#if (F_RMDIR && F_WRITING)
unsigned char fn_rmdir(const char *dirname);
#define f_rmdir(dirname) fn_rmdir(dirname)
#endif

#if F_FINDING
unsigned char fn_findfirst(const char *filename,F_FIND *find);
unsigned char fn_findnext(F_FIND *find);
#define f_findfirst(filename,find) fn_findfirst(filename,find)
#define f_findnext(find) fn_findnext(find)
#endif

#if (F_RENAME && F_WRITING)
unsigned char fn_rename(const char *filename, const char *newname);
#define f_rename(filename,newname) fn_rename(filename,newname)
#endif

#if F_FILELENGTH
long fn_filelength(const char *filename);
#define f_filelength(filename) fn_filelength(filename)
#endif

unsigned char fn_close(F_FILE *filehandle);
F_FILE *fn_open(const char *filename,const char *mode);

long fn_read(void *buf,long size,long _size_t,F_FILE *filehandle);

#if (F_WRITE && F_WRITING && F_FLUSH)
unsigned char fn_flush(F_FILE *f);
#define f_flush(filehandle) fn_flush(filehandle)
#endif

#if (F_WRITE && F_WRITING)
long fn_write(const void *buf,long size,long _size_t,F_FILE *filehandle);
#define f_write(buf,size,_size_t,filehandle) fn_write(buf,size,_size_t,filehandle)
#endif

#if F_SEEK
unsigned char fn_seek(F_FILE *filehandle,long offset,unsigned char whence);
#define f_seek(filehandle,offset,whence) fn_seek(filehandle,offset,whence)
#endif

#if F_TELL
long fn_tell(F_FILE *filehandle);
#define f_tell(filehandle) fn_tell(filehandle)
#endif
#if F_GETC
int fn_getc(F_FILE *filehandle);
#define f_getc(filehandle) fn_getc(filehandle)
#endif
#if (F_PUTC && F_WRITING)
int fn_putc(int ch,F_FILE *filehandle);
#define f_putc(ch,filehandle) fn_putc(ch,filehandle)
#endif
#if F_REWIND
unsigned char fn_rewind(F_FILE *filehandle);
#define f_rewind(filehandle) fn_rewind(filehandle)
#endif
#if F_EOF
unsigned char fn_eof(F_FILE *filehandle);
#define f_eof(filehandle) fn_eof(filehandle)
#endif

#if F_GETTIMEDATE
unsigned char fn_gettimedate(const char *filename,unsigned short *pctime,unsigned short *pcdate);
#define f_gettimedate(filename,pctime,pcdate) fn_gettimedate(filename,pctime,pcdate)
#endif
#if (F_SETTIMEDATE && F_WRITING)
unsigned char fn_settimedate(const char *filename,unsigned short ctime,unsigned short cdate);
#define f_settimedate(filename,ctime,cdate) fn_settimedate(filename,ctime,cdate)
#endif
#if (F_DELETE && F_WRITING)
unsigned char fn_delete(const char *filename);
#define f_delete(filename) fn_delete(filename)
#endif

#if F_GETATTR
unsigned char fn_getattr(const char *filename,unsigned char *attr);
#define f_getattr(filename,attr) fn_getattr(filename,attr)
#endif

#if (F_SETATTR && F_WRITING)
unsigned char fn_setattr(const char *filename,unsigned char attr);
#define f_setattr(filename,attr) fn_setattr(filename,attr)
#endif

#if F_GETLABEL
unsigned char fn_getlabel(char *label, unsigned char len);
#define f_getlabel(label,len) fn_getlabel(label,len)
#endif

#if (F_SETLABEL && F_WRITING)
unsigned char fn_setlabel(const char *label);
#define f_setlabel(label) fn_setlabel(label)
#endif

#if (F_WRITING && F_SETEOF)
unsigned char fn_seteof(F_FILE *,long);
#define f_seteof(file,filesize) fn_seteof(file,filesize)
#endif

#if (F_WRITING && F_TRUNCATE)
F_FILE *fn_truncate(const char *,long);
#define f_truncate(filename,filesize) fn_truncate(filename,filesize)
#endif


#define f_close(filehandle) fn_close(filehandle)
#define f_open(filename,mode) fn_open(filename,mode)
#define f_read(buf,size,_size_t,filehandle) fn_read(buf,size,_size_t,filehandle)

#define _SET_BIT(array,pos)	(array[(pos)>>3]|=(1U<<((pos)&7)))
#define _CLEAR_BIT(array,pos)	(array[(pos)>>3]&=~(1U<<((pos)&7)))
#define _GET_BIT(array,pos)	(array[(pos)>>3]&(1U<<((pos)&7)))

/****************************************************************************
 *
 * end of fat_thin.h
 *
 ***************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /*_FAT_THIN_H_*/

