#include <math.h>
#include "../sin.h"
#include "./instruments.h"

AMP sawtooth(FREQ f, FREQ f1, AMP aa)
{
	AMP ret = (AMP)f1 / (AMP)f;
	ret *= ret;
	return ret * aa;
}

AMP square(FREQ f, FREQ f1, AMP aa)
{
	if ((f / f1) & 1)
	{
		AMP ret = (AMP)f1 / (AMP)f;
		ret *= ret;
		return ret * aa;
	}
	else
	{
		return 0;
	}
}

INSTRUMENT select_instrument(int note, int velocity, int channel)
{
	if(channel & 1)
	{
		return square;
	}
	else
	{
		return sawtooth;
	}
}
