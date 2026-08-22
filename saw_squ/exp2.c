//#include <stdlib.h>
#include <stdio.h>
#include <math.h>
typedef unsigned int FREQ;
static const FREQ FREQ_MAX = (FREQ)0 - (FREQ)1;
static const FREQ FREQ_MID = FREQ_MAX - (FREQ_MAX >> 1);
typedef double AMP;
static const AMP AMP_MAX = 0x1p32;
static const AMP MYPI = M_PI / (FREQ)FREQ_MID;

static FREQ tuning[12] = { 0x002d9000,0x00300000,0x00334200,0x00360000,0x0038f400,0x003cc000,0x00401280,0x00445800,0x00480000,0x004bf000,0x00510000,0x00556e00 };

#define NOTE_MAX 16
typedef struct osc_tag OSC;

struct osc_tag
{
	OSC* next;
	FREQ f;
	FREQ rank;
	AMP aa_sum;
	AMP b;
};

OSC osc_pool[1 << NOTE_MAX];
OSC* osc_gc[1 << NOTE_MAX];
OSC** osc_top = osc_gc + (1 << NOTE_MAX);

OSC* osc_alloc(void)
{
	if (osc_top == osc_gc)
	{
		return NULL;
	}
	else
	{
		return *(--osc_top);
	}
}

void osc_free(OSC* p)
{
	*(osc_top++) = p;
}

static OSC zero = { NULL,1,FREQ_MAX,0,0 };

struct
{
	FREQ t;
	OSC* o;
}organ = { 0,&zero };

double S(void)
{
	AMP out = 0;
	OSC* p = organ.o;
	const AMP filter = -0x42 / 4.0 / (AMP)FREQ_MID;
	while (p != NULL)
	{
		FREQ phase = p->f * organ.t;
		AMP r = exp(filter * p->f);
                AMP w = r
                        * (1 + r) * sin(phase * MYPI)
                        / (1 - 2 * r * cos(phase * MYPI) + r * r);
		if (!(p->rank & FREQ_MID))
		{
			AMP r = exp(filter * 2.0 * p->f);
			w -= r
				* (1 + r) * sin((phase * 2) * MYPI)
				/ (1 - 2 * r * cos((phase * 2) * MYPI) + r * r);
		}
		out += w * p->b;
		p = p->next;
	}
	++organ.t;
	return (double)out;
}

//ユークリッドの互除法を用いてmとnの最大公約数を求める関数
FREQ euc(FREQ m, FREQ n)
{
	if (n == 0)
	{
		return m;
	}
	FREQ r = m % n;
	if (r)
	{
		return euc(n, r);
	}
	else
	{
		return n;
	}
}

int osc_on(FREQ f, AMP aaff, int if_saw)
{
	FREQ rank;
	if (if_saw)
	{
		rank = FREQ_MAX;
	}
	else
	{
		rank = 1;
	}
	FREQ rank_test = 1;
	while (!(rank_test & f))
	{
		rank <<= 1;
		rank_test <<= 1;
	}
	OSC** mid = &organ.o;
	while (*mid != NULL)
	{
		mid = &(*mid)->next;
	}
	OSC** p = mid;
	OSC* q = organ.o;
	int j = 0;
	while (q != *mid)
	{
		*p = osc_alloc();
		if (*p == NULL)
			return -1;
		FREQ tmpf = f / euc(f, q->f) * q->f;
		AMP tmpd = (AMP)f / (AMP)euc(f, q->f) * (AMP)q->f / 2;
		if (tmpf < tmpd)
		{
			(*p)->f = FREQ_MAX;
			(*p)->rank = 0;
			(*p)->aa_sum = 0;
			(*p)->b = 0;
		}
		else
		{
			(*p)->f = tmpf;
			(*p)->aa_sum = q->aa_sum + aaff;
			(*p)->rank = rank & q->rank;
			if ((*p)->rank)
			{
				(*p)->b = sqrt((*p)->aa_sum) - sqrt(q->aa_sum);
				OSC* r = *mid;
				int k = 0;
				while (r != *p)
				{
					if (!(~j & k++))
					{
						(*p)->b -= r->b;
					}
					r = r->next;
				}
			}
			else
			{
				(*p)->b = 0;
			}
		}
		q = q->next;
		p = &(*p)->next;
		*p = NULL;
		++j;
	}
	return 0;
}

void osc_off(int i)
{
	OSC** p = &organ.o;
	int j = 0;
	while (*p != NULL)
	{
		if (j & (1 << i))
		{
			OSC* tmp = *p;
			*p = (*p)->next;
			osc_free(tmp);
		}
		else
		{
			p = &(*p)->next;
		}
		++j;
	}
	return;
}

typedef struct note_tag NOTE;

struct note_tag
{
	NOTE* next;
	int n;
	int ch;
};

NOTE note_pool[NOTE_MAX];
NOTE* note_gc[NOTE_MAX];
NOTE** note_top = note_gc + NOTE_MAX;

NOTE* note_alloc(void)
{
	if (note_top == note_gc)
	{
		return NULL;
	}
	else
	{
		return *(--note_top);
	}
}

void note_free(NOTE* p)
{
	*(note_top++) = p;
}

static NOTE* start_note = NULL;

int N_on(int note, int velocity, int channel)
{
	int if_saw = 1;
	if ((channel & 1) != 0)
	{
		if_saw = 0;
	}
	NOTE** end = &start_note;
	while (*end != NULL)
	{
		end = &(*end)->next;
	}
	*end = note_alloc();
	if (*end == NULL)
	{
		return -1;
	}
	(*end)->next = NULL;
	(*end)->n = note;
	(*end)->ch = channel;
	FREQ f = tuning[note % 12] << ((note - 24) / 12);
	AMP aa = exp((-0x41 * (note - 24) + -0xb7 * (84 - note)) /16.0 / (AMP)(84 - 24)) * (AMP)FREQ_MID * (AMP)FREQ_MID;
	if (osc_on(f, aa, if_saw))
	{
		return -1;
	}
	return 0;
}

int N_off(int note, int velocity, int channel)
{
	NOTE** p = &start_note;
	int i = 0;
	while (*p != NULL)
	{
		if ((*p)->n == note && (*p)->ch == channel)
		{
			NOTE* tmp = *p;
			*p = (*p)->next;
			note_free(tmp);
			osc_off(i);
			return 0;
		}
		else
		{
			p = &(*p)->next;
		}
		++i;
	}
	return -1;
}

int initiate(void)
{
	for (int i = 0; i < (1 << NOTE_MAX); ++i)
	{
		osc_gc[i] = &osc_pool[i];
	}
	for (int i = 0; i < NOTE_MAX; ++i)
	{
		note_gc[i] = &note_pool[i];
	}
	return 0;
}

int terminate(void)
{
	return 0;
}
