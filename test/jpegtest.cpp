#include <stdlib.h>
#include <stdio.h>

#include <string>
#include "jpeg.h"

int main(int argc, char **argv) {
  std::string filename("P1000761.JPG");
  if (argc > 1)
    filename = argv[1];
  // printf("Testing with loadall=false\n");
  // Jpeg j(filename.c_str(), false);
  printf("Testing with loadall=true\n");
  Jpeg j2(filename.c_str(), true);
  j2.Save("testoutput.jpg");
  // j2.Corrupt(0,0,100);
  // j2.Save("testcorrupted.jpg");
}