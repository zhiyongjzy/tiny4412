/*************************************************************************
	> File Name: typedef.h
	> Created Time: 2014年07月11日 星期五 15时15分11秒
 ************************************************************************/

#pragma once

typedef unsigned int u32;
typedef signed int s32;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned char u8;
typedef signed long s8;
typedef u32 size_t;
typedef s32 ssize_t;

#define module_init(pfunc) \
	static void (*p_##pfunc)() __attribute__((__section__(".initcall"))) = pfunc



int printk(const char *fmt, ...);
void led_test();
void leds_on(u8 n);
void request_irq(int irqno, void (*handler)());
void *memcpy(void *dest, const void *src, size_t count);
void uart_test();
void uart_print_ch(u8 ch);
void uart_print_str(const char *str);


/* mmu related */
void set_domain_all_manager();
void set_ttb(u32);

void set_cpsr_int_on();
