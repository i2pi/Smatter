#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define __USE_XOPEN
#include <time.h>

#include "data.h"

typeT	type[MAX_TYPES];
int		types;

void	date_from_string (struct typeT *self, char *str, double *x)
{
	struct tm t;

	if (!self->data)
	{
		// We use the data section to hold the format string.
		int	f = is_date(str);
		if (f == -1)
		{
			fprintf (stderr, "'%s' is not a Date\n", str);
			exit (-1);
		}

		self->data = (void *) strdup (date_format[f]);
	}

	memset (&t, 0, sizeof(struct tm));
	strptime (str, (char *) self->data, &t);
	*x  = (double) mktime (&t);
}

void	date_to_string (struct typeT *self, double *x, char *str)
{
	struct tm t;
	time_t	tt = (time_t) *x;
	localtime_r (&tt, &t);
	strftime(str, 100, (char *)self->data, &t);
}

void numeric_from_string (struct typeT *self, char *str, double *x)
{
	*x = atof(str);
}

void numeric_to_string (struct typeT *self, double *x, char *str)
{
	snprintf (str, 256, "%.6f", *x);
}

void factor_to_string (struct typeT *self, double *x, char *str)
{
	unsigned long i = (unsigned long) *x;
	factorT	*f;

	f = (factorT *) self->data;
	if (i < f->items)
	{
		snprintf (str, 256, "%s", f->item[i]);
	} else
	{
		snprintf (str, 256, "NA!!");
	}
}

void factor_from_string (struct typeT *self, char *str, double *x)
{
	factorT	*f;
	int		i;

	if (!self->data)
	{
		f = (factorT *) malloc (sizeof(factorT));
		if (!f)
		{
			fprintf (stderr, "Failed to alloc factor\n");
			exit (-1);
		}
		f->items = 0;
		f->allocated = 32;
		f->item = (char **) malloc (sizeof (char *) * f->allocated);
		if (!f->item)
		{
			fprintf (stderr, "Failed to alloc factor list\n");
			exit (-1);
		}

		self->data = (void *) f;
	} else
	{
		f = (factorT *) self->data;
	}

	i = f->items - 1;
	while ((i >= 0) && strcmp(f->item[i], str)) i--;

	if (i < 0)
	{
		if (f->items >= f->allocated)
		{
			char	**new;
			f->allocated += (f->allocated >> 1) + 1;
			new = realloc (f->item, sizeof (char *) * f->allocated);
			if (!new)
			{
				fprintf (stderr, "Failed to realloc factor array\n");
				exit (-1);
			}
			f->item = new;
		}
		f->item[f->items] = strdup(str);
		if (!f->item[f->items])
		{
			fprintf (stderr, "Failed to strdup factor\n");
			exit (-1);
		}
		*x = (double) f->items;
		f->items++;
	} else
	{
		*x = (double) i;
	}
}



void	new_type (typeT *t, dtype type, 
	              void (*from_string)(struct typeT *, char *, double *), 
	              void (*to_string)(struct typeT *, double *, char *))
{
	t->type = type;
	t->from_string = from_string;
	t->to_string = to_string;
	t->data = NULL; 
}

void	init_types (void)
{
	new_type(&type[Numeric], Numeric, numeric_from_string, numeric_to_string);
	new_type(&type[Factor], Factor, factor_from_string, factor_to_string);
	new_type(&type[Date], Date, date_from_string, date_to_string);
}


frameT	*new_frame (char *name, int columns)
{
	frameT	*f;
	int		i;

	f = (frameT *) malloc (sizeof(frameT));
	if (!f)
	{
		fprintf (stderr, "Failed to allocate frame\n");
		exit (-1);
	}

	f->columns = columns;
	f->column = (columnT **) malloc (sizeof (columnT *) * f->columns);
	if (!f->column)
	{
		fprintf (stderr, "Failed to alloc columns\n");
		exit (-1);
	}
	for (i = 0; i<columns; i++)
	{
		f->column[i] = (columnT *) malloc (sizeof (columnT));
		if (!f->column[i])
		{
			fprintf (stderr, "Failed to alloc column %d\n", i);
			exit (-1);
		}
		f->column[i]->name = NULL;
		f->column[i]->transform = NULL;
		f->column[i]->orig_data = NULL;
	}
	f->rows = 0;

	return (f);
}

void	free_frame (frameT *f)
{
	int	i;

	if (!f) return;

	for (i=0; i<f->columns; i++)
	{
		// TODO 
		// free_column (f->column[i]);
		free (f->column[i]);
	}
	free (f->column);

// TODO
//	free_csv (f->csv);
}


void	init_column (frameT *f, int i, char *name, dtype t)
{
	if (f->column[i]->name) free (f->column[i]->name);
	f->column[i]->name = strdup(name);	

	if (!f->column[i]->name)
	{
		fprintf (stderr, "Failed to init column name '%s'\n", name);
		exit (-1);
	}

	memcpy (&f->column[i]->type, &type[t], sizeof (typeT));

	if (f->column[i]->transform) 
	{
		// TODO
		// free_transform_chain (f->column[i]->transform);
		f->column[i]->transform = NULL;
	}	

	if (f->column[i]->orig_data)
	{
		free (f->column[i]->orig_data);
		f->column[i]->orig_data = NULL;
	}

	f->column[i]->frame = f;
}

void	column_realloc_data (frameT *f, int i, unsigned long rows)
{
	double	*new;
	columnT *c = f->column[i];

	new = (double *) realloc (c->orig_data, sizeof (double) * rows);
	if (!new)
	{
		fprintf (stderr, "Column realloc failed\n");
		exit (-1);
	}
	c->orig_data = new;
}

void	column_init_data (frameT *f, int i, unsigned long rows)
{
	columnT *c = f->column[i];

	if (c->orig_data)
	{
		free (c->orig_data);
	}
	c->orig_data = (double *) malloc (sizeof (double) * rows);
	if (!c->orig_data)
	{
		fprintf (stderr, "Failed to alloc column data\n");
		exit (-1);
	}

	memset (&c->orig_stats, 0, sizeof(statsT));
}

int		double_cmp (const void *va, const void *vb)
{
	double a = *(double *)va;
	double b = *(double *)vb;
	if (a < b) return (-1);
	if (a > b) return ( 1);
	return (0);
}

void	show_stats (statsT *s)
{
	int	b, d;

	printf ("Range: %f, %f\n", s->min, s->max);
	printf ("Mean (sdev): %f (%f)\n", s->mean, s->stddev);
	printf ("Quartiles: %f, %f, %f\n", s->quartile[0], s->quartile[1], s->quartile[2]);
	printf ("Cardinality: %ld\n", s->cardinality);

	printf ("Histogram: \n");
	for (b=0; b<s->histogram.bins; b++)
	{
		printf ("%9.4f: ", s->histogram.breaks[b]);
		for (d=0; d<s->histogram.counts[b] * 20.0 / (double) s->histogram.max_count; d++) printf ("*");
		printf (" [%ld]\n", s->histogram.counts[b]);
	}
}

void	update_stats (statsT *s, double *orig_data, unsigned long rows)
{
	histogramT		*h = &s->histogram;
	unsigned long	i;
	double			prev_x;
	double			*data;
	
	data = (double *) malloc (sizeof (double) * rows);
	if (!data)
	{
		fprintf (stderr, "Failed to alloc data buffer for sorting\n");
		exit (-1);
	}
	memcpy (data, orig_data, sizeof(double) * rows);

	// First sort, as we need this for median, quantiles & histogram
	// TODO: Should merge in new things rather than always qsorting
	qsort (data, rows, sizeof (double), double_cmp);

	s->min = data[0];
	s->max = data[rows-1];
	s->quartile[0] = data[(int)(rows/4.0)];
	s->quartile[1] = data[(int)(rows/2.0)];
	s->quartile[2] = data[(int)(3.0*rows/4.0)];
	
	s->sum = 0;
	s->sum_squares = 0;
	prev_x = s->min;
	s->cardinality = 1;
	for (i=0; i<rows; i++)
	{
		s->sum += data[i];
		s->sum_squares += data[i] * data[i];
		if (data[i] != prev_x)
		{
			s->cardinality++;
			prev_x = data[i];
		}
	}
	s->mean = s->sum / (double) rows;
	s->stddev = sqrt((s->sum_squares / (double) rows) - (s->mean * s->mean));

	if (!h->bins)
	{
		if (s->cardinality > DEFAULT_HIST_BINS)
		{
			// Even spacing
			int	b;

			h->bins = DEFAULT_HIST_BINS;
			h->breaks = (double *) malloc (sizeof (double) * h->bins);
			h->counts = (unsigned long *) malloc (sizeof (unsigned long) * h->bins);
			if (!h->breaks || !h->counts)
			{
				fprintf (stderr, "Failed to alloc histogram breaks\n");
				exit (-1);
			}	
			for (i=0; i<h->bins; i++)
			{
				h->breaks[i] = s->min + ((i+1) / (float) h->bins) * (s->max - s->min); 	
			}
			b = 0;
			memset (h->counts, 0, sizeof(unsigned long) * h->bins);
			h->max_count = 0;
			for (i=0; i<rows; i++)
			{
				if ((data[i] > h->breaks[b]) && (b < h->bins-1)) 
				{
					b++;
				}
				if (b >= h->bins) {printf ("HOLD YO HORSES |\n"); exit (-1);}
				h->counts[b]++;
				if (h->counts[b] > h->max_count) h->max_count = h->counts[b];
			}
		} else
		{
			// Uneven spacing - according to cardinality breaks
			int	b;

			h->bins = s->cardinality;
			h->breaks = (double *) malloc (sizeof (double) * h->bins);
			h->counts = (unsigned long *) malloc (sizeof (unsigned long) * h->bins);
			if (!h->breaks || !h->counts)
			{
				fprintf (stderr, "Failed to alloc histogram breaks\n");
				exit (-1);
			}	
		
			b = 0;
			h->breaks[b] = s->min;
			h->counts[b] = 1;
			h->max_count = 0;
			for (i = 1; i<rows; i++)
			{
				if (data[i] > h->breaks[b]) 
				{
					h->breaks[++b] = data[i];	
					h->counts[b] = 0;
				}
				if (b >= h->bins) {printf ("HOLD YO HORSES ||\n"); exit (-1);}
				h->counts[b]++;
				if (h->counts[b] > h->max_count) h->max_count = h->counts[b];
			}
		}
	}
}

void	column_add_data (frameT *f, int i, char *str)
{
	// Given a string piece of data, will convert to the
	// binary data type and add the data to the end of the column

	columnT *c = f->column[i];
	double	*d = c->orig_data;

	c->type.from_string (&c->type, str, &d[f->rows]);
}

void column_add_transform (columnT *c, transformT *t)
{
	// Transforms are kept in a LIFO linked list

	if (c->transform) c->transform->next = t;
	t->prev = c->transform;
	t->next = NULL;
	t->column = c;
	c->transform = t;
	t->data_out = (double *) malloc (sizeof (double) * c->frame->rows);
	if (!t->data_out)
	{
		fprintf (stderr, "Failed to alloc data out\n");
		exit (-1);
	}
	t->rows = c->frame->rows;
}
