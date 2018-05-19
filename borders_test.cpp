#include <iostream>
#include "borders.h"
#include "err.h"

int
main(){
  try {
    const char *country = "FR";
    const char *file = "data/CNTR_RG_03M_2013_4326.geojson";
    dMLine ML = get_border(file, country);
    std::cout << country << ": " << ML.size() << " lines\n";

    dPt p1, p2;
    if (!border_bbox(ML, p1, p2)) return 1;
    std::cout << "x: " << p1.first << " " << p2.first << "\n";
    std::cout << "y: " << p1.second << " " << p2.second << "\n";
  }
  catch (Err e){
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }
  return 0;
}
