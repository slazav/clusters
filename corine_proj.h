#ifndef CORINE_PROJ_H
#define CORINE_PROJ_H

#include <proj_api.h>
#include "pt.h"
#include <fstream>

/// read tfw-file and make conversions between corine map and geo coordinates

#define SRC_PARS "+proj=lonlat +datum=WGS84"
#define DST_PARS "+proj=laea +lon_0=10 +lat_0=52"\
                 " +x_0=4321000 +y_0=3210000 +ellps=GRS80 +towgs84=0,0,0"

class corine_proj {

  projPJ pj_src, pj_dst;
  double a[6]; // tfw parameters

  public:

  corine_proj(const char *tfw_file){

    // read tfw file
    std::ifstream tfw(tfw_file);
    for (int i=0; i<6; i++){
      if (tfw.bad()) throw Err() << "can't read tfw-file: " << tfw_file;
      tfw >> a[i];
    }
    if (a[1] != 0 || a[2] != 0)
      throw Err() << "non-diagonal elements in tfw not supported: " << tfw_file;

    // build projections
    pj_src = pj_init_plus(SRC_PARS);
    if (!pj_src)
      throw Err() << "Can't create projection \""
                  << SRC_PARS << "\": " << pj_strerrno(pj_errno);

    pj_dst = pj_init_plus(DST_PARS);
    if (!pj_dst){
      pj_free(pj_src);
      throw Err() << "Can't create projection \""
                  << DST_PARS << "\": " << pj_strerrno(pj_errno);
    }
  }

  ~corine_proj(){
    if (pj_dst) pj_free(pj_dst);
    if (pj_src) pj_free(pj_src);
  }

  void map2wgs(double & x, double & y) const {
    x = a[0]*x + a[4]; y = a[3]*y + a[5];
    if (pj_transform(pj_dst, pj_src, 1, 1, &x, &y, NULL)!=0)
      throw Err() << "Can't convert coordinates: " << pj_strerrno(pj_errno);
    x/=DEG_TO_RAD; y/=DEG_TO_RAD;
  }

  void wgs2map(double & x, double & y) const {
    x*=DEG_TO_RAD; y*=DEG_TO_RAD;
    if (pj_transform(pj_src, pj_dst, 1, 1, &x, &y, NULL)!=0)
      throw Err() << "Can't convert coordinates: " << pj_strerrno(pj_errno);
    x=(x-a[4])/a[0]; y=(y-a[5])/a[3];
  }
  void map2wgs(dPt & p) const { map2wgs(p.first, p.second); }
  void wgs2map(dPt & p) const { wgs2map(p.first, p.second); }

  void wgs2map(dLine & l) const { for (dLine::iterator p=l.begin(); p!=l.end(); p++) wgs2map(p->first, p->second); }
  void map2wgs(dLine & l) const { for (dLine::iterator p=l.begin(); p!=l.end(); p++) map2wgs(p->first, p->second); }

  void wgs2map(dMLine & m) const { for (dMLine::iterator l=m.begin(); l!=m.end(); l++) wgs2map(*l); }
  void map2wgs(dMLine & m) const { for (dMLine::iterator l=m.begin(); l!=m.end(); l++) map2wgs(*l); }

};

#endif
