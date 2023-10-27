#ifndef		_IBIT_VECTOR_H_
#define		_IBIT_VECTOR_H_

#include	<limits.h>
#include        <stdint.h>

// Do this typedef with suitable int type in your source before including 
// this. This is to configure for your exact needs.


#ifndef		bitvector
typedef		unsigned long long int		bitvector;
#endif

typedef		bitvector *		BITVPTR;

#define		BITS			( CHAR_BIT * sizeof( bitvector ))

#define		setbit( v, i )		v |= ( bitvector )1 << ( i )
#define		clrbit( v, i )		v &= ~(( bitvector )1 << ( i ))
#define		xorbit( v, i )		v ^= ( bitvector )1 << ( i )
#define		tstbit( v, i )		(( v >> ( i )) & ( bitvector )1 )
#define		setvecbits( v )		v = ~( bitvector )0
#define		clrvecbits( v )		v = ( bitvector )0

#define		bv_size( len )		((( len ) + BITS - 1 ) / BITS )
#define		bv_bytes( len )		( bv_size( len ) * sizeof( bitvector ))
#define		bv_hi( i )		(( i ) / BITS )
#define		bv_lo( i )		(( i ) % BITS )

#define		bv_setbit( v, i )	setbit( v[ bv_hi( i ) ], bv_lo( i ))
#define		bv_clrbit( v, i )	clrbit( v[ bv_hi( i ) ], bv_lo( i ))
#define		bv_xorbit( v, i )	xorbit( v[ bv_hi( i ) ], bv_lo( i ))
#define		bv_tstbit( v, i )	tstbit( v[ bv_hi( i ) ], bv_lo( i ))

#define		bv_set( v, len )	memset( v, ~0, bv_bytes( len ))
#define		bv_clr( v, len )	memset( v,  0, bv_bytes( len ))

#define		bv_alloc( len )		( BITVPTR )malloc( bv_bytes( len ))
#define		bv_realloc(v, len )	( BITVPTR )realloc(v, bv_bytes( len ))
#define		bv_free( v )		free( v )

#ifdef		__GNUC__
// #define		bv_popcount( v )	__builtin_popcount( v )
#endif

#define		bv_print_bits( bitz )				\
                { 						\
			int			i;		\
		        unsigned long long 	b = bitz;	\
                                                                \
		        i = sizeof( bitz ) * CHAR_BIT;		\
		        while( i-- ) {				\
			        printf("%d", ( int )b & 1 );	\
			        b >>= 1;			\
			}					\
		        printf("\n");				\
		}

static inline bitvector	bv_getbits( BITVPTR v, int i, int j, int len )
{
	int			h, l;
	bitvector		w;

	h = bv_hi( i );
	l = bv_lo( i );

	if( h == len - 1 ) 
		w = v[ h ] >> l;
	else
		w = ( v[ h ] >> l ) | ( v[ h + 1 ] << ( BITS - l ));

	return w & (( 1 << ( j - i + 1 )) - 1 );
}

static inline int	bv_cmp( BITVPTR v, BITVPTR w, int i )
{
 
	while( !( --i < 0 )) if( v[ i ] != w[ i ] ) break;
	if( i < 0 ) return 0;
	if( v[ i ] < w[ i ] ) return -1;
	if( v[ i ] > w[ i ] ) return  1;

	return 0; // supress warning
}


static inline int	bv_is_zero( BITVPTR v, int len )
{ 
  int     i = bv_size (len);

	do if( *v++ ) return 0; while( --i );

	return 1;
}

static inline void	bv_copy( BITVPTR v, BITVPTR w, int i )
{
  if (!i) return;
	do *v++ = *w++; while( --i );
}

static inline void	bv_or_c( BITVPTR q, BITVPTR v, BITVPTR w, int i )
{
	do *q++ = *v++ | *w++; while( --i );
}

static inline void	bv_or_r_c( BITVPTR q, BITVPTR v, BITVPTR w, 
				   int i, int j )
{
	for( ; i <= j; i++ ) q[ i ] = v[ i ] | w[ i ];
}

static inline void	bv_or( BITVPTR v, BITVPTR w, int i )
{
	do *v++ |= *w++; while( --i );
}

/* Huom! l=bittien lkm. */
/* Bittior kun |v|>=|w| */
static inline void      bv_or_vlonger( BITVPTR v, BITVPTR w, int l)
{
        int             i, n = bv_size(l);
        bitvector       q = ~( bitvector )0;

        for( i = 0; i < n - 1; i++ ) v[ i ] |= w[ i ];
        if( l % BITS ) q >>= BITS - (l % BITS);
        v[ i ] |= w[ i ] & q;
}






static inline void	bv_and( BITVPTR v, BITVPTR w, int i )
{
  if (!i) return;
	do *v++ &= *w++; while( --i );
}

/* Huom! l=bittien lkm. */
/* Bittiand kun |v|>=|w| */
static inline void      bv_and_vlonger( BITVPTR v, BITVPTR w, int l)
{
        int             i, n = bv_size(l);
        bitvector       q = ~( bitvector )0;

        for( i = 0; i < n - 1; i++ ) v[ i ] &= w[ i ];
        if( l % BITS ) q >>= BITS - (l % BITS);
        v[ i ] &= ( w[ i ] & q ) | ~q;
}



static inline void	bv_or_c_and( BITVPTR v, BITVPTR w, BITVPTR u, int i )
{
	do *v++ |= *w++ & *u++; while( --i );
}

static inline void	bv_and_c( BITVPTR q, BITVPTR v, BITVPTR w, int i )
{
	do *q++ = *v++ & *w++; while( --i );
}

static inline void	bv_xor_c( BITVPTR q, BITVPTR v, BITVPTR w, int i )
{
	do *q++ = *v++ ^ *w++; while( --i );
}

static inline void	bv_not_c( BITVPTR v, BITVPTR w, int i )
{
	do *v++ = ~*w++; while( --i );
}

static inline void	bv_and_c_not( BITVPTR v, BITVPTR w, int i )
{
	do *v++ &= ~*w++; while( --i );
}

static inline void	bv_not( BITVPTR v, int i )
{
	do { *v = ~*v; ++v; } while( --i );
}






/*
static inline void	bv_imul( BITVPTR v, int m, int j )
{
	int			i;
	unsigned long long	r = 0LL;

	for( i = 0; i < j; i++ )
	{
		r >>= BITS;
		r = ( unsigned long long )v[ i ] * m + r;
		v[ i ] = ( bitvector )r;
	}
}

static inline void	bv_imul_c( BITVPTR v, BITVPTR w, int m, int j )
{
	int			i;
	unsigned long long	r = 0LL;

	for( i = 0; i < j; i++ )
	{
		r >>= BITS;
		r = ( unsigned long long )w[ i ] * m + r;
		v[ i ] = ( bitvector )r;
	}
}
*/
/*
static inline void	bv_add( BITVPTR v, BITVPTR w, int j )
{
	int			i;
	unsigned long long	r = 0LL;

	for( i = 0; i < j; i++ )
	{
		r = ( unsigned long long )v[ i ] + 
		    ( unsigned long long )w[ i ] + r;
		v[ i ] = ( bitvector )r;
		r >>= BITS;
	}
}

static inline void	bv_add_c( BITVPTR q, BITVPTR v, BITVPTR w, int j )
{
	int			i;
	unsigned long long	r = 0LL;

	for( i = 0; i < j; i++ )
	{
		r = ( unsigned long long )v[ i ] + 
		    ( unsigned long long )w[ i ] + r;
		q[ i ] = ( bitvector )r;
		r >>= BITS;
	}
}

static inline void	bv_add_r_c( BITVPTR q, BITVPTR v, BITVPTR w, 
				    int i, int j )
{
//	int			k = 0;
	unsigned long long	r = 0LL;

	for( ; i <= j; i++ )
	{
		r = ( unsigned long long )v[ i ] + 
		    ( unsigned long long )w[ i ] + r;
		q[ i ] = ( bitvector )r;
		r >>= BITS;
	}
}

#ifdef __i386__

static inline void	bv_add_asm( BITVPTR v, BITVPTR w, int j )
{
	int		i;

//	__asm__ volatile ("clc");

	__asm__ volatile ("addl	%1,%0" : "=m" (v[0]) : "r" (w[0]) );
	__asm__ volatile ("pushfl");
	for( i = 1; i < j; i++ )
	{
		__asm__ volatile ("popfl");
		__asm__ volatile ("adcl	%1,%0" : "=m" (v[i]) : "r" (w[i]) );
		__asm__ volatile ("pushfl");
//		__asm__ volatile ("addl	%1,%0" : "=m" (v[i]) : "r" (w[i]) );
//		__asm__ volatile ("adcl	$0,%0" : "=m" (v[i+1]) );
	}
	__asm__ volatile ("popfl");

//	__asm__ volatile(    "addl	%1,%0" : "=m" (v[i]) : "r" (w[i]) );
}

#endif
*/
// Assumes that s is at most BITS...

static inline void	bv_shr( BITVPTR v, int s, int i )
{
	while( --i ) 
		{ *v = ( *v >> s ) | (( *( v + 1 )) << ( BITS - s )); ++v; }

	*v >>= s; 
}

// Assumes that s is at most BITS...

static inline void	bv_shl( BITVPTR v, int s, int i )
{
	int		j, bs = BITS - s;

	for( j = i - 1; j > 0; j-- )
		v[ j ] = ( v[ j ] << s ) | (( v[ j - 1 ] ) >> bs );

	v[ j ] <<= s;
}



#ifdef __i386__

// Assumes that s is at most BITS... This is slow, as shld is not pairaple...

static inline void	bv_shl_asm( BITVPTR v, int s, int i )
{
	int		j;

	__asm__("movb	%0,%%cl"  : : "g" (s) : "%cl" ); 
	for( j = i - 1; j > 0; j-- )
	{
		__asm__("shldl	%1,%0" : "=m" (v[j]) : "r" (v[j-1]) : "%cl" );
	}

	v[ j ] <<= s;
}

static inline void	bv_shl_asm_1( BITVPTR v, int i )
{
	int		j;

	for( j = i - 1; j > 0; j-- )
	{
		__asm__("shldl	$1,%1,%0" : "=m" (v[j]) : "r" (v[j-1]) );
	}

	v[ j ] <<= 1;
}

#endif

static inline void	x_bv_shl( BITVPTR v, int s, int i )
{
	bitvector	r = (( 1 << s ) - 1 ) << ( BITS - s );
	int		j;

	for( j = i - 1; j > 0; j-- )
		v[ j ] = ( v[ j ] << s ) | 
			(( v[ j - 1 ] & r ) >> ( BITS - s ));

	v[ j ] <<= s;

}


/* is w a subset of v? */
static inline int  bv_subset( BITVPTR v, BITVPTR w, int len )
{
        int     i = bv_size (len);

        do { if ((*v & *w) != *w) return 0; v++, w++; } while( --i );
        return 1;
}




#define	popcounta(x)	({ 	uint32_t __u = (x), __v = __u - ((__u >> 1) & 0x55555555);	\
				__v = (__v & 0x33333333) + ((__v >> 2) & 0x33333333);		\
				(((__v + (__v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;})



static const unsigned char poptbl [] = 
{
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

static inline int	popcountt (uint32_t s)
{
	unsigned char * p = (unsigned char *) &s;

	return	poptbl [p [0]] + poptbl [p [1]] + 
	    	poptbl [p [2]] + poptbl [p [3]];
}

#define popcountts(v,r)	({	int i; unsigned char * p = (unsigned char *)&v; \
				for (i = r = 0; i < sizeof (v); i++) r += poptbl [*p++]; }) 


static inline int	bv_popcntx (BITVPTR v, int i)
{
	int	r = 0;

	do popcountts ((*v), r); while( --i );

	return r;
}

#ifndef          __GNUC__

static inline int	bv_popcnt (BITVPTR v, int i)
{
	int		r = 0;
	unsigned char 	* p = (unsigned char *)v;

	i *= sizeof (bitvector);

	do r += poptbl [*p++]; while( --i );

	return r;
}

#else

static inline int	bv_popcnt (BITVPTR v, int i)
{
	int		r = 0;


	if (sizeof (bitvector) <= sizeof (unsigned))
		do r += __builtin_popcount (*v++); while( --i );
	else if (sizeof (bitvector) == sizeof (long))
		do r += __builtin_popcountl (*v++); while( --i );
	else
		do r += __builtin_popcountll (*v++); while( --i );

	return r;
}

#endif

// static void init_rank_1()
// {
// 	int	i;

// 	rank_1_bst [0] = 0;
// 	for (i = 0; i < 1 << 8; i++) {
// 		rank_1_bst [i] = (i & 1) + rank_1_bst [i / 2];
// 	}
// }





#endif	/*	_IBIT_VECTOR_H_	*/


