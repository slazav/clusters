#ifndef CLUSTERS_H
#define CLUSTERS_H

#include <set>
#include <queue>
#include <set>
#include <iostream>
#include "pt.h"

Pt neighbour(const Pt &p, const int dir);

/// Class for finding connected clusters on a 2D plane.
class cluster_finder {

  /// function for calculating distance between points
  /// (on a map it can be non-trivial function)
  typedef double (*dist_fn_t)(const Pt & p1, const Pt & p2);

  char *data;
  char val;
  int n;
  dist_fn_t  dist_fn;
  int x1,x2,y1,y2;    ///< plane boundary: x1 <= x < x2, y1 <= y < y2
  int x,y;            ///< current coordinates

  std::set<Pt> brd; ///< cluster border
  std::set<Pt> pts; ///< cluster points

  /// Put a point in the cluster; update cluster boundary.
  /// Return true if point have been added, false if it exists in te cluster.
  bool put_pt(const Pt & p);

  char get_pt(const Pt & p) const;

  public:
    /// Constructor: start finding clusters in the x1:x2, y1:y2 range
    cluster_finder(const int x1, const int x2,
                   const int y1, const int y2,
                   char *data, char val, dist_fn_t dist_fn):
        x1(x1), x2(x2), y1(y1), y2(y2), x(x1), y(y1),
        data(data), val(val), dist_fn(dist_fn), n(0) {};

    /// find a cluster starting with point p
    void find_cl(const Pt & p);

    /// find next cluster, return true if it was possible
    bool find_next();

    /// get reference to point sets
    const std::set<Pt> & get_pts() const { return pts;}
    const std::set<Pt> & get_brd() const { return brd;}

    /// get number of points in the cluster
    int get_npts() const { return pts.size();}

    /// get cluster area
    double get_area() const;

    /// get cluster "large size" (diameter of escribed circle)
    double get_lsize() const;

    /// get cluster "small size" (diameter of inscribed circle)
    double get_ssize() const;

};

#endif