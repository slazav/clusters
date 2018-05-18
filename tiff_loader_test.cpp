#include "tiff_loader.h"
#include "err.h"
#include <iostream>

int
main() {

  try{
    tiff_loader L("data/2012-250/g250_clc12_V18_5.tif");



//    int x0=0, y0=0, s=20, w=L.get_w()/s, h=L.get_h()/s;
    int x0=1359*20, y0=534*20, s=1, w=110*20, h=110*20;

    std::cout << "P5\n" << w << " " << h << "\n255\n";
    for (int y=y0; y<y0+h*s; y+=s){
      for (int x=x0; x<x0+w*s; x+=s){
        std::cout << L.get(x,y);
      }
    }


  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }

}