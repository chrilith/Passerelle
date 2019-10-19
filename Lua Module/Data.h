#pragma once

inline int Read8(const char *ptr) {
	return ptr[0] & 0xFF;
}

inline int Read16(const char *ptr) {
	short p1 = Read8(ptr + 0);
	short p2 = Read8(ptr + 1);

	return (p1 << 8) + p2;
}

inline int Read32(const char *ptr) {
	int p1 = Read8(ptr + 0);
	int p2 = Read8(ptr + 1);
	int p3 = Read8(ptr + 2);
	int p4 = Read8(ptr + 3);

	return (p1 << 24) + (p2 << 16) + (p3 << 8) + p4;
}

inline void Write8(char *ptr, int value) {
	ptr[0] = value & 0xFF;
}

inline void Write16(char *ptr, int value) {
	Write8(ptr + 0, value >> 8);
	Write8(ptr + 1, value);
}

inline void Write32(char *ptr, int value) {
	Write8(ptr + 0, value >> 24);
	Write8(ptr + 1, value >> 16);
	Write8(ptr + 2, value >> 8);
	Write8(ptr + 3, value);
}
