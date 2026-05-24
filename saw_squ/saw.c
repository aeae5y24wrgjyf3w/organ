#include <math.h>
typedef unsigned int FREQ;
static const FREQ FREQ_MAX = (FREQ)0 - (FREQ)1;
static const FREQ FREQ_MID = FREQ_MAX - (FREQ_MAX >> 1);
typedef double AMP;
static const AMP AMP_MAX = 0x1p32;

static FREQ tuning[12] = { 0x002d9000,0x00300000,0x00334200,0x00360000,0x0038f400,0x003cc000,0x00401280,0x00445800,0x00480000,0x004bf000,0x00510000,0x00556e00 };

#define NOTE_MAX 16
typedef struct osc_tag OSC;

struct osc_tag
{
	OSC* next;
	FREQ f;
	AMP aaff_sum;
	AMP bf;
	AMP b;
};

OSC osc_null;
OSC osc_pool[1 << NOTE_MAX];
OSC* osc_gc[1 << NOTE_MAX];
OSC** osc_top = osc_gc + (1 << NOTE_MAX);

OSC* osc_alloc(void)
{
	if (osc_top == osc_gc)
	{
		return &osc_null;
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

OSC zero = { &osc_null,1,FREQ_MAX,0,0 };

struct
{
	FREQ t;
	OSC* o;
}organ = { 0,&zero };

double S(void)
{
	AMP out = 0;
	OSC* p = organ.o;
	while (p != &osc_null)
	{
		FREQ saw = p->f * organ.t;
		AMP tmp = saw ? (AMP)saw - (AMP)((FREQ)0 - saw) : -AMP_MAX;
		out += tmp * p->b;
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

int osc_on(FREQ f, AMP aaff)
{
	OSC** mid = &organ.o;
	while (*mid != &osc_null)
	{
		mid = &(*mid)->next;
	}
	OSC** p = mid;
	OSC* q = organ.o;
	int j = 0;
	while (q != *mid)
	{
		*p = osc_alloc();
		if (*p == &osc_null)
			return -1;
		FREQ tmpf = f / euc(f, q->f) * q->f;
		AMP tmpd = (AMP)f / (AMP)euc(f, q->f) * (AMP)q->f / 2;
		if (tmpf < tmpd)
		{
			(*p)->f = FREQ_MAX;
			(*p)->aaff_sum = 0;
			(*p)->bf = 0;
			(*p)->b = 0;
		}
		else
		{
			(*p)->f = tmpf;
			(*p)->aaff_sum = q->aaff_sum + aaff;
			(*p)->bf = sqrt((*p)->aaff_sum) - sqrt(q->aaff_sum);
			OSC* r = *mid;
			int k = 0;
			while (r != *p)
			{
				if (!(~j & k++))
				{
					(*p)->bf -= r->bf;
				}
				r = r->next;
			}
			(*p)->b = (*p)->bf / (*p)->f;
		}
		q = q->next;
		p = &(*p)->next;
		*p = &osc_null;
		++j;
	}
	return 0;
}

void osc_off(int i)
{
	OSC** p = &organ.o;
	int j = 0;
	while (*p != &osc_null)
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

NOTE note_null;
NOTE note_pool[NOTE_MAX];
NOTE* note_gc[NOTE_MAX];
NOTE** note_top = note_gc + NOTE_MAX;

NOTE* note_alloc(void)
{
	if (note_top == note_gc)
	{
		return &note_null;
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

static NOTE* start_note = &note_null;

int N_on(int note, int velocity, int channel)
{
	NOTE** end = &start_note;
	while (*end != &note_null)
	{
		end = &(*end)->next;
	}
	*end = note_alloc();
	if (*end == &note_null)
	{
		return -1;
	}
	(*end)->next = &note_null;
	(*end)->n = note;
	(*end)->ch = channel;
	FREQ f = tuning[note % 12] << ((note - 24) / 12);
	AMP af = sqrt(velocity) * (AMP)f;
	if (osc_on(f, af * af))
	{
		return -1;
	}
	return 0;
}

int N_off(int note, int velocity, int channel)
{
	NOTE** p = &start_note;
	int i = 0;
	while (*p != &note_null)
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
