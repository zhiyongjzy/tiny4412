#include "config.h"
#include <stdarg.h> //直接取交叉编译器对应头文件

static void str_reverse(char *str)
{
	char *head = str;
	char *tail = 0;
	for (tail = str; *(tail + 1); ++tail) {}

	char tmp = 0;
	for (; head < tail; ++head, --tail) {
		tmp = *head;
		*head = *tail;
		*tail = tmp;
	}
}

enum {
	UNSIGNED = 0,
	SIGNED = 1,
};

#define BUF_LEN 33
static char str[BUF_LEN];
const char tab[] = "0123456789abcdef";
/* 数字转成字符串并直接输出到串口
 * @n: 被转换的目标数字
 * @base: 进制
 * @is_signed: 是否是有符号的类型
 * return: 转换成字符串后的字节数
 */
static size_t itoa(int n, size_t base, size_t is_signed)
{
	size_t count = 0;

	char *p = str;

	int SIGNED_FLG = UNSIGNED;

	//确定最终结果是否是负数
	if ((base == 10) && (is_signed) && (n < 0)) {
		SIGNED_FLG = SIGNED;
		n *= -1;
	}

	//排除n == 0的情况，因为0会导致下面的for循环直接退出
	if (!n) {
		*p++ = '0';
		count++;
	}

	for (; n; p++, n /= base, count++) {
		*p = tab[n % base];
	}

	if (SIGNED_FLG == SIGNED) {
		*p++ = '-';
		count++;
	}
	*p = '\0';

	str_reverse(str);

	return count;
}

int printk(const char *fmt, ...)
{
	//上一次遍历到的字符是不是%，normal表示不是%，fmt表示是%。该标记决定了当前遍历到的字符是格式符还是正常字符
	enum {
		NORMAL, //非'%'
		FMT, //'%'
	} FMT_FLG;

	int count = 0; //统计最终打印的字符数量
	int cur_count = 0;

	FMT_FLG = NORMAL;

	va_list ap;
	va_start(ap, fmt);

	for (; *fmt; ++fmt) {
		switch (*fmt) {
		case '%':
			if (FMT_FLG == NORMAL) {
				FMT_FLG = FMT;
			} else { //连续两个%
				uart_print_ch(*fmt);
				++count;
				FMT_FLG = NORMAL;
			}
			break;
		case 'c':
			//排除普通的字符
			if (FMT_FLG == NORMAL) {
				uart_print_ch(*fmt);
			} else {
				uart_print_ch((char)va_arg(ap, int));
				FMT_FLG = NORMAL;
			}
			++count;
			break;
		case 's':
			if (FMT_FLG == NORMAL) {
				uart_print_ch(*fmt);
				++count;
			} else {
				char *str = va_arg(ap, char*);
				uart_print_str(str);
				for (; *str; str++) {
					++count;
				}
				FMT_FLG = NORMAL;
			}
			break;
		case 'd':
			if (FMT_FLG == NORMAL) {
				uart_print_ch(*fmt);
				++count;
			} else {
				cur_count = itoa(va_arg(ap, int), 10, SIGNED);
				count += cur_count;
				FMT_FLG = NORMAL;
			}
			break;
		case 'u':
			if (FMT_FLG == NORMAL) {
				uart_print_ch(*fmt);
				++count;
			} else {
				cur_count = itoa(va_arg(ap, int), 10, UNSIGNED);
				count += cur_count;
				FMT_FLG = NORMAL;
			}
			break;
		case 'b':
			if (FMT_FLG == NORMAL) {
				uart_print_ch(*fmt);
				++count;
			} else {
				cur_count = itoa(va_arg(ap, int), 2, UNSIGNED);
				uart_print_str(str);
				count += cur_count;
				FMT_FLG = NORMAL;
			}
			break;
		case 'x':
			if (FMT_FLG == NORMAL) {
				uart_print_ch(*fmt);
				++count;
			} else {
				cur_count = itoa(va_arg(ap, int), 16, UNSIGNED);
				count += cur_count;
				FMT_FLG = NORMAL;
			}
			break;
		default:
			if (FMT_FLG == NORMAL) {
				uart_print_ch(*fmt);
				++count;
			} else {
				FMT_FLG = NORMAL;
			}
			break;
		}
	}

	va_end(ap);

	return count;
}
