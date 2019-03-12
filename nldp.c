/******************************************************************************/
// This module solves for the optimal cost of controlling a force-controlled
// nonlinear pendulum from any initial condition. The algorithm is broadly
// described in Bellman, "Introduction to the Mathematical Theory of Control
// Processes, vol. II." 
/******************************************************************************/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
//number of rows
#define N 101
//number of columns
#define M 101
//samples in control action, time.
#define S 50
//control resolution
#define U (100*S + 1)

typedef struct Pair{
    double x0;
    double x1;
} Pair;

typedef struct Tuple{
    double x0;
    double x1;
    double u;
    double cost;
} Tuple;

/******************************************************************************/
// these functions convert between normalized values for state and control
// variables and grid points in the sampled state space. The state is given
// by (x0,x1), with x0, x1, and the control u elements of [-1,1].
double d2c( int index, int len )
{
    double d = ( index * 2./(len - 1)) - 1;

    if ( d > 1 )
	d = 1;
    else if ( d < -1 )
	d = -1;
    
    return d;
}

int c2d( double d, int len )
{    
    int i = (int)round( (d + 1) * (len - 1) / 2. );

    if ( i < 0 )
	i = 0;
    else if ( i > len - 1 )
	i = len - 1;
    
    return i;
}

/******************************************************************************/
//functions associate with costs and dynamics, retaining their usual meanings.

//terminal costs if distinct from h, below. 
double v( double x0, double x1, double u )
{
    return (x0*x0 + x1*x1 + u*u)/S;
}

//arguments differ from the above so that we may call h(g(c,u),u).
double h( Pair x, double u )
{   
    return (x.x0*x.x0 + x.x1*x.x1 + u*u)/S;
}

//currently configured for a pendulum controlled by force. Note the gain c on
//the force -- the state and control variables are taken to be normalized
//throughout, so this gain must be high enough to ensure the optimal control
//given in its typical units falls into the normalized range of the search.
Pair g( Pair x, double u )
{
    static double k = 1.;
    static double T = 1/S;
    static double mass = 0.3;
    double c = U;

    double x1 = x.x1 - T*( k*sin(x.x0) + c*u )/mass;
    double x0 = x.x0 + x1*T;
    return (Pair){x0,x1};

}

/******************************************************************************/
// for verifying behaviour of coarsely-approximated systems.
void print_mat( double* buf )
{
    for ( int ii = 0; ii < N; ii++ )
    {
	for ( int jj = 0; jj < M; jj++ )
	    printf("%1.4f ", buf[jj + M*ii]);
	printf("\n");
    }
    printf("\n\n");
}
/******************************************************************************/
// interpolates the optimal cost f_n(c) from f_n-1.
double lookup ( Pair state, double *buf )
{
    int x = c2d( state.x0, N );
    int y = c2d( state.x1, M );

    double estimate, nearest = buf[ x + y*N ];
    
    //may be either positive or negative; applied to the local forwards difference.
    double err_x = state.x0 - d2c( x, N );
    double err_y = state.x1 - d2c( y, M );

    double x_next, y_next;
    x_next = y_next = nearest;
    
    if ( (x + 1 + y*N ) < N*M )
	x_next = buf[ x + 1 + y*N ];

    if ( ( x + (y+1)*N ) < N*M )
	y_next = buf[ x + (y+1)*N ];
    
    estimate = nearest + (err_x/(N-1)) * (x_next - nearest ) + (err_y/(M-1)) * ( y_next - nearest );
    
    return estimate;
}




/******************************************************************************/
// This is a gradient descent search for convex systems. The function h is
// evaluated at points of a distance determined by both the controller resolution
// and also the time resolution, since the time resolution enters into g above.

Tuple search( double* prev, Pair state, double guess )
{
    //tuning parameters
    double threshold = 1E-9;
    static double k = 2;
    double eps = 0.00001;
    int max = 70;

    //find the gradient at the guess
    double c1 = h( g(state,guess), guess ) + lookup( g(state,guess), prev );
    double c2 = h( g(state, guess + eps), eps ) + lookup( g(state, guess+eps), prev );
    double grad = ( c2 - c1 )/eps;

    //loop variables
    double u = guess;
    int ii, count = 0;
    
    //diagnostic & performance information
    static int total_count = 0;
    static int calls = 0;
    calls++;

    //buffer for shanks transform
    double vec[3];

    //used to change the tunings within the loop
    double m1, m2;
    m1 = m2 = fabs(grad);
    int s1, s2;
    s1 = s2 = ( grad > 0 );

    
    while ( ((grad > threshold) || (grad < -1*threshold)) && (count < max ) )
    {
	total_count++;
	
        ii = count%3;
	vec[ii] = u;
	
	if ( ii == 2 )
	{
	    //series acceleration
	    double s = ( vec[ii]*vec[ii-2] - vec[ii-1]*vec[ii-1] )/( vec[ii] -2*vec[ii-1] + vec[ii-2] );
	    //make sure the denominator doesn't cause any problems
	    u = ( isfinite(s) ? s : u - k*grad );
	    
	}
	else
	{
	    u -= k*grad;
	}
	
	count++;

	c1 = h( g(state, u), u ) + lookup( g(state,u), prev );
	c2 = h( g(state, u+eps), u+eps)+ lookup( g(state,u+eps), prev );

	grad = ( c2 - c1 )/eps;
	s2 = ( grad > 0 );
	m2 = fabs(grad);


	//make sure the series is oscillating and converging
	if ( s1 == s2 )
	{
	    if ( m2 >= m1 )
		k *= 0.8;
	    else
		k *= 1.2;
	}
	else
	{
	    if ( m2 >= m1 )
		k *= 0.8;
	}

	m1 = m2;
	s1 = s2;
	
    }

    Pair x = g(state,u);    
    Tuple t = (Tuple){x.x0,x.x1,u,c1};

    
    return t;

}


/******************************************************************************/
//brute-force search for checking work & for more general cases
Tuple search_bf( double *prev, Pair state, double guess )
{
    //minimum depends on time window and must be set accordingly
    double min = 10000.;
    double v;
    for ( double u = -1.; u <= 1.; u += 1./U )
    {
	double c1 = h( g(state, u), u ) + lookup( g(state,u), prev );
	if ( c1 < min )
	{
	    min = c1;
	    v = u;
	}	
    }
    Pair next = g(state,v);
    return (Tuple){ next.x0, next.x1, v, min };
}


/******************************************************************************/
// Optimal policy & dynamics are not captured. These are the other attributes
// of the Tuple t in the second loop.
int main( char argc, char** argv )
{

    double h0[N][M];

    memset( h0, 0., sizeof(double)*N*M );

    for ( int ii = 0; ii < N; ii++ )
    {
	for ( int jj = 0; jj < M; jj++ )
	    h0[ii][jj] = v( d2c(ii, N), d2c(jj, M), 0. );
    }

    printf("f0 done\n");
//    print_mat(&h0[0][0]);
    
    
    double f0[N][M];
    memcpy( f0, h0, sizeof(double)*N*M );

    double f1[N][M];    

    double *f, *f_prev;
    f = &f1[0][0];
    f_prev = &f0[0][0];
    double guess = 0;
    for ( int kk = 0; kk < S; kk++ )
    {
	for ( int ii = 0; ii < M; ii++ )	 
	{ 
	    for ( int jj = 0; jj < N; jj++ ) 
	    {

		Pair p = (Pair){ d2c(jj, N), d2c(ii, M) };
		Tuple t = search( f_prev, p, guess );
		guess = t.u;
		f[jj + N*ii] = t.cost;
	    }

	}
//	printf("time %d: %f\n", kk, f[N*M -1]);
	f_prev = f;
	
    }
    //spot check
    printf("done: %2.2f\n", f[N*M - 1]);
//    print_mat(f);
    
    return 0;
}
