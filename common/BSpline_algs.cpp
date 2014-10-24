

#include <armadillo>
#include "BSpline_algs.hpp"
#include <assert.h>

using namespace arma;

void surf_de_boor( int degree_u, int degree_v,  field<vec> cps, std::vector<double> knots_u, std::vector<double> knots_v, double u, double v, Mat<double> &value, int calc_type )
{

  //first we'll reduce the v direction
  field<vec> cps_post_v(2, cps.n_cols); //holder for points before reducing in u
  Mat<double> v_pnts;//(cps(0,0).size(), cps.n_rows );

  //temp structs for holding setup info
  Mat<double> base; 
  std::vector<double> sub_knots_v; 

  for( unsigned int i = 0; i < cps.n_cols; i++)
    {
      
      //use the all vectors of this column to make the new matrix of v_pnts
      for(unsigned int j=0; j<cps.n_rows; j++) v_pnts.insert_cols( j, cps(j,i) );
      //std::cout << v_pnts << std::endl; 
      //now send this off to blossom_de_boor and add the returned point onto the u_pnts
      Mat<double> pnts;
      //blossom_de_boor( degree_v, v_pnts, knots_v, v, v_pnt, false);
      setup_params( degree_v, v_pnts, knots_v, v, base, sub_knots_v);
      find_pnts( base, sub_knots_v, pnts, v, 2);

      //should get two points back from find_pnts, add these to the field of points
      //for the reduction in u
      cps_post_v(0,i) = pnts.col(0); 
      cps_post_v(1,i) = pnts.col(1);
      
      //reset these for the next round
      v_pnts.clear();
      pnts.clear();
    }
  
  //now we'll do a reduction in u as we normally do, but with the new u cps


  std::cout << cps_post_v << std::endl; 

  field<vec> tbt(2,2); //two by two matrix of vectors that will let us get derivatives, points, etc. 
  
  Mat<double> u_pnts;
  std::vector<double> sub_knots_u; 

  for ( unsigned int i = 0; i < cps_post_v.n_rows; i++) 
    {
      //construct a matrix from all the points in a given row of our field
      for(unsigned int j = 0; j < cps_post_v.n_cols; j++) u_pnts.insert_cols( j, cps_post_v(i,j) );
      
      Mat<double>pnts; 

      setup_params(degree_u, u_pnts, knots_u, u, base, sub_knots_u); 
      find_pnts( base, sub_knots_u, pnts, u, 2); 

      tbt(i,0) = pnts.col(0); 
      tbt(i,1) = pnts.col(1); 

      u_pnts.clear(); 
      pnts.clear(); 

    }



  assert( 2 == sub_knots_v.size() ); 
  assert( 2 == sub_knots_u.size() ); 

  std::cout << tbt << std::endl; 
  //now calculate what we want from this two by two matrix 

  switch(calc_type){
  case PNT: 
    field<vec> tmp(2,1);
    //reduce in both directions 
    //u first 
    double u_interpolant = (sub_knots_u[1] - u)/(sub_knots_u[1]-sub_knots_u[0]);
    for ( unsigned int i = 0 ; i < tbt.n_rows; i++) tmp(i,0) = ( u_interpolant*tbt(i,0) + (1-u_interpolant)*tbt(i,1) ); 

    //now v
    double v_interpolant = (sub_knots_v[1] - v)/(sub_knots_v[1]-sub_knots_v[0]);
    for ( unsigned int i = 0 ; i < tmp.n_cols; i ++) value.insert_cols(0, v_interpolant*tmp(0,i) + (1-v_interpolant)*tmp(1,i));
  }

  
}


void setup_params( int degree, Mat<double> cps, std::vector<double> knots, double u, Mat<double> &base, std::vector<double> &sub_knots )
{
  
  base.clear();
  sub_knots.clear();
  //make sure the u-value is valid
  if( u < knots[degree-1] || u > knots[ knots.size() - (degree)] )
    {
      std::cout << "Invalid u-value passed to blossom_de_boor" << std::endl; 
      assert(false);
    }

  int interval = -1;
  //figure out what interval of the curve we're on
  for( unsigned int i = 0 ; i < knots.size()-1 ; i ++ ) 
    {
      
      if( u >= knots[i] && u < knots[i+1] )
	{
	  interval= i-(degree-1);
	  break;
	}
      
    }
   
  assert( 0 <= interval );

  base = cps.cols(interval, interval+(degree));
  
  sub_knots.insert(sub_knots.begin(), knots.begin()+interval, knots.begin()+interval+(degree*2) );

}

void blossom_de_boor( int degree, Mat<double> cps, std::vector<double> knots, double u, Mat<double> &pnt, bool deriv )
{

  Mat<double> base; 
  std::vector<double> sub_knots; 

  setup_params( degree, cps, knots, u, base, sub_knots);

  if(deriv)
    {
      find_pnts(base, sub_knots, pnt, u, 2);
      pnt = (degree/(sub_knots[1]-sub_knots[0]))*(pnt.col(1)-pnt.col(0));
    }
  else
    {
      find_pnts( base, sub_knots, pnt, u, 1);
    }

}


void find_pnts( Mat<double> base, std::vector<double> &knots, Mat<double> &pnt, double u, int num_pnts )
{

  assert( 0 < num_pnts && num_pnts <= base.n_cols );

  if ( num_pnts == base.n_cols ) 
    {
      pnt = base;

      return;
    }
  int offset = base.n_cols-1; 


  Mat<double> new_base(base.n_rows,offset);


  for( unsigned int i = 0 ; i < offset ; i ++)
    {
      double coeff1 = (double(knots[i+offset])-u)/double(knots[i+offset]-knots[i]);

      double coeff2 = (u - double(knots[i]))/double(knots[i+offset]-knots[i]);

      new_base.col(i)= coeff1*base.col(i) + coeff2*base.col(i+1);

    }
      
  //remove the fisrt and last knot before the recursive call 
  std::vector<double> new_knots;
  new_knots.insert(new_knots.begin(), knots.begin()+1, knots.end()-1);
  knots = new_knots;

  find_pnts(new_base, knots, pnt, u, num_pnts);

}
      
