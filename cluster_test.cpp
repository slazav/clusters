#include "clusters.h"
#include <iostream>
#include <cmath>

const int W=10, H=10;

int data[H][W] = {
{0,1,0,0,0,1,1,1,1,0},
{0,1,1,1,1,1,0,0,1,0},
{0,1,1,1,1,1,0,0,1,0},
{0,1,0,1,1,1,0,0,0,0},
{0,1,1,1,1,0,0,1,1,0},
{0,0,0,0,0,0,1,0,1,0},
{0,0,0,0,0,0,1,1,0,0},
{0,0,0,0,0,0,0,0,1,0},
{0,1,0,0,0,0,0,1,1,0},
{0,0,0,0,0,0,0,0,1,0}};

bool check_fn(const Pt & p){
  if (p.first < 0 || p.first >=W || p.second < 0 || p.second >=H) return false;
  return data[p.second][p.first];
}

double dist_fn(const Pt & p1, const Pt & p2){
  double dx = p2.first-p1.first;
  double dy = p2.second-p1.second;
  return sqrt(dx*dx+dy*dy);
}

main(){

  // print data
  std::cout << "Original data:\n";
  for (int y=0; y<H; y++) {
    for (int x=0; x<W; x++) {
      std::cout << (check_fn(Pt(x,y))? "*":".");
    }
    std::cout << "\n";
  }
  std::cout << "\n";


  cluster_finder finder(0,W,0,H, check_fn, dist_fn);
  while (finder.find_next()){

    std::cout << "New cluster:\n";
    std::cout << "  points: " << finder.get_npts() << "\n";
    std::cout << "  area:   " << finder.get_area() << "\n";
    std::cout << "  lsize:  " << finder.get_lsize() << "\n";
    std::cout << "  ssize:  " << finder.get_ssize() << "\n";

    // print the cluster
    for (int y=0; y<H; y++) {
      for (int x=0; x<W; x++) {
        if      (finder.get_pts().count(Pt(x,y))) std::cout << "*";
//        else if (finder.get_brd().count(Pt(x,y))) std::cout << "o";
        else std::cout << ".";
      }
      std::cout << "\n";
    }
    std::cout << "\n";

  }

}
