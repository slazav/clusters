#ifndef TIFF_LOADER_H
#define TIFF_LOADER_H

#include "pt.h"
#include "err.h"
#include "cache.h"
#include <tiffio.h>
#include <string>

/// Class for random access to a tiff file.
/// TODO: proper memory handling!

class tiff_loader {
  TIFF* tiff;
  int tiff_w, tiff_h;
  int scan, bpp;
  Cache<Pt, std::string> cache; //line cache
  uint8 *cbuf;
  uint32 tw,tl; //tile size

  public:

  // constructor: open a tiff file
  tiff_loader(const char *fname): cache(100) {

    tiff = TIFFOpen(fname, "rb");
    if (!tiff) throw Err() << "tiff_loader: can't read file: " << fname << "\n";


    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &tiff_w);
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &tiff_h);

    scan = TIFFScanlineSize(tiff);
    bpp = scan/tiff_w;
    if (bpp!=1)  throw Err() << "tiff_loader: non-1bpp file: " << fname;

    std::cerr << "Open tiff file: " << fname << "\n";
    std::cerr << "Width:  " << tiff_w << "\n";
    std::cerr << "Height: " << tiff_h << "\n";

    /// check if we can do random access
    int compression_type, rows_per_strip;
    TIFFGetField(tiff, TIFFTAG_COMPRESSION,  &compression_type);
    TIFFGetField(tiff, TIFFTAG_ROWSPERSTRIP, &rows_per_strip);
    std::cerr << "Compression:  " << compression_type << "\n";
    std::cerr << "Rows:         " << rows_per_strip << "\n";

    uint32 tileWidth, tileLength;
    TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tw);
    TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tl);
    std::cerr << "Tiles: " << tw << "x" << tl << "\n";
    std::cerr << "TileSize: " << TIFFTileSize(tiff) << "\n";
    cbuf = (uint8 *)_TIFFmalloc(TIFFTileSize(tiff));
    if (!cbuf)   throw Err() << "tiff_loader: can't allocate memory: " << fname;
  }

  /// Destructior: close the file
  ~tiff_loader() {
    _TIFFfree(cbuf);
    TIFFClose(tiff);
  }

  /// read a single line and put in the cache
  void read_tile(const int x, const int y){
    if (x<0 || x>=tiff_w) return;
    if (y<0 || y>=tiff_h) return;
    if (cache.contains(Pt(x,y))) return;
    if (TIFFReadTile(tiff, cbuf, x, y, 0, 0) <0) throw Err() << "can't read tile: " << x << " " << y;
    cache.add(Pt(x,y), std::string(cbuf, cbuf+tw*tl));
  }

  /// get a point
  char get(const int x, const int y) {
    if (x<0 || y<0 ||x>=tiff_w || y>=tiff_h) return 0;
    int tx=(x/tw)*tw, ty=(y/tl)*tl;
    if (!cache.contains(Pt(tx,ty))) read_tile(tx,ty);
    return cache.get(Pt(tx,ty))[(x-tx)+(y-ty)*tw];
  }

  int get_w() const {return tiff_w;}
  int get_h() const {return tiff_h;}

};

#endif