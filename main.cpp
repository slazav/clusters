/* main program for clusters project */
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "err.h"
#include "tiff_loader.h"
#include "corine_proj.h"
#include "borders.h"

bool is_artificial(const char c){ return c>=1 && c<=11; }
bool is_roads(const char c){ return c==4; } // also in artificial
bool is_agricultural(const char c){ return c>=12 && c<=22; }
bool is_natural(const char c){ return c>=23 && c<=39; }
bool is_water(const char c){ return c>=40 && c<=44; }

int
main(int argc, char *argv[]){
  try {

    /* default values */
    const char *datadir = "data";
    const char *country = "";
    const char *res     = "250";
    const char *year    = "00";
    const char *ver     = "18_5";
    const char *brd_scale = "03M";
    const char *brd_year  = "2013";
    const char *image     = "";
    int im_scale = 1;



    /* parse  options */
    while(1){
      opterr=0;
      int c = getopt(argc, argv, "hd:c:r:y:v:Y:S:i:s:");
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
        case 'h':
          std::cout << "cluster finding\n"
                  "Usage: main [options]\n"
                  "Options:\n"
                  " -d <dir>     -- data directory (contains tif and tfw Corine files, RG border geojson)\n"
                  " -c <country> -- select a country using a 2-letter code\n"
                  " -r <res>     -- Corine data resolution, pt/m (100 or 250)\n"
                  " -y <year>    -- Corine data year (90,00,06,12)\n"
                  " -v <ver>     -- Corine data version (such as 18_5)\n"
                  " -t <type>    -- Corine terrain type (number)\n"
                  " -Y <year>    -- Border year (such as 2013)\n"
                  " -S <scale>   -- Border scale (such as 03M, 01M etc.)\n"
                  " -i <file>    -- Write image into a pnm file (default "")\n"
                  " -s <int>     -- Image scale (2 for 1/2 etc., default 1)\n"
                  " -h        -- write this help message and exit\n";
          return 0;
      }
    }

    std::string corine_pref = std::string(datadir) + "/g" + res + "_clc" + year + "_V" + ver;
    std::string corine_tif = corine_pref + ".tif";
    std::string corine_tfw = corine_pref + ".tfw";
    std::string brd_file = std::string(datadir) + "/CNTR_RG_" + brd_scale + "_" + brd_year + "_4326.geojson";

    // access to Corine data
    tiff_loader tiff(corine_tif.c_str());

    // convert Corine coordinate converter
    corine_proj proj(corine_tfw.c_str());

    // coordinate range
    int x0=0, y0=0, w=tiff.get_w(), h=tiff.get_h();

    // border tester
    brd_tester * tester = NULL;

    // country selection
    if (country!=NULL && strlen(country)) {
      dMLine brd = get_border(brd_file.c_str(), country);

      // crop borders to continental Europe (France!)
      border_crop(brd, dPt(-30,25), dPt(45,82));

      proj.wgs2map(brd); // convert border to map coordinates

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
          char c = tiff.get(x0+x*im_scale,y0+y*im_scale);

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

    if (tester) delete tester;

  }
  catch (Err E){
    std::cerr << "Error: " << E.str() << "\n";
    return 1;
  }

}
