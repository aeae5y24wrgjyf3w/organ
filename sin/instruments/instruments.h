#include "../sin.h"

typedef AMP (*INSTRUMENT)(FREQ, FREQ, AMP);

INSTRUMENT select_instrument(int note, int velocity, int channel);
