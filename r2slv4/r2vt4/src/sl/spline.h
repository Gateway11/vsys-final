//
//  spline.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__spline__
#define __r2vt4__spline__

#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>

// unnamed namespace only because the implementation is in this
// header file and we don't want to export symbols to the obj files
namespace __r2vt4__ {
  
  // band matrix solver
  class band_matrix {
  private:
    std::vector< std::vector<double> > m_upper;  // upper band
    std::vector< std::vector<double> > m_lower;  // lower band
  public:
    band_matrix() {};                             // constructor
    band_matrix(int dim, int n_u, int n_l);       // constructor
    ~band_matrix() {};                            // destructor
    void resize(int dim, int n_u, int n_l);      // init with dim,n_u,n_l
    int dim() const;                             // matrix dimension
    int num_upper() const {
      return m_upper.size()-1;
    }
    int num_lower() const {
      return m_lower.size()-1;
    }
    // access operator
    double & operator () (int i, int j);            // write
    double   operator () (int i, int j) const;      // read
    // we can store an additional diogonal (in m_lower)
    double& saved_diag(int i);
    double  saved_diag(int i) const;
    void lu_decompose();
    std::vector<double> r_solve(const std::vector<double>& b) const;
    std::vector<double> l_solve(const std::vector<double>& b) const;
    std::vector<double> lu_solve(const std::vector<double>& b,
                                 bool is_lu_decomposed=false);
    
  };
  
  
  // spline interpolation
  class spline {
  private:
    std::vector<double> m_x,m_y;           // x,y coordinates of points
    // interpolation parameters
    // f(x) = a*(x-x_i)^3 + b*(x-x_i)^2 + c*(x-x_i) + y_i
    std::vector<double> m_a,m_b,m_c,m_d;
  public:
    void set_points(const std::vector<double>& x,
                    const std::vector<double>& y, bool cubic_spline=true);
    double operator() (double x) const;
    double get_point(double x);
  };
  
} // namespace tk



#endif /* __r2vt4__spline__ */
