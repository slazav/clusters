#include <iostream>
#include "pt.h"
#include <cmath>

double dist_fn(const dPt & p1, const dPt & p2){
//  if (!proj) return 0;
  dPt p1a(p1), p2a(p2);
//  proj->map2wgs(p1a);
//  proj->map2wgs(p2a);

  // see: https://www.movable-type.co.uk/scripts/latlong.html
  double R = 6371e3; // metres
  double lat1 = p1.second * M_PI/180;
  double lat2 = p2.second * M_PI/180;
  double dlat = lat2-lat1;
  double dlon = (p2.first-p1.first) * M_PI/180;

  double sdlat = sin(dlat/2);
  double sdlon = sin(dlon/2);
  double a = sdlat*sdlat + cos(lat1)*cos(lat2)*sdlon*sdlon;
  double c = 2.0 * atan2(sqrt(a), sqrt(1-a));
  return R * c;
}

int
main(){
  dPt p1(37.40295, 55.80379), p2(37.39257,55.72972);
  std::cout << dist_fn(p1,p2)/1000;
}
