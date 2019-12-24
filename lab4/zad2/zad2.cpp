#include <cstdio>

void fun() {
  unsigned char array2[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  printf("%x%x%x%x\n", array2[0], array2[1], array2[2], array2[3]);
  return;
}

int main(int argc, char* argv[]) {
  unsigned char array1[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xbb, 0xbb, 0xbb, 0xbb};
  fun();
  return 0;
}