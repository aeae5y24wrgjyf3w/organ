#include <math.h>
#include "../sin.h"
#include "./instruments.h"

AMP formant(AMP f, AMP c, AMP w, AMP g)
{
	AMP x = (f - c) / w;
	return g * exp(-0.5 * x * x);
}

#define HZ(x) (x / 24000.0)

AMP regi_3(FREQ f, FREQ f1, AMP aa)
{
	FREQ r = f / f1;
	AMP ret = 1 / (AMP)r;
	if (r & 1)
	{
		ret *= 1;
	}
	else if (r & 2)
	{
		ret *= 2;
	}
	else
	{
		ret *= 4;
	}
	ret *= ret * aa *
		(
			formant((AMP)f / (AMP)FREQ_MID, HZ(800.0), HZ(4000.0), 0.75)
			+ formant((AMP)f / (AMP)FREQ_MID, HZ(1200.0), HZ(6000.0), 0.25)
			//+ formant((AMP)f / (AMP)FREQ_MID, HZ(2400.0), HZ(12000.0), 0.0625)
			);

	return ret;
}

AMP regi_4(FREQ f, FREQ f1, AMP aa)
{
	FREQ r = f / f1;
	AMP ret = 1 / (AMP)r;
	if (r & 1)
	{
		ret *= 1;
	}
	else if (r & 2)
	{
		ret *= 2;
	}
	else if (r & 4)
	{
		ret *= 4;
	}
	else
	{
		ret *= 8;
	}
	ret *= ret * aa *
		(
			formant((AMP)f / (AMP)FREQ_MID, HZ(800.0), HZ(4000.0), 0.75)
			+ formant((AMP)f / (AMP)FREQ_MID, HZ(1200.0), HZ(6000.0), 0.25)
			//+ formant((AMP)f / (AMP)FREQ_MID, HZ(2400.0), HZ(12000.0), 0.0625)
			);
	return ret;
}
INSTRUMENT select_instrument(int note, int velocity, int channel)
{
	if (channel == 2)
		return regi_4;
	else
		return regi_3;
}
