#include <math.h>
#include <string.h>

#include "data.h"

#ifndef NAN
#define NAN (0.0/0.0)
#endif

transformT   transform[MAX_TRANSFORMS];
int          transforms=0;

void	malloc_transform_data (transformT *t, unsigned long rows)
{
	if (!t->data_out)
	{
		t->data_out = (double *) malloc (sizeof (double) * rows);
		if (!t->data_out)
		{
			fprintf (stderr, "Couldn't alloc transform data\n");
			exit (-1);
		}
	}
}


void	apply (transformT *t);

void	get_transform_data (transformT *t, double **data, statsT **stats)
{
	if (t->prev == NULL)
	{
		// Get the original data
		*data = t->column->orig_data;
		*stats = &t->column->orig_stats;
	} else
	{
		if (!t->prev->data_out) 
		{
			// recurse
			if (t->prev->data_out) free (t->prev->data_out);		
			apply (t->prev);
		}
		*data = t->prev->data_out;
		*stats = &t->prev->stats_out;
	}
}

void	apply (transformT *t)
{
	double	*data;
	statsT	*stats;

	malloc_transform_data (t, t->column->frame->rows);	

	get_transform_data (t, &data, &stats);
	t->apply(t, data, stats);
}


void	apply_log (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	for (i=0; i<t->column->frame->rows; i++) t->data_out[i] = log(data[i]);
}

void	apply_sqr (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	for (i=0; i<t->column->frame->rows; i++) t->data_out[i] = data[i] * data[i];
}

void	apply_abs (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	for (i=0; i<t->column->frame->rows; i++) t->data_out[i] = fabs(data[i]);
}

void	apply_recip (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	for (i=0; i<t->column->frame->rows; i++) t->data_out[i] = 1.0 / data[i];
}

void	apply_diff_before (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	t->data_out[0] = NAN;
	for (i=1; i<t->column->frame->rows; i++) t->data_out[i] = data[i] - data[i-1];
}

void	apply_pct_change (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	t->data_out[0] = NAN;
	for (i=1; i<t->column->frame->rows; i++) t->data_out[i] = (data[i] / data[i-1]) - 1.0;
}

void	apply_diff_after (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	t->data_out[t->column->frame->rows-1] = NAN;
	for (i=0; i<t->column->frame->rows-1; i++) t->data_out[i] = data[i+1] - data[i];
}





void	new_transform (char key, char *name, char is_monotonic, void (*apply)(struct transformT *, double *, statsT *))
{
	transformT 	*t = &transform[transforms++];

	memset(t, 0, sizeof (transformT));
	t->key = key;
	t->name = strdup (name);
	t->is_monotonic = is_monotonic;
	t->param = NULL;
	t->apply = apply;
	t->prev = t->next = NULL;
}

void	init_transforms (void)
{
	new_transform('l', "log", 1, apply_log);	
	new_transform('s', "square", 0, apply_sqr);	
	new_transform('a', "absolute", 0, apply_abs);	
	new_transform('p', "% change", 0, apply_diff_before);	
	new_transform('b', "diff (before)", 0, apply_diff_before);	
	new_transform('d', "diff (after)", 0, apply_diff_after);	
	new_transform('r', "reciprocal", 1, apply_recip);	
	
}

transformT	*clone_transform (transformT *t)
{
	transformT	*r;

	r = (transformT *) malloc (sizeof(transformT));
	memcpy (r, t, sizeof(transformT));
	return (r);
} 

void	column_apply_transforms (frameT *f, int col)
{
	columnT	*c = f->column[col];

	// Simply apply the last transform -- the get_transform_data() chain should apply the needed ones, if any
	if (c->transform) apply (c->transform);

	// TODO: For monotonic functions, I shouldn't need to do a full update!
	f->column[col]->orig_stats.histogram.bins = 0;        // Force recalc of histogram
	update_column_stats (f, col);
}
