// ipip.c from https://www.ipip.net/download.html
// rewrite from https://github.com/ipipdotnet/datx-python by james@ustc.edu.cn
// 感谢 北京天特信科技有限公司
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define L2I(a,b,c,d) ((a & 0xFF) | ((b << 8) & 0xFF00) | ((c << 16) & 0xFF0000) | ((d << 24) & 0xFF000000))
#define B2I(a,b,c,d) ((d & 0xFF) | ((c << 8) & 0xFF00) | ((b << 16) & 0xFF0000) | ((a << 24) & 0xFF000000))

struct {
	unsigned char *data;
	uint32_t indexsize;
} ipip;

int init(const char *ipdbx)
{
	if (ipip.indexsize)
		return 0;
	FILE *file = fopen(ipdbx, "rb");
	if (file == NULL)
		return 0;
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	ipip.data = (unsigned char *)malloc(size * sizeof(unsigned char));
	if (ipip.data == NULL)
		return 0;
	size_t r = fread(ipip.data, sizeof(unsigned char), (size_t) size, file);

	if (r == 0)
		return 0;

	fclose(file);

	ipip.indexsize = B2I(ipip.data[0], ipip.data[1], ipip.data[2], ipip.data[3]);

	return 1;
}

int find_u(uint32_t val, char *result, int len)
{
	uint32_t low, mid, high;

	if (ipip.indexsize == 0) {
		snprintf(result, len, "datx file not load");
		return 0;
	}

	low = mid = 0;
	high = (ipip.indexsize - 262144 - 262148) / 9 - 1;

	for (; low <= high;) {
		mid = (low + high) / 2;
		uint32_t pos = mid * 9 + 262148;
		uint32_t start = 0;
		if (mid > 0) {
			uint32_t pos1 = (mid - 1) * 9 + 262148;
			start = B2I(ipip.data[pos1], ipip.data[pos1 + 1], ipip.data[pos1 + 2], ipip.data[pos1 + 3]) + 1;
		} else
			start = 0;
		uint32_t end = B2I(ipip.data[pos], ipip.data[pos + 1], ipip.data[pos + 2], ipip.data[pos + 3]);

		if (val < start)
			high = mid - 1;
		else if (val > end)
			low = mid + 1;
		else {
			uint32_t off = L2I(ipip.data[pos + 4], ipip.data[pos + 5],
					   ipip.data[pos + 6], 0);
			len = ipip.data[pos + 7] * 256 + ipip.data[pos + 8];

			off = off - 262144 + ipip.indexsize;
			memcpy(result, ipip.data + off, len);
			result[len] = 0;
			return 0;
		}
	}

	snprintf(result, len, "IP NOT FOUND");
	return 0;
}

int find(const char *ip, char *result, int len)
{
	uint ips[4];
	int num = sscanf(ip, "%d.%d.%d.%d", &ips[0], &ips[1], &ips[2], &ips[3]);
	if (num != 4) {
		snprintf(result, len, "IP format error");
		return 0;
	}
	uint32_t val = B2I(ips[0], ips[1], ips[2], ips[3]);
	return find_u(val, result, len);
}
