//#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "./sin.h"
#include "./instruments/instruments.h"
const FREQ FREQ_MAX = (FREQ)0 - (FREQ)1;
const FREQ FREQ_MID = FREQ_MAX - (FREQ_MAX >> 1);
static const AMP MYPI = M_PI / (FREQ)FREQ_MID;

static FREQ tuning[12] = { 0x002d9000,0x00300000,0x00334200,0x00360000,0x0038f400,0x003cc000,0x00401280,0x00445800,0x00480000,0x004bf000,0x00510000,0x00556e00 };

#define NOTE_MAX 64
#define HM_MAX 1024
typedef struct hm_tag HM;

struct hm_tag
{
	HM*next;
	AMP aa;
	int n;
};

HM hm_null;
HM hm_pool[NOTE_MAX * HM_MAX];
HM* hm_gc[NOTE_MAX * HM_MAX];
HM** hm_top = hm_gc + NOTE_MAX * HM_MAX;

HM* hm_alloc(void)
{
	if (hm_top == hm_gc)
	{
		return &hm_null;
	}
	else
	{
		return  *(--hm_top);
	}
}

void hm_free(HM* p)
{
	*(hm_top++) = p;
}

typedef struct osc_tag OSC;

struct osc_tag
{
	OSC* next;
	FREQ f;
	AMP a;
	HM* hm;	
};

OSC osc_null;
OSC osc_pool[NOTE_MAX * HM_MAX];
OSC* osc_gc[NOTE_MAX * HM_MAX];
OSC** osc_top = osc_gc + NOTE_MAX * HM_MAX;

OSC* osc_alloc(void)
{
	if (osc_top == osc_gc)
	{
		return &osc_null;
	}
	else
	{
		return  *(--osc_top);
	}
}

void osc_free(OSC* p)
{
	*(osc_top++) = p;
}

OSC* start = &osc_null;

void dump(void)
{
	OSC* p = start;
	while (p != &osc_null)
	{
		printf("\nf:%d,a:%f",p->f,p->a);
		HM* q = p->hm;
		while (q != &hm_null)
		{
			printf("\tn:%d,aa:%f:",q->n,q->aa);
			q = q->next;
		}
		p = p->next;
	}
	putchar('\n');
}
		
double S(void)
{
	static FREQ t = 0;
	AMP out = 0;
	OSC* p = start;
	while (p != &osc_null)
	{
		out += p->a * sin((p->f * t) * MYPI);
		p = p->next;
	}
	++t;
	return (double)out;
}

int osc_on(FREQ f1, AMP aa, INSTRUMENT inst, int n)
{
	OSC** p = &start;
	for (FREQ f = f1; f  < FREQ_MID && f / f1 <= HM_MAX;)
	{
		if (*p == &osc_null || (*p)->f >= f)
		{
			if (*p == &osc_null || (*p)->f > f)
			{
				OSC* tmp = *p;
				*p = osc_alloc();
				if (*p == &osc_null)
				{
					return -1;
				}
				(*p)->next = tmp;
				(*p)->f = f;
				(*p)->hm = &hm_null;
			}
			HM** q = &(*p)->hm;
			AMP aa_sum = 0;
			while (*q != &hm_null)
			{
				aa_sum += (*q)->aa;
				q = &(*q)->next;
			}
			*q = hm_alloc();
			if (*q == &hm_null)
			{
				return -1;
			}
			(*q)->next = &hm_null;
			(*q)->n = n;
			(*q)->aa = inst(f, f1, aa);
			(*p)->a = sqrt(aa_sum + (*q)->aa);
			f += f1;
		}
		p = &(*p)->next; 
	}
	return 0;
}

int osc_off(FREQ f1, int n)
{
	OSC** p = &start;
	for (FREQ f = f1; f  < FREQ_MID && f / f1 <= HM_MAX;)
	{
		if (*p == &osc_null || (*p)->f > f)
		{
			return -1;
		}
		else if ((*p)->f == f)
		{
			HM** q = &(*p)->hm;
			AMP aa_sum = 0;
			while (*q != &hm_null)
			{
				if ((*q)->n == n)
				{
					HM* tmp = *q;
					*q = (*q)->next;
					hm_free(tmp);
				}
				else
				{
					aa_sum += (*q)->aa;
					q = &(*q)->next;
				}
			}
			f += f1;
			(*p)->a = sqrt(aa_sum);
		}
		if ((*p)->hm == &hm_null)
		{
			OSC* tmp = *p;
			*p = (*p)->next;
			osc_free(tmp);
		}
		else
		{
			p = &(*p)->next;
		}
	}
	return 0;
}

typedef struct note_tag NOTE;

struct note_tag
{
	NOTE* next;
	int n;
	int ch;
	int id;
	FREQ f;
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
		return  *(--note_top);
	}
}

void note_free(NOTE* p)
{
	*(note_top++) = p;
}

static NOTE* start_note = &note_null;

int N_on(int note, int velocity, int channel)
{
	static int id =0;
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
	(*end)->id = id;
	(*end)->f = tuning[note % 12] << ((note - 24) / 12); 
	AMP aa = velocity;
	INSTRUMENT inst = select_instrument(note, velocity, channel);
	if (osc_on((*end)->f, aa, inst, id++))
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
			FREQ f = (*p)->f;
			int id = (*p)->id;
			NOTE* tmp = *p;
			*p = (*p)->next;
			note_free(tmp);
			if(osc_off(f, id))
			{
				return -1;
			}
			return 0;
		}
		else
		{
			p = &(*p)->next;
				++i;
		}
	}
	return -1;
}

int initiate(void)
{
	for (int i = 0; i < NOTE_MAX * HM_MAX; ++i)
	{
		hm_gc[i] = &hm_pool[i];
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
