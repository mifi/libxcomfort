#include <stdlib.h>

#include "lib_crc/lib_crc.h"

unsigned short calculate_crc(char *buf, size_t len) {
	unsigned short crc = 0;

	size_t i=0;
	for (i=0; i<len; i++) {
		crc = update_crc_kermit(crc, buf[i]);
	}
		
	return crc;
}
