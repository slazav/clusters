/* main program for clusters project */
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include "err.h"
#include "tiff_loader.h"
#include "corine_proj.h"
#include "borders.h"
#include "clusters.h"

bool is_artificial(const char c){ return c>=1 && c<=11; }
bool is_roads(const char c){ return c==4; } // also in artificial
bool is_agricultural(const char c){ return c>=12 && c<=22; }
bool is_natural(const char c){ return c>=23 && c<=39; }
bool is_water(const char c){ return c>=40 && c<=44; }

// define some global vars used in check functions
/* default values */
const char *datadir = "data";
const char *country = "";
const char *res     = "250";
const char *year    = "00";
const char *ver     = "18_5";
const char *brd_scale = "03M";
const char *brd_year  = "2013";
const char *image     = "";
const char *clpref    = "";
int cltype  = 0;
int im_scale = 1;

// border tester
brd_tester * tester = NULL;
tiff_loader *tiff = NULL;
corine_proj *proj = NULL;

bool check_fn(const Pt & p){
  if (tester && !tester->test(p.first, p.second)) return false;
  if (!tiff) return false;
  char c = tiff->get(p.first, p.second);
  switch (cltype){
    case 0: return is_artificial(c);
    case 1: return is_agricultural(c);
    case 2: return is_natural(c);
    case 3: return is_water(c);
  }
  return false;
}


double dist_fn(const Pt & p1, const Pt & p2){
  if (!proj) return 0;
  dPt p1a(p1.first, p1.second), p2a(p2.first, p2.second);

  proj->map2wgs(p1a);
  proj->map2wgs(p2a);

  // see: https://www.movable-type.co.uk/scripts/latlong.html
  double R = 6371e3; // metres
  double lat1 = p1a.second * M_PI/180;
  double lat2 = p2a.second * M_PI/180;
  double dlat = lat2-lat1;
  double dlon = (p2a.first-p1a.first) * M_PI/180;

  double sdlat = sin(dlat/2.0);
  double sdlon = sin(dlon/2.0);
  double a = sdlat*sdlat + cos(lat1)*cos(lat2)*sdlon*sdlon;
  double c = 2.0 * atan2(sqrt(a), sqrt(1-a));
  return R * c / 1000.0;
}



int
main(int argc, char *argv[]){
  try {

    /* parse  options */
    while(1){
      opterr=0;
      int c = getopt(argc, argv, "hd:c:r:y:v:Y:S:i:s:C:T:");
      if (c==-1) break;
      switch (c){
        case '?':
        case ':': throw Err() << "incorrect options, see -h"; /* error msg is printed by getopt*/
        case 'd': datadir = optarg; break;
        case 'c': country = optarg; break;
        case 'r': res  = optarg; break;
        case 'y': year = optarg; break;
        case 'v': ver  = optarg; break;
        case 'Y': brd_year = optarg; break;
        case 'S': brd_scale = optarg; break;
        case 'i': image = optarg; break;
        case 's': im_scale = atoi(optarg); break;
        case 'C': clpref = optarg; break;
        case 'T': cltype = atoi(optarg); break;
        case 'h':
          std::cout << "cluster finding\n"
                  "Usage: main [options]\n"
                  "Options:\n"
                  " -d <dir>     -- data directory (contains tif and tfw Corine files, RG border geojson)\n"
                  " -c <country> -- select a country using a 2-letter code\n"
                  " -r <res>     -- Corine data resolution, pt/m (100 or 250)\n"
                  " -y <year>    -- Corine data year (90,00,06,12)\n"
                  " -v <ver>     -- Corine data version (such as 18_5)\n"
                  " -Y <year>    -- Border year (such as 2013)\n"
                  " -S <scale>   -- Border scale (such as 03M, 01M etc.)\n"
                  " -i <file>    -- Write image into a pnm file (default "")\n"
                  " -s <int>     -- Image scale (2 for 1/2 etc., default 1)\n"
                  " -C <pref>    -- Cluster file prefix (default "")\n"
                  " -T <int>     -- Cluster type (0: artificial, 1: agricultural, 2: natural, 3: water)\n"
                  " -h        -- write this help message and exit\n";
          return 0;
      }
    }

    std::string corine_pref = std::string(datadir) + "/g" + res + "_clc" + year + "_V" + ver;
    std::string corine_tif = corine_pref + ".tif";
    std::string corine_tfw = corine_pref + ".tfw";
    std::string brd_file = std::string(datadir) + "/CNTR_RG_" + brd_scale + "_" + brd_year + "_4326.geojson";

    // access to Corine data
    tiff = new tiff_loader(corine_tif.c_str());

    // convert Corine coordinate converter
    proj = new corine_proj(corine_tfw.c_str());

    // coordinate range
    int x0=0, y0=0, w=tiff->get_w(), h=tiff->get_h();


    // country selection
    if (country!=NULL && strlen(country)) {
      dMLine brd = get_border(brd_file.c_str(), country);

      // crop borders to continental Europe (France!)
      border_crop(brd, dPt(-30,25), dPt(45,82));

      proj->wgs2map(brd); // convert border to map coordinates

      // crop borders to file range
      border_crop(brd, dPt(0,0), dPt(w,h));


      // get bbox and update the coordinate range
      dPt p1, p2;
      if (!border_bbox(brd, p1, p2))
        throw Err() << "empty border for country: " << country;
      x0 = p1.first;
      y0 = p1.second;
      w = p2.first-p1.first;
      h = p2.second-p1.second;

      // make border tester
      tester = new brd_tester(brd);
    }


    // make image if needed
    if (image!=NULL && strlen(image)){
      std::ofstream ff(image);
      std::cerr << "Writing image for: " << country
                << " size: " << w/im_scale << " x " << h/im_scale << "\n";
      ff << "P6\n" << w/im_scale << " " 
                   << h/im_scale << "\n255\n";

      for (int y=0; y<h/im_scale; y++){
        for (int x=0; x<w/im_scale; x++){
          char c = tiff->get(x0+x*im_scale,y0+y*im_scale);

          // outside the country
          if (tester && !tester->test(x0+x*im_scale, y0+y*im_scale)){
            c=255-c/2;
            ff << c << c << c;
            continue;
          }

          if (is_artificial(c))   { ff << (char)255 << (char)0   << (char)0;   continue; }
          if (is_agricultural(c)) { ff << (char)255 << (char)255 << (char)0;   continue; }
          if (is_natural(c))      { ff << (char)0   << (char)255 << (char)0;   continue; }
          if (is_water(c))        { ff << (char)0   << (char)255 << (char)255; continue; }
          ff << c << c << c;

        }
      }
    }

    // find clusters
    if (clpref!=NULL && strlen(clpref)){
      std::cerr << "Writing cluster information for: " << country << "\n";

      cluster_finder finder(x0,x0+w,y0,y0+h, check_fn, dist_fn);
      std::string pr(clpref);
      std::ofstream f_npts((pr + "_npts.txt").c_str());
      std::ofstream f_area((pr + "_area.txt").c_str());
      std::ofstream f_lsize((pr + "_lsize.txt").c_str());
      std::ofstream f_ssize((pr + "_ssize.txt").c_str());
      while (finder.find_next()){
        f_npts << finder.get_npts() << "\n";
        f_area << finder.get_area() << "\n";
        f_lsize << finder.get_lsize() << "\n";
        f_ssize << finder.get_ssize() << "\n";
      }
    }


    if (tester) delete tester;
    if (tiff) delete tiff;
    if (proj) delete proj;

  }
  catch (Err E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }

}
