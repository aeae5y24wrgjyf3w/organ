#include <math.h>
#include "../sin.h"
#include "./instruments.h"

AMP sawtooth(FREQ f, FREQ f1, AMP aa)
{
	AMP ret = (AMP)f1 / (AMP)f;
	ret *= ret;
	return ret * aa;
}

INSTRUMENT select_instrument(int note, int velocity, int channel)
{
	return sawtooth;
}
