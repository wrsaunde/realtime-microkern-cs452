#include <lib/all.h>

//return the square root of a number


int math_sqrt( int x ) {

	if( x < 0 ) {
		bwprintf(COM2, "\x1B[2J\x1B[HX IN SQRT [%d]", x);
		while(1);
		assert( 0, "TRIED TO SQUARE ROOT A NEGATIVE NUMBER" );
	}
	if( x == 0 ) {
		return 0;
	}

	int top = 1;
	int bottom = 0;

	//gallop search to find the an interval containing the sqrt
	while( top * top < x ) {
		bottom = top;
		top = top * 2;

		//make sure the square won't overflow
		if( top > 46340 ) {
			//if it will overflow, don't let it
			top = 46340;
			break;
		}
	}

	//if it would have caused overflow, just return the top
	//should almost never happen
	if( x >= top * top ) {
		return top;
	}


	//loop until we have zeroed in
	while( top - bottom > 1 ) {
		int mid = ((top - bottom) / 2) + bottom;
		int sqr = mid * mid;
		if( sqr == x ) {
			return mid;
		} else if( sqr < x ) {
			bottom = mid;
		} else {
			top = mid;
		}
	}

	int topdiff = x - (top * top);
	topdiff = ABS( topdiff );
	int botdiff = x - (bottom * bottom);
	botdiff = ABS( botdiff );

	if( topdiff < botdiff ) {
		return top;
	}

	return bottom;
}
