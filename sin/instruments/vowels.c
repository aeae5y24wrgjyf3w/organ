#include <math.h>
#include "../sin.h"
#include "./instruments.h"

AMP formant(AMP f, AMP c, AMP w, AMP g)
{
	AMP x = (f - c) / w;
	return g * exp(-0.5 * x * x);
}

#define HZ(x) (x / 24000.0)

AMP vowel_female_a(FREQ f, FREQ f1, AMP aa)
{
	AMP r = (AMP)f1 / (AMP)f;
	return aa * r * r * 0.8 *
		(
		 1
		 *formant((AMP)f / (AMP)FREQ_MID, HZ(1000.0), HZ(150.0) , 1.0)
		 +formant((AMP)f / (AMP)FREQ_MID, HZ(1500.0), HZ(250.0), 0.7)
		 +formant((AMP)f / (AMP)FREQ_MID, HZ(3120.0), HZ(375.0), 0.4)
		);
}

AMP vowel_male_a(FREQ f, FREQ f1, AMP aa)
{
	AMP r = (AMP)f1 / (AMP)f;
	return aa * r * r * 
		(
		 1
		 *formant((AMP)f / (AMP)FREQ_MID, HZ(800.0), HZ(120.0) , 1.0)
		 +formant((AMP)f / (AMP)FREQ_MID, HZ(1200.0), HZ(200.0), 0.7)
		 +formant((AMP)f / (AMP)FREQ_MID, HZ(2500.0), HZ(300.0), 0.4)
		);
}

INSTRUMENT select_instrument(int note, int velocity, int channel)
{
	if (channel & 1)
		return vowel_male_a;
	else
		return vowel_female_a;
}
