#include "libmng.h"
#include "base/logging.h"

int main(int argc, char* argv[]) {
  LOG(ERROR) << mng_version_so;
  return 0;
}
