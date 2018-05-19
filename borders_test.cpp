#include <iostream>
#include "borders.h"
#include "err.h"

int
main(){
  try {
    const char *country = "FI";
    const char *file = "data/CNTR_RG_03M_2013_4326.geojson";
    dMLine ML = get_border(file, country);
    std::cerr << country << ": " << ML.size() << " lines\n";
  }
  catch (Err e){
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
