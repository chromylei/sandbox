#include "IL/ilu.h"

const char* kColorMapPath = "sbox/terrain/res/tex/colorm01.bmp";
int main(int argc, char* argv[]) {
  ILuint id;
  ilGenImages(1, &id);
  ilBindImage(id);
  return 0;
}
