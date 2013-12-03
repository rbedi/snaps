/**************************************************************************
**
**		$Source: C:\\RCS\\D\\Pumpkin\\CubeSatKit\\HCC-Embedded\\EFFS-THIN\\src\\csk_effs_thin-4.h,v $
**		$Author: aek $
**		$Revision: 3.3 $
**		$Date: 2011-03-23 21:55:14-08 $
**
**  	FAT super thin user settings
**
**  	Settings for CubeSat Kit EFFS-THIN library #4
**
**************************************************************************/

#ifndef CSK_EFFS_THIN_4
#define CSK_EFFS_THIN_4


#define F_GETVERSION		0			/*defines f_getversion*/
#define F_FORMATTING		0			/*defines f_hardformat*/
#define F_GETFREESPACE	0			/*defines f_getfreespace*/
#define F_CHDIR					0			/*defines f_chdir*/
#define F_MKDIR				  0			/*defines f_mkdir*/
#define F_RMDIR					0			/*defines f_rmdir*/
#define F_DELETE				1			/*defines f_delete*/
#define F_FILELENGTH		1			/*defines f_filelength*/
#define F_FINDING				1			/*defines f_findfirst, f_findnext*/
#define F_TELL					0			/*defines f_tell*/
#define F_GETC					0			/*defines f_getc*/
#define F_PUTC					0			/*defines f_putc*/
#define F_REWIND				0			/*defines f_rewind*/
#define F_EOF					  1			/*defines f_eof*/
#define F_FLUSH         0 	  /*defines f_flush*/
#define F_SEEK					1			/*defines f_seek*/
#define F_WRITE					1			/*defines f_write*/
#define F_DIRECTORIES		1			/*enables subdirectories*/
#define F_CHECKNAME			0			/*enables name checking*/
#define F_TRUNCATE			0			/*enable truncate command*/
#define F_SETEOF				0			/*enable seteof*/

#define F_WRITING				1			/*removes everything related to write*/

#define F_FAT12					0			/*enables FAT12 usage*/
#define F_FAT16					1			/*enables FAT16 usage*/
#define F_FAT32					0			/*enables FAT32 usage*/

#define F_MAXFILES      1 		/*maximum number of files -- superthin only allows a single file!*/

#define F_MAXPATH				14		/*considered unsigned char in thin version*/

#define F_CHANGE_NOTIFY	0 		/*CSK doesn't use uCdrive*/
#define F_CHANGE_MAX		4			/*(unused): maximum number of files for change notification*/

#include "fat_sthin/fat_sthin.h"

#endif /* include guard */
