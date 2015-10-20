

/*´óĞ¡¶Ë×ª»»*/
#define ENDIAN_SWAP32(data)  	((data >> 24) | ((data & 0x00ff0000) >> 8) | ((data & 0x0000ff00) << 8) | ((data & 0x000000ff) << 24)) 