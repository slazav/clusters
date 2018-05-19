#include "tiff_loader.h"
#include "err.h"
#include "corine_proj.h"
#include "borders.h"
#include <iostream>

int
main() {

  try{
    tiff_loader L("data/g250_clc12_V18_5.tif");

    corine_proj P("data/g250_clc12_V18_5.tfw");
    dPt p(24.825002,60.186950);
    P.wgs2map(p);

    const char *country = "FI";
    const char *file = "data/CNTR_RG_03M_2013_4326.geojson";
    dMLine brd = get_border(file, country);

    P.wgs2map(brd);
    dPt p1, p2;
    if (!border_bbox(brd, p1, p2)) return 1;

    std::cerr << "x: " << p1.first << " - " << p2.first << "\n";
    std::cerr << "y: " << p1.second << " - " << p2.second << "\n";
    std::cerr << "p: " << p.first << " - " << p.second << "\n";

//    int x0=0, y0=0, s=20, w=L.get_w()/s, h=L.get_h()/s;
//    int x0=1359*20, y0=534*20, s=1, w=110*20, h=110*20;
    int x0=p1.first, y0=p1.second, s=1, w=p2.first-p1.first, h=p2.second-p1.second;

    brd_tester test_brd(brd);

    std::cout << "P5\n" << w << " " << h << "\n255\n";
    for (int y=y0; y<y0+h*s; y+=s){
      for (int x=x0; x<x0+w*s; x+=s){
        char c = L.get(x,y);
        if (!test_brd.test(x,y)) c=255-c/2;
        if (x==(int)p.first && y==(int)p.second) c = 255;
        std::cout << c;
      }
    }



  }
  catch (Err e) {
    std::cerr << "Error: " << e.str() << "\n";
    return 1;
  }

}