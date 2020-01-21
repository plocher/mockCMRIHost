/*
 * mockCMRIHost
 */

#include <cpCMRI.h>
#include <mockCMRIHost.h>
#include <elapsedMillis.h>

/**
 * Take a string description of a packet and stuff the corresponding
 * real packet into the "to be read from" buffer
 * @param str
 */
void mockCMRIHost::preload_data(const char *str) {
    /*
     * Format:
     * <c=type> <#=address> '[' <#> [',' ...] ']'
     * I3[0x42,0xFF]
     */
    const char *s = str;
    byte buffer[128];
    int bpos = 0;

    for (int i = 0; i < strlen(s); ++i) {
	char *nexts;
	buffer[bpos++] = CMRI_Packet::SYN;
	buffer[bpos++] = CMRI_Packet::SYN;
	buffer[bpos++] = CMRI_Packet::STX;
	byte ptype = *s++;
	unsigned long int paddr = strtoul(s, &nexts, 0) + 'A';
	buffer[bpos++] = (byte)paddr;
	buffer[bpos++] = ptype;

	s = nexts;
	if (*s++ != '[') { return; }
	while (*s != ']') {
	    unsigned long int val = strtoul(s, &nexts, 0);
	    if (  (val == CMRI_Packet::DLE)
	       || (val == CMRI_Packet::SYN)
	       || (val == CMRI_Packet::STX)
	       || (val == CMRI_Packet::ETX) ) {
		buffer[bpos++] = CMRI_Packet::DLE;
	    }
	    buffer[bpos++] = val;
	    s = nexts;
	    if (*s == ',') { s++;}  // skip commas
	    else if ( *s != ']') { return; }
	}
	buffer[bpos++] = CMRI_Packet::ETX;
    }
    for (int i = 0; i < bpos; i++) {
	pushback(buffer[i]);
    }
}

