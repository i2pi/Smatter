#include <math.h>
#include <string.h>

#include "data.h"

transformT   transform[MAX_TRANSFORMS];
int          transforms=0;


void	get_data (transformT *t, double **data, statsT **stats)
{
	if (t->prev == NULL)
	{
		// Get the original data
		*data = t->column->orig_data;
		*stats = &t->column->orig_stats;
	} else
	{
		if ((t->prev->data_out) && (t->prev->rows == t->column->frame->rows))
		{
			// the predecessor has data, use it
			*data = t->prev->data_out;
			*stats = &t->prev->stats_out;
		} else
		{
			fprintf (stderr, "TODO: Apply chained transforms to find missing data\n");
			exit (-1);
		}
	}
}

void	apply (transformT *t)
{
	double	*data;
	statsT	*stats;

	get_data (t, &data, &stats);
	t->apply(t, data, stats);
}


void	apply_log (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	for (i=0; i<t->column->frame->rows; i++) t->data_out[i] = log(data[i]);
	// TODO: For monotonic functions, I shouldn't need to do a full update!
	update_stats (stats, t->data_out, t->column->frame->rows);
}

void	apply_sqr (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	for (i=0; i<t->column->frame->rows; i++) t->data_out[i] = data[i] * data[i];
	update_stats (stats, t->data_out, t->column->frame->rows);
}

void	apply_abs (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	for (i=0; i<t->column->frame->rows; i++) t->data_out[i] = fabs(data[i]);
	update_stats (stats, t->data_out, t->column->frame->rows);
}

void	apply_recip (transformT *t, double *data, statsT *stats)
{
	unsigned long i;
	for (i=0; i<t->column->frame->rows; i++) t->data_out[i] = 1.0 / data[i];
	update_stats (stats, t->data_out, t->column->frame->rows);
}


void	new_transform (char *name, char is_monotonic, void (*apply)(struct transformT *, double *, statsT *))
{
	transformT 	*t = &transform[transforms++];

	memset(t, 0, sizeof (transformT));
	t->name = strdup (name);
	t->is_monotonic = is_monotonic;
	t->apply = apply;
}

void	init_transforms (void)
{
	new_transform("log", 1, apply_log);	
	new_transform("sqr", 0, apply_sqr);	
	new_transform("abs", 0, apply_abs);	
	new_transform("recip", 1, apply_recip);	
	
}

transformT	*clone_transform (transformT *t)
{
	transformT	*r;

	r = (transformT *) malloc (sizeof(transformT));
	memcpy (r, t, sizeof(transformT));
	return (r);
} 

void	column_apply_transforms (columnT *c)
{
	// Simply apply the last transform

	if (c->transform)
	{
		apply (c->transform);
	}
}
