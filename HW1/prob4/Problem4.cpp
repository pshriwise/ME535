


#include <iostream>
#include <fstream>
#include <armadillo>
#include <assert.h>

using namespace arma;


void de_cast( double u, Mat<double> CP, Mat<double> &pnt, std::vector<Mat<double > > &plot_data );

int main(int argc, char** argv)
{

  std::vector<std::string> filenames;

  int u_pnts = 20;
  double u = 0.5;

  // setup the point matrix
  Mat<double> P;
  P << 0 << 0 << 0 << endr
    << 1 << 2 << 0 << endr
    << 3 << 5 << 0 << endr
    << 4 << 4 << 0 << endr
    << 5 << 0 << 0 << endr;

  P = P.t();
  std::cout << P.col(0) << std::endl;

  std::vector<Mat<double> > plot_data;

  Mat<double> pnt(3,1);
  de_cast( u, P, pnt, plot_data );

  
  //first, let's use de_cast to create the curve
  
  double m = 1/(double(u_pnts)-1);
      
  Mat<double> curve(3,u_pnts);
  for(int i=0; i < u_pnts; i++)
    {
      u = double(i)*m;

      de_cast( u, P, pnt, plot_data );
      
      curve(0,i) = pnt(0,0);
      curve(1,i) = pnt(1,0);
      curve(2,i) = pnt(2,0);

    }

  //write the curve points to a data file
  std::ofstream data_file;
  
  //create the filename
  std::ostringstream data_filename;
  data_filename << "Problem4_curve.dat";
  filenames.push_back( data_filename.str() );
  
  data_file.open(&(data_filename.str()[0]) );
  
  for(unsigned int i = 0; i < curve.n_cols; i++)
    {
      
      data_file << curve(0,i) << "\t" << curve(1,i) << "\t" << curve(2,i) << "\n";
      
    }
  
  data_file.close();

  return 0;
}


// plot data will contain all information if the user wants to plot
// the progression of the algorithm
void de_cast( double u, Mat<double> CP, Mat<double> &pnt, std::vector<Mat<double > > &plot_data )
{
  assert(3 == CP.n_rows);

  //add this matrix to the plot_data
  plot_data.push_back( CP );

  // if we only have one point, we're done, return the point as a matrix
  if( CP.n_cols == 1) 
    {

      pnt = CP;
      return;

    }
  // otherwise calculate the points and then recurse on the new matrix
  else
    {
      //create a new matrix for storing the values
      int new_cols = CP.n_cols - 1;
      Mat<double> N(3, new_cols);
      
      for( unsigned int i = 0; i < new_cols; i++) 
	{
	  //extract the vectors we want
	  vec A = CP.col(i);

	  vec B = CP.col(i+1);

	  //calculate the new vectors
	  vec new_vec = A + u*(B-A);
	  
	  //insert this new vector into the new matrix
	  N(0,i) = new_vec(0);
	  N(1,i) = new_vec(1);
	  N(2,i) = new_vec(2);
	  
	}
      
      de_cast( u, N, pnt, plot_data );

    }





}
