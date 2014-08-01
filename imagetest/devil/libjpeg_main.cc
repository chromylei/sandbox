#include <stdlib.h>
#include <stdio.h>
#include "jpeglib.h"
#include "base/logging.h"

int main(int argc, char* argv[]) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr       jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  return 0;
}
