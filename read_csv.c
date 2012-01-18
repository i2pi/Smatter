#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define __USE_XOPEN
#include <time.h>

#include "mba_csv.h"

#include "data.h"

#define CSV_BUF_SIZE	16384
#define MAX_COLUMNS		1024

// TODO: Check for times following the date
char	*date_format[] = {"%Y-%m-%d", "%d/%m/%Y", "%m/%d/%Y"};
int		date_formats = 2;

unsigned char	nlo[256];	// lookup take for number of leaading ones

void	nlo_init (void)
{
	int				i;
	unsigned char	x;
	unsigned char	mask;

	for (i=0; i<256; i++)
	{
		x = i;
		mask = 1 << 7;
		nlo[i] = 0;
		while (mask && (x & mask)) { mask >>= 1; nlo[i]++; }
	}
}

int is_date (char *buf)
{
	struct tm	t;
	int			i;
	char		*end;	

	i = 0;
	while ( (i < date_formats) && !(end = strptime(buf, date_format[i], &t)) ) i++;
	if (end) return (i);

	return (-1);
}


dtype guess_type (char *buf)
{
	int		len;
	char	*end;
	float	x;
	dtype	ret;
	
	len = strlen (buf);

	x = strtod (buf, &end);
	if (end != &buf[len])
	{
		// Not parsable as a float
		// Could be a date or a factor
		if (is_date(buf) != -1)
		{
			// Its a date
			ret = Date;
		} else
		{
			// A factor
			ret = Factor;
		}
	} else
	{
		ret = Numeric;
	}

	return (ret);	
}

char	*type_name (typeT *t)
{
	switch (t->type)
	{
		case Numeric:	return ("Numeric"); break;
		case Factor:	return ("Factor"); break;
		case Date:		return ("Date"); break;
		default: return ("!! UNKNOWN !!");
	}
}

void	print_frame (frameT *frame)
{
	int	n, i;
	int ncol = frame->columns;
	columnT	*c ;

	for (i=0; i<ncol; i++)
	{
		c = frame->column[i];
		printf ("%s (%s)%c ", c->name, type_name(&c->type), i < ncol-1 ? ',' : ' ');
	}
	printf ("\n");

	for (n=0; n<frame->rows; n++)
	{
		for (i=0; i<ncol; i++)
		{
			char	str[256];
			double	d;

			c = frame->column[i];
			if (c->transform) d = c->transform->data_out[n]; else d = c->orig_data[n];

			c->type.to_string (&c->type, &d, str);
			printf ("%s%c", str, i < ncol-1 ? ',' : ' ');
		}
		printf ("\n");
	}
}

void	estimate_csv (csvT *csv)
{
	double	row_len_sum = 0;
	double	samples = 0;	
	char	buf[MAX_CSV_LINE_LENGTH];

	int		samples_to_take = 1000;	

	while (samples < samples_to_take)
	{
		off_t	off;
		off = (random() / (float)RAND_MAX) * csv->size;
		fseek (csv->fp, off, SEEK_SET);
		// now seek to the next newline
		fgets (buf, MAX_CSV_LINE_LENGTH, csv->fp);	
		// now get a line
		fgets (buf, MAX_CSV_LINE_LENGTH, csv->fp);	
		// TODO: Should use this time to also work out the column types
		row_len_sum += strlen(buf);
		samples++;
	}
	csv->est_row_size = row_len_sum / samples;
	csv->est_rows = csv->size * samples / (float) row_len_sum;
	fseek (csv->fp, 0L, SEEK_SET);
}

csvT	*open_csv (char *file_name)
{
	csvT	*csv;
	struct stat st;

	csv = (csvT *) malloc (sizeof (csvT));
	if (!csv)
	{
		fprintf (stderr, "Failed to alloc csv\n");
		exit (-1);
	}

	csv->filename = strdup(file_name);
	csv->fp = fopen (file_name, "r");
	if (!csv->fp)
	{
		fprintf (stderr, "Failed to open '%s'\n", file_name);
		exit (-1);
	}

	stat(file_name, &st);
	csv->size = st.st_size;

	estimate_csv (csv);

	csv->bytes_per_bitmap_bit = ceil (csv->size / (BITMAP_SIZE * 8.0));
	if (csv->bytes_per_bitmap_bit < 4 * csv->est_row_size)
	{
		csv->bytes_per_bitmap_bit = 4 * csv->est_row_size;
	}
	csv->bitmap_size = ceil (csv->size / (csv->bytes_per_bitmap_bit * 8.0));

	csv->bitmap_display = (unsigned char *) malloc (csv->bitmap_size);
	csv->bitmap_loaded = (unsigned char *) malloc (csv->bitmap_size);
	if (!csv->bitmap_loaded || !csv->bitmap_display)
	{
		fprintf (stderr, "Failed to alloc bitmap\n");
		exit (-1);
	}
	memset (csv->bitmap_display, 0, csv->bitmap_size);
	memset (csv->bitmap_loaded, 0, csv->bitmap_size);

	return (csv);	
}

void load_row (frameT *frame)
{
	csvT	*csv = frame->csv;
	int		bytes;
	int		i, cols;
	unsigned char	buf[CSV_BUF_SIZE];
	unsigned char	*row[MAX_COLUMNS];

	bytes = csv_row_fread (csv->fp, buf, CSV_BUF_SIZE, row, frame->columns, ',', CSV_TRIM | CSV_QUOTES, &cols);
	if (bytes < 0)
	{
		fprintf (stderr, "Failed to read csv line!\n");
		exit (-1);
	}

	if (cols != frame->columns)
	{
		fprintf (stderr, "Expected %d columns, got %d\n", frame->columns, cols);
		for (i=0; i<cols; i++) printf ("%s ", row[i]);
		printf ("\n");
		exit (-1);
	}

	// TODO: UPDATE ESTIMATES

	for (i=0; i<cols; i++) column_add_data (frame, i, (char *)row[i]);
	frame->rows++;
	if (frame->rows >= frame->allocated_rows * 0.8)
	{
		unsigned long new_alloc = frame->rows * 1.5;

		for (i=0; i<cols; i++) column_realloc_data (frame, i, new_alloc);

		free (frame->region_rows);
		frame->region_rows = calloc (new_alloc, sizeof(unsigned char));
		if (!frame->region_rows)
		{
			fprintf (stderr, "Failed to realloc region_rows\n");
			exit (-1);
		}
		frame->allocated_region_rows = new_alloc;

		frame->allocated_rows = new_alloc;
	}

	// We don't over-allocate transform data, so we must force reallocs whenever
	// we load new data.	
	for (i=0; i<cols; i++) column_wipe_transforms (frame, i);

	if (frame->nn_distance)
	{
		free (frame->nn_distance);
		frame->nn_distance = NULL;
	}
}

void 	load_offset_row (frameT *frame, off_t off, off_t end)
{
	csvT	*csv = frame->csv;
	char	buf[CSV_BUF_SIZE];
	unsigned char c;

	if (off == 0)
	{
		// skip the header
		fseek(csv->fp, 0L, SEEK_SET);
		fgets (buf, CSV_BUF_SIZE, csv->fp);
		off = ftell(csv->fp);
		if (off >= end)	return;
	} 

	if (off >= csv->size)
	{
		// don't go past the end of the file
		return;
	}
	
	// we go to off - 1 to check and see whether the previous
	// character is '\n', if so we happen to be aligned to a
	// row.

	fseek (csv->fp, off - 1, SEEK_SET);
	c = fgetc (csv->fp);
	if (c == '\n') 
	{
		load_row (frame);
		return;
	}

	// need to align to the next '\n';
	fgets (buf, CSV_BUF_SIZE, csv->fp);
	if (ftell(csv->fp) >= end) 
	{
		// Beyond the end of where we should be looking, so don't look
		return;
	}
	load_row (frame);
}

void	update_all_stats (frameT *frame)
{
	int	i;
	for (i=0; i<frame->columns; i++)
	{
		frame->column[i]->orig_stats.histogram.bins = 0;	// Force recalc of histogram
		update_column_stats (frame, i);
	}
}

void load_random_rows (frameT *frame, float pct)
{
	// This finds a series of empty bits in the bitmap and loads the
	// rows that correspond to the bytes in the full csv file. As rows
	// are loaded, bits are set in the bitmap to ensure that the same
	// row isn't loaded multiple times.

	int		i;
	long	rows_to_load;
	csvT	*csv = frame->csv;

	rows_to_load = csv->est_rows * pct / 100.0;
	if (!rows_to_load)  rows_to_load = 1;

	for (i=0; i<rows_to_load; i++)
	{
		unsigned long	bitmap_byte, st;
		char	bitmap_bit;
		off_t	start, end;

		bitmap_byte = (random() / (float) RAND_MAX) * csv->bitmap_size;
		st = bitmap_byte;
		while ((bitmap_byte < csv->bitmap_size) && (csv->bitmap_loaded[bitmap_byte] == 0xFF)) bitmap_byte++;
		// TODO: this is probably why we can load more rows than exist.. bitmap may be bigger than file or something
		if (bitmap_byte >= csv->bitmap_size)
		{
			bitmap_byte = 0;
			while ((bitmap_byte < st) && (csv->bitmap_loaded[bitmap_byte] == 0xFF)) bitmap_byte++;
			if (bitmap_byte >= st)
			{
				// We have already loaded the entire file
				update_all_stats (frame);
				return ;
			}
		}
		bitmap_bit = nlo[csv->bitmap_loaded[bitmap_byte]];
		
		start = (bitmap_byte*8 + bitmap_bit) * csv->bytes_per_bitmap_bit;
		end   = (bitmap_byte*8 + bitmap_bit + 1) * csv->bytes_per_bitmap_bit;
		while ((start < end) && (start < csv->size))
		{
			load_offset_row (frame, start, end);
			start = ftell (frame->csv->fp);
		}
		csv->bitmap_loaded[bitmap_byte] |= 1 << bitmap_bit;
	}	

	update_all_stats (frame);

	return; 
}

void	load_all_rows (frameT *frame)
{
	csvT	*csv = frame->csv;
	char	buf[CSV_BUF_SIZE];

	fseek (csv->fp, 0L, SEEK_SET);
	// Skip header
	fgets (buf, CSV_BUF_SIZE, csv->fp);

	while (ftell (csv->fp) < csv->size)
	{
		load_row (frame);
	}	

	update_all_stats (frame);
}



frameT *read_csv (char *file_name)
{
	FILE			*fp;
	unsigned char	buf[CSV_BUF_SIZE];
	unsigned char	header_buf[CSV_BUF_SIZE];
	unsigned char	*row[MAX_COLUMNS];
	unsigned char	*header[MAX_COLUMNS];
	int				ncol, cols;
	int				n, i;
	frameT			*frame;
	csvT			*csv;

	csv = open_csv (file_name);
	fp = csv->fp;


	// Parse header
	n = csv_row_fread (fp, header_buf, CSV_BUF_SIZE, header, MAX_COLUMNS, ',', CSV_TRIM | CSV_QUOTES, &ncol);
	if (n <= 0)
	{
		fprintf (stderr, "Failed to read header from CSV file\n");
		exit (-1);
	}

	// Parse the first data row to determine data types

	n = csv_row_fread (fp, buf, CSV_BUF_SIZE, row, ncol, ',', CSV_TRIM | CSV_QUOTES, &cols);
	if (n <= 0)
	{
		fprintf (stderr, "File has no data\n");
		exit (-1);
	}
	frame = new_frame (file_name, ncol);
	frame->csv = csv;
	for (i=0; i<ncol; i++) init_column (frame, i, (char *)header[i], guess_type((char *) row[i]));

	for (i=0; i<ncol; i++) column_init_data (frame, i, csv->est_rows);
	frame->allocated_rows = csv->est_rows;
		
	frame->region_rows = (unsigned char *) malloc (frame->allocated_rows);
	if (!frame->region_rows)
	{
		fprintf (stderr, "Failed to allocate region_rows\n");	
		exit (-1);
	}
	frame->allocated_region_rows = frame->allocated_rows;

	if (csv->est_rows < 10000)
	{
		load_all_rows (frame);
	} else
	{
		load_random_rows (frame, 1.0);
	}

	return (frame);
}
