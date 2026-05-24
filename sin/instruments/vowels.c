#include <math.h>
#include "../sin.h"
#include "./instruments.h"

AMP formant(AMP f, AMP c, AMP w, AMP g)
{
	AMP x = (f - c) / w;
	return g * exp(-0.5 * x * x);
}

#define HZ(x) (x / 24000.0)

AMP vowel_a(FREQ f, FREQ f1, AMP aa)
{
	AMP r = (AMP)f1 / (AMP)f;
	return aa * r * r *
		(
		 1
		 *formant((AMP)f / (AMP)FREQ_MID, HZ(1200.0), HZ(2000.0) , 1.0)
		 //+formant((AMP)f / (AMP)FREQ_MID, HZ(1200.0), HZ(200.0), 0.5)
		 //+formant((AMP)f / (AMP)FREQ_MID, HZ(2500.0), HZ(300.0), 0.25)
		);
}

INSTRUMENT select_instrument(int note, int velocity, int channel)
{
	return vowel_a;
}
