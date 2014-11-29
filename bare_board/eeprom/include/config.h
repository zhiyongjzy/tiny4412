/*************************************************************************
	> File Name: typedef.h
	> Created Time: 2014年11月28日 星期五 18时15分11秒
 ************************************************************************/
#pragma once


typedef unsigned int u32;
typedef signed int s32;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned char u8;
typedef signed long s8;
typedef s32 ssize_t;

typedef unsigned int size_t;


#define module_init(pfunc) \
	static void (*p_##pfunc)() __attribute__((__section__(".initcall"))) = pfunc


#define htons(n) ((u16)((n) >> 8 | (n) << 8))
#define htonl(n) ((n) >> 24 | (((n) >> 8) & (0xff << 8)) | (((n) << 8) & (0xff << 16)) | (n) << 24)
#define ntohs(n) htons(n)
#define ntohl(n) htonl(n)

