#include <iostream>
#include <fstream>
#include <cerrno>
#include <vector>
#include <complex>
#include <climits>
#include <chrono>
#include <thread>

/* This is the Cooley-Tukey radix-2 fft which ran 2^13 points in 5 seconds
 * in 1956. 
 */

using std::complex;
using std::vector;
using std::cout;
typedef complex<double> z_16;
using namespace std::chrono;
constexpr int num_threads = 4;
/******************************************************************************/
constexpr int global_max = INT_MAX;
int msb = -1;

/* in-place sort by bit-reversal */
int bit_reverse_indices( vector<z_16> &v );

/* read data from file to be Fourier transformed. Truncates to 2^n. */
int fill_data( vector<z_16> &start, const char* str );

/* iterative radix-2 fft; len, min_len are DFT sequence end and start lengths
 * so a full length-N DFT looks is called with fft(v.begin(), v.end(), N, 2).
 * (noting that the symmetries employed preclude length-1 DFT sequences) 
 */
void fft ( vector<z_16>::iterator p1, vector<z_16>::iterator p2 , int len, int min_len );

/******************************************************************************/
int main( int argc, char** argv )
{

     int ret;
     vector<complex<double>> fft_out {};
     const char* str = ( argc >= 2 ) ? argv[1] : "in.txt";

     ret = fill_data ( fft_out, str );
     high_resolution_clock::time_point t1 = high_resolution_clock::now();

     std::thread th[num_threads];
     if ( ::msb > 0 ){
	  ret = bit_reverse_indices( fft_out );
	  int offset = fft_out.size()/num_threads;

	  for ( int ii = 0; ii < num_threads; ii++ )
	       th[ii] = std::thread( fft, fft_out.begin() + (ii * offset),
				     fft_out.begin() + ( ii + 1 ) * offset, offset, 2 );

	  for ( int ii = 0; ii < num_threads; ii++ )
	       th[ii].join();

	  fft( fft_out.begin(), fft_out.end(), fft_out.size(), 2*fft_out.size()/(num_threads) );
     }

     high_resolution_clock::time_point t2 = high_resolution_clock::now();
     
     if ( ::msb < 8 ){
	  for ( auto x : fft_out )
	       cout << x << '\n';
     }
     duration<double> elapsed = duration_cast<duration<double>>(t2 - t1);
     cout << "elapsed: " << elapsed.count() << "s\n";

     
     return 0;
}
/******************************************************************************/
int fill_data( vector<z_16> &start, const char* str ){

     int ret = 0;
     int count = 0;
     std::ifstream istream(str, std::ios::in);
     double d;

     if ( !istream.is_open() ){
	  cout << "error with file " << str << '\n';
	  exit(-1);
     }
     
     int n = 2;
     ::msb = 0;
     while ( (istream >> d) && (count < ::global_max) ){
	  count++;
	  start.push_back( std::polar( d, 0. ) );
	  if ( count >= 2*n ){
	       n = count;
	       ::msb++;
	  }
     }
     start.resize(n);
     printf("msb %u\n", ::msb );
     

     return 0;

}
/******************************************************************************/
int bit_reverse_indices( vector<z_16> &v )
{
     //bit reversal f() is its own inverse operation, so all operations are swaps
     //as long as the range of f is its entire domain.
     int c, b = 0;
     int jj;
     int len = v.size() / 2;

     for ( jj = 1;  jj < len; jj++ ){

	  for( c = 1 << ::msb; (b | c) == b; c >>= 1 )
	       ;

	  b |= c;
	  b &= ( ( c << 1 ) - 1 );

	  if ( b != jj )
	       std::swap( v[jj], v[ b ] );


     }
     return 0;
}
/******************************************************************************/
void fft ( vector<z_16>::iterator p1, vector<z_16>::iterator p2 , int len, int min_len )
{
     //loop control
     vector<complex<double>>::iterator x = p1;
     int n;
     //math helpers
     double freq;
     complex<double> phase, swap;

     register int ii, m;
     for ( n = min_len; n <= len; n <<= 1 ){
	  freq = 2 * M_PI / (n);
	  m = n/2;
	  for ( x = p1; x != p2; x += n ){
	       for ( ii = 0; ii < m; ii++ ){
		    swap = x[ii];
		    phase = std::polar(1.0, freq * ii);
		    x[ii] = x[ii] + phase * x[ ii + m];
		    x[ii + m] = swap - phase * x[ ii + m];	    
	       }
	  }
     }
}
