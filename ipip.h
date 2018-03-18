// ipip.h from https://www.ipip.net/download.html
//
// 感谢 北京天特信科技有限公司
//
#ifndef _IPIP_H_
#define _IPIP_H_

int init(const char *ipdbx);
int find(const char *ip, char *result, int len);
int find_u(uint32_t ip, char *result, int len);

#endif				//_IPIP_H_
