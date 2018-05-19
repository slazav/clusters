#include <iostream>
#include <algorithm>
#include <jansson.h>
#include <cstring>
#include "err.h"
#include "borders.h"

/// read geojson file with country borders from
/// http://ec.europa.eu/eurostat/web/gisco/geodata/reference-data/administrative-units-statistical-units/countries
/// jansson api: http://jansson.readthedocs.io/en/2.8/apiref.html
/// geojson: http://geojson.org/geojson-spec.html

/// read a polygon from json into dMLine
void read_poly(json_t *J, dMLine & ML){
  size_t i,j;
  json_t *j_seg, *j_pt;
  json_array_foreach(J, i, j_seg) {
    dLine L;
    if (!json_is_array(j_seg)) throw Err() << "bad segment coordinates";
    json_array_foreach(j_seg, j, j_pt) {
      if (!json_is_array(j_pt)) throw Err() << "bad point coordinates";
      if (json_array_size(j_pt)!=2) throw Err() << "wrong number of point coordinates";
      json_t *jx =json_array_get(j_pt, 0);
      json_t *jy =json_array_get(j_pt, 1);
      if (!json_is_number(jx)) throw Err() << "coordinate expected";
      if (!json_is_number(jy)) throw Err() << "coordinate expected";
      L.push_back(dPt(json_number_value(jx), json_number_value(jy)));
    }
    ML.push_back(L);
  }
}

/// get a border for country (selected by 2-letter code) into dMLine
dMLine
get_border(const char *rg_file, const char *country){

    // load and parse geojson file
    json_error_t e;
    json_t *j_top = json_load_file(rg_file, 0, &e);

    //////////////////////
    // top level: object with type=FeatureCollection, crs, features fields
    if (!j_top)
      throw Err() << e.text;
    if (!json_is_object(j_top))
      throw Err() << "Reading Opt: a JSON object with string fields expected";

    json_t *j_fctype = json_object_get(j_top, "type");
    if (!j_fctype || !json_is_string(j_fctype)) throw Err() << "type string missing";
    if (strcasecmp(json_string_value(j_fctype), "FeatureCollection")!=0)
      throw Err() << "type=FeatureCollection expected: " << json_string_value(j_fctype) ;

    json_t *j_features = json_object_get(j_top, "features");
    if (!j_features || !json_is_array(j_features)) throw Err() << "features array missing";

    json_t *j_crs = json_object_get(j_top, "crs");
    if (!j_crs || !json_is_object(j_crs)) throw Err() << "crs object missing";

    //////////////////////
    // crs: object with type=name and properties
    json_t *j_crstype = json_object_get(j_crs, "type");
    if (!j_crstype || !json_is_string(j_crstype)) throw Err() << "type string missing";
    if (strcasecmp(json_string_value(j_crstype), "name")!=0)
      throw Err() << "type=name for CRS expected: " << json_string_value(j_crstype) ;

    json_t *j_crspr = json_object_get(j_crs, "properties");
    if (!j_crspr || !json_is_object(j_crspr)) throw Err() << "properties object missing";

    json_t *j_crsname = json_object_get(j_crspr, "name");
    if (!j_crsname || !json_is_string(j_crsname)) throw Err() << "name string missing";
    if (strcasecmp(json_string_value(j_crsname), "urn:ogc:def:crs:EPSG::4326")!=0)
      throw Err() << "unsupported CRS name: " << json_string_value(j_crsname);

    //////////////////////
    // for each feature
    size_t index;
    json_t *F;
    json_array_foreach(j_features, index, F) {
      if (!json_is_object(F))
        throw Err() << "feature: not an object";

      // each featuere is an object with type=Feature, id, geometry, properties
      json_t *j_ftype = json_object_get(F, "type");
      if (!j_ftype || !json_is_string(j_ftype)) throw Err() << "type string missing";
      const char *type = json_string_value(j_ftype); // "Feature"
      if (strcasecmp(json_string_value(j_ftype), "Feature")!=0) 
        throw Err() << "Feature type expected: " << json_string_value(j_ftype);

      json_t *j_fid = json_object_get(F, "id");
      if (!j_fid || !json_is_string(j_fid)) throw Err() << "bad id";
      const char *id = json_string_value(j_fid);
      if (strcasecmp(id, country)!=0) continue; // select country

      json_t *j_fprop = json_object_get(F, "properties");
      if (!j_fprop || !json_is_object(j_fprop)) throw Err() << "bad properties: " << id;
      // TODO: parse properties

      json_t *j_fgeom = json_object_get(F, "geometry");
      if (!j_fgeom || !json_is_object(j_fgeom))  throw Err() << "bad geometry: " << id;

      //////////////////////
      // each geometry is an object with type=Polygon or MultiPolygon and coordinates

      json_t *j_gtype = json_object_get(j_fgeom, "type");
      if (!j_gtype || !json_is_string(j_gtype))  throw Err() << "bad geometry type" << id;
      const char *gtype = json_string_value(j_gtype);

      json_t *j_gcoord = json_object_get(j_fgeom, "coordinates");
      if (!j_gcoord || !json_is_array(j_gcoord))  throw Err() << "bad geometry coordinates: " << id;

      // Coordinates of a Polygon are an array of LinearRing coordinate
      // arrays. The first element in the array represents the exterior
      // ring. Any subsequent elements represent interior rings (or holes).
      // Coordinates of a MultiPolygon are an array of Polygon coordinate arrays.
      // TODO: now I ignore all holes!
      dMLine ML;
      if (strcasecmp(gtype, "MultiPolygon")==0) {
        size_t i;
        json_t *j_c;
        json_array_foreach(j_gcoord, i, j_c) { read_poly(j_c, ML); }
      }
      else if (strcasecmp(gtype, "Polygon")==0) {
        read_poly(j_gcoord, ML);
      }
      else
        throw Err() << "Polygon or MultiPolygon expected: " << id << ": " << gtype;

      return ML;
    }
  return dMLine(); // no country found
}

// border bounding box (for empty border do nothing);
bool border_bbox(const dMLine & brd, dPt & p1, dPt & p2){
  if (brd.size()==0 || brd[0].size()==0) return false;
  p1 = p2 = brd[0][0];
  for (dMLine::const_iterator seg = brd.begin(); seg!=brd.end(); seg++) {
    for (dLine::const_iterator pt = seg->begin(); pt!=seg->end(); pt++) {
      if (p1.first > pt->first)  p1.first = pt->first;
      if (p2.first < pt->first)  p2.first = pt->first;
      if (p1.second > pt->second)  p1.second = pt->second;
      if (p2.second < pt->second)  p2.second = pt->second;
    }
  }
  return true;
}

// remove border points and segments outside p1-p2 rectangle
void border_crop(dMLine & brd, const dPt & p1, const dPt & p2){
  dMLine::iterator seg = brd.begin();
  while (seg!=brd.end()) {
    dLine::iterator pt = seg->begin();
    while (pt!=seg->end()) {
      if (pt->first  >= p1.first  && pt->first < p2.first &&
          pt->second >= p1.second && pt->second < p2.second)
        pt++; else pt = seg->erase(pt);
    }
    if (seg->size()) seg++; else seg = brd.erase(seg);
  }
}


/// brd_tester
brd_tester::brd_tester(const dMLine & brd): cache(100){
  // collect line sides info: start and end points, slopes.
  for (dMLine::const_iterator l = brd.begin(); l!=brd.end(); l++){
    int pts = l->size();
    for (int i = 0; i < pts; i++){
      dPt b = (*l)[i%pts], e = (*l)[(i+1)%pts];
      if (b.second == e.second) continue; // no need for horisontal sides
      double s = (e.first-b.first)/(e.second-b.second); // side slope
      sb.push_back(b);
      se.push_back(e);
      ss.push_back(s);
    }
  }
}

/// get integer crossings for a given y coordinate
std::vector<int>
brd_tester::get_cr(int y){
  std::vector<int> cr;
  for (int k = 0; k < sb.size(); k++){
    if ((sb[k].second >  y)&&(se[k].second >  y)) continue; // side is above the row
    if ((sb[k].second <= y)&&(se[k].second <= y)) continue; // side is below the row
    cr.push_back((ss[k] * ((double)y - sb[k].second)) + sb[k].first);
  }
  std::sort(cr.begin(), cr.end());
  return cr;
}

// test if the points is inside the border
bool
brd_tester::test(const int x, const int y){
  if (!cache.contains(y)) cache.add(y, get_cr(y));
  std::vector<int> cr = cache.get(y);
  std::vector<int>::const_iterator i = std::lower_bound(cr.begin(), cr.end(), x);
  int k=0;
  while (i!=cr.end()) {i++; k++;}
  return k%2==1;
}

