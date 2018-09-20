#include <set>
#include <queue>
#include <set>
#include <cmath>

#include "clusters_d.h"

double triv_dist_fn(const Pt & p1, const Pt & p2){
  return hypot(p1.first-p2.first, p1.second-p2.second);
}

Pt
neighbour(const Pt &p, const int dir){
  switch(dir%8){
    case 0: return Pt(p.first-1,p.second-1);
    case 1: return Pt(p.first,  p.second-1);
    case 2: return Pt(p.first+1,p.second-1);
    case 3: return Pt(p.first+1,p.second  );
    case 4: return Pt(p.first+1,p.second+1);
    case 5: return Pt(p.first,  p.second+1);
    case 6: return Pt(p.first-1,p.second+1);
    case 7: return Pt(p.first-1,p.second  );
  }
}

char
cluster_finder::get_pt(const Pt & p) const {
  if (p.first < x1 || p.first >= x2 ||
      p.second < y1 || p.second >= y2) return 255;
  return data[(p.first-x1) + (p.second-y1)*(x2-x1)];
}

bool
cluster_finder::put_pt(const Pt & p) {
  if (pts.count(p)) return false; // point exists
  data[(p.first-x1) + (p.second-y1)*(x2-x1)] = (char)128 + n%128;
  pts.insert(p);
  brd.erase(p);
  for (int i=0; i<8; i++){
    Pt p2 = neighbour(p, i);
    if (pts.count(p2)==0) brd.insert(p2);
  }
  return true;
}

void
cluster_finder::find_cl(const Pt & p) {
  pts=brd=std::set<Pt>(); // reset cluster data
  std::queue<Pt> q;

  if (get_pt(p) != val) return;
  // add original point in the cluster and in the queue
  q.push(p);
  put_pt(p);

  // Get points from the queue until it is empty.
  // For each point find 8 neighbours of it.
  // If neighbour should be put to the cluster and it is not there yet
  // put in in the cluster and in the queue.
  // Also keep set boundary in brd: after adding a new point
  while (!q.empty()){
    Pt p1 = q.front();
    q.pop();
    for (int i=0; i<8; i++){
      Pt p2 = neighbour(p1, i);
      if (get_pt(p2) == val && put_pt(p2)) q.push(p2);
    }
  }
}

/// find next cluster, return true if it was possible
bool
cluster_finder::find_next() {

  // continue loop through the whole plane
  for (; y<y2; y++) {
    if (x==x2) x=x1;
    for (; x<x2; x++) {
      Pt p(x,y);
      if (get_pt(p)!=val) continue; // "bad" point
    std::cerr << "Cluster start: " << p.first << "\n";

      // find a cluster
      find_cl(p);
      n++;
    std::cerr << "Cluster: " << pts.size() << "\n";
      return true;
    }
  }
  // no more clusters:
  pts=brd=std::set<Pt>(); // reset cluster data
  return false;
}


double
cluster_finder::get_area() const {
    std::cerr << "area: ";
  double ret = 0.0;
  for (std::set<Pt>::const_iterator i=pts.begin(); i!=pts.end(); i++){
    // find 1-point distances dx and dy, calculate 1 point area
    ret += dist_fn(*i, neighbour(*i,3)) * dist_fn(*i, neighbour(*i,5));
  }
    std::cerr << ret << "\n";
  return ret;
}


double
cluster_finder::get_lsize() const {

    std::cerr << "lsize: ";

  // just a double loop for all border points
  double ret=0.0;
  Pt p1,p2;
  std::set<Pt>::const_iterator i1,i2;
  for (i1=brd.begin(); i1!=brd.end(); i1++){
    for (i2=brd.begin(); i2!=brd.end(); i2++){
      double d = triv_dist_fn(*i1,*i2);
      //double d = dist_fn(*i1,*i2);
      if (d>ret) { ret=d; p1=*i1; p2=*i2;} // find max distance, save border points
    }
  }

  // Fix distance (we want to use not boundary point centers,
  // but edges). Loop through all neighbours of furthermost boundary points
  // included in the cluster, find the smallest distance
  ret=dist_fn(p1, p2);
  double dist=ret;
  for (int i=0; i<8; i++){
    Pt p1a = neighbour(p1, i);
    if (!pts.count(p1a)) continue;
    for (int j=0; j<8; j++){
      Pt p2a = neighbour(p2, j);
      double d = dist_fn(p1a, p2a);
      if (d<dist) dist=d;
    }
  }
  // correction:
  ret = (ret + dist)/2.0;

    std::cerr << ret << "\n";
  return ret;
}


/// get cluster "small size" (diameter of inscribed circle)
double
cluster_finder::get_ssize() const {
  // we need to check this explicitely:
  if (!brd.size() || !pts.size()) return 0.0;

    std::cerr << "ssize: ";

  // just a double loop for all set points + border points
  double ret=0.0;
  Pt pc,pb;
  std::set<Pt>::const_iterator i1,i2;
  for (i1=pts.begin(); i1!=pts.end(); i1++){
    //double d0 = dist_fn(*i1,*brd.begin()); // non-empty brd needed!
    double d0 = triv_dist_fn(*i1,*brd.begin()); // non-empty brd needed!
    Pt pb1;
    for (i2=brd.begin(); i2!=brd.end(); i2++){
      //double d = dist_fn(*i1,*i2);
      double d = triv_dist_fn(*i1,*i2);
      if (d<d0) { d0=d; pb1=*i2;} // find min distance, save border point
    }
    if (d0>ret) {ret=d0; pc=*i1; pb=pb1;} // find max size, save central point
  }

  // Fix distance (similar to one in get_lsize(), but fix only one border point
  ret=dist_fn(pb, pc);
  double dist=ret;
  for (int j=0; j<8; j++){
    Pt pba = neighbour(pb, j);
    double d = dist_fn(pc, pba);
    if (d<dist) dist=d;
  }
  // correction (we want diameter, not radius):
  ret = (ret + dist);

    std::cerr << ret << "\n";
  return ret;
}
