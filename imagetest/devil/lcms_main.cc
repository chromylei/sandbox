#include "lcms2.h"
#include "base/logging.h"

int main(int argc, char* argv[]) {
  cmsHPROFILE hInProf = cmsOpenProfileFromFile("", "r");
  LOG(ERROR) << "....";
  return 0;
}
