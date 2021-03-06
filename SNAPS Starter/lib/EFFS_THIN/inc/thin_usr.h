#ifndef _THIN_USR_H
#define _THIN_USR_H

/****************************************************************************
**
**		$Source: C:\\RCS\\D\\Pumpkin\\CubeSatKit\\HCC-Embedded\\EFFS-THIN\\src\\thin_usr.h,v $
**		$Author: aek $
**		$Revision: 3.7 $
**		$Date: 2012-12-10 15:50:58-08 $
**
** 		EFFS-THIN global configuration file for CubeSat Kit
**
**		USERS: DO NOT EDIT THIS FILE! DO NOT EDIT THE INCLUDED FILES, EITHER!
**
**   	For use with HCC-Embedded's EFFS-THIN software, exclusively on Pumpkin, 
** Inc.'s CubeSat Kit hardware.
**
**    Derived from HCC-Embedded's own thin_usr.h, this file replaces the
** thin_usr.h supplied by HCC-Embedded and then causes the inclusion of one
** header file based on defined symbols passed to it. This way, the various
** defines are guaranteed to match between when the library was originally 
** built at/by Pumpkin, Inc. and when the CubeSat Kit customer uses it.
**
** 		Library names ('1', '2', etc.) are assigned with no rhyme or reason 
** other than that they indicate the order in which they were created. 
** Individual library settings can ** only ** be derived by looking at the 
** library's associated include file.
**
** 		Libraries can be "thin" or "superthin" builds -- the library number 
** does not reveal this information. 
**
**		uCdrive is not supported. 
**
**		User include path should always be set to:
**
**			/Pumpkin/CubeSatKit/HCC-Embedded/EFFS-THIN/src
** 
**		Requests for new libraries should be sent to Pumpkin, Inc.
**
****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


#if   (CSK_EFFS_THIN_LIB == 1)
#include "csk_effs_thin-1.h"
#elif (CSK_EFFS_THIN_LIB == 2)
#include "csk_effs_thin-2.h"
#elif (CSK_EFFS_THIN_LIB == 3)
#include "csk_effs_thin-3.h"
#elif (CSK_EFFS_THIN_LIB == 4)
#include "csk_effs_thin-4.h"
#elif (CSK_EFFS_THIN_LIB == 5)
#include "csk_effs_thin-5.h"
#elif (CSK_EFFS_THIN_LIB == 6)
#include "csk_effs_thin-6.h"
#else
#error Invalid CubeSat Kit EFFS-THIN library specified ... see defined symbol CSK_EFFS_THIN_LIB.
#endif

/*#define INTERNAL_MEMFN*/
#ifdef INTERNAL_MEMFN
#define __memcpy(d,s,l)	_f_memcpy(d,s,l)
#define __memset(d,c,l)	_f_memset(d,c,l)
#define __size_t		int
#else
#include <string.h>
#define __memcpy(d,s,l)	memcpy(d,s,l)
#define __memset(d,c,l)	memset(d,c,l)
#define __size_t		size_t
#endif


#ifdef __cplusplus
}
#endif

#endif

