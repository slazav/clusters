#ifndef BORDERS_H
#define BORDERS_H

#include "pt.h"
#include "err.h"
#include "cache.h"


/// read geojson file with country borders from
/// http://ec.europa.eu/eurostat/web/gisco/geodata/reference-data/administrative-units-statistical-units/countries
/// jansson api: http://jansson.readthedocs.io/en/2.8/apiref.html
/// geojson: http://geojson.org/geojson-spec.html

/// get a country border (selected by 2-letter code) as a dMLine
dMLine get_border(const char *rg_file, const char *country);

// border bounding box (for empty border returns false);
bool border_bbox(const dMLine & brd, dPt & p1, dPt & p2);

// remove border segments outside p1-p2 rectangle
void border_crop(dMLine & brd, const dPt & p1, const dPt & p2);

/// test if integer points are inside the border
class brd_tester {
  Cache<int, std::vector<int> > cache;
  std::vector<dPt> sb,se;
  std::vector<double> ss;

  /// get integer crossings for a given y coordinate
  std::vector<int> get_cr(int y);

  public:
    /// constructor
    brd_tester(const dMLine & brd);

    /// test if the points is inside the border
    bool test(const int x, const int y);
};

#endif
