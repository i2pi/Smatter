#ifndef __DATA_H__
#define __DATA_H__
#include <stdio.h>
#include <stdlib.h>

#define MAX_TYPES 16
#define MAX_TRANSFORMS 64

#define MAX_CSV_LINE_LENGTH	16384
#define BITMAP_SIZE	(1024*1024*16)
#define DEFAULT_HIST_BINS	32

extern char *date_format[];
extern int   date_formats;

typedef struct csvT {
	char	*filename;
	FILE	*fp;
	off_t	size;
	unsigned int est_row_size;
	unsigned long est_rows;
	unsigned int bytes_per_bitmap_bit;
	unsigned char	*bitmap;
	unsigned long	bitmap_size;
} csvT;

typedef enum 
{
	Numeric = 0,
	Factor,
	Date
} dtype;

typedef struct factorT
{
	char			**item;
	unsigned long	items;
	unsigned long	allocated;
} factorT;

typedef struct typeT
{
	dtype		type;
	void		(*from_string)(struct typeT *, char *, double *);
	void		(*to_string)(struct typeT *, double *, char *);
	void		*data;		
} typeT;

extern typeT	type[MAX_TYPES];
extern int		types;

typedef struct histogramT
{
	int		bins;
	double	*breaks;
	unsigned long	*counts;
	unsigned long	max_count;
} histogramT;

typedef struct statsT
{
	double		max, min;
	double		mean;
	double		stddev;
	double		sum;
	double		sum_squares;
	double		quartile[3];
	histogramT	histogram;
	unsigned long	cardinality;
} statsT;

typedef struct transformT
{
	char			*name;
	void			(*apply)(struct transformT *, double *, statsT *);
	char			is_monotonic;
	void			*param;
	size_t			param_sz;
	double			*data_out;
	unsigned long	rows;			
	statsT			stats_out;
	struct transformT	*next;
	struct transformT	*prev;
	struct columnT		*column;
} transformT;

extern transformT	transform[MAX_TRANSFORMS];
extern int			transforms;

typedef struct columnT 
{
	char		*name;
	typeT		type;
	transformT	*transform;
	double		*orig_data;
	statsT		orig_stats;
	struct frameT	*frame;
} columnT;

typedef struct frameT
{
	csvT			*csv;
	unsigned long	rows;
	unsigned long	allocated_rows;
	columnT			**column;
	int				columns;
} frameT;

int is_date (char *buf);
frameT	*read_csv (char *filename);

frameT	*new_frame (char *name, int columns);
void	free_frame (frameT *f);
void    print_frame (frameT *frame);
void	init_column (frameT *f, int i, char *name, dtype t);
void	column_init_data (frameT *f, int i, unsigned long rows);
void    column_realloc_data (frameT *f, int i, unsigned long rows);
void	column_add_data (frameT *f, int i, char *str);
void 	column_add_transform (columnT *c, transformT *t);
void	column_apply_transforms (columnT *c);
void    update_stats (statsT *s, double *data, unsigned long rows);
void    show_stats (statsT *s);

void	init_transforms (void);
transformT 	*clone_transform (transformT *t);

void	init_types (void);

#endif /* __DATA_H__ */
