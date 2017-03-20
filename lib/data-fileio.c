
/* include the dataset header. */
#include <vfl/data.h>

/* data_fread(): read a text file into an allocated dataset structure.
 *
 * arguments:
 *  @dat: dataset structure pointer to augment.
 *  @fname: filename to read from.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_fread (data_t *dat, const char *fname) {
  /* declare required variables:
   *  @N, @D: sizes of the input data.
   *  @buf: buffer of input lines.
   *  @fh: input file handle.
   */
  unsigned int N, D;
  char buf[1024];
  FILE *fh;

  /* check the input pointers. */
  if (!dat || !fname)
    return 0;

  /* open the input file. */
  fh = fopen(fname, "r");
  if (!fh)
    return 0;

  /* attempt to read and parse the first line of input. */
  if (!fgets(buf, 1024, fh) || sscanf(buf, "# %u %u", &N, &D) != 2)
    goto fail;

  /* check that the dataset has conforming dimensionality. */
  if (dat->D && D != dat->D)
    goto fail;

  /* determine the total observation count. */
  unsigned int i = dat->N;
  N += dat->N;

  /* attempt to resize the dataset to accomodate the augmenting data. */
  if (!data_resize(dat, N, D))
    goto fail;

  /* read the remainder of the file. */
  while (!feof(fh)) {
    /* read a new line of the file. */
    if (fgets(buf, 1024, fh)) {
      /* skip commented lines. */
      if (buf[0] == '#')
        continue;

      /* read the output index. */
      char *tok = strtok(buf, " ");
      dat->data[i].p = atoi(tok);

      /* read each observation input value. */
      for (unsigned int d = 0; d < D; d++) {
        tok = strtok(NULL, " ");
        vector_set(dat->data[i].x, d, atof(tok));
      }

      /* read the observed value. */
      tok = strtok(NULL, " ");
      dat->data[i].y = atof(tok);

      /* increment the observation index. */
      i++;
    }
  }

  /* close the input file and return success. */
  fclose(fh);
  return 1;

fail:
  /* close the input file and return failure. */
  fclose(fh);
  return 0;
}

/* data_fwrite(): write the contents of a dataset to a text file.
 *
 * arguments:
 *  @dat: dataset structure pointer to access.
 *  @fname: filename to write to.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int data_fwrite (const data_t *dat, const char *fname) {
  /* check the input pointers. */
  if (!dat || !fname)
    return 0;

  /* open the output file. */
  FILE *fh = fopen(fname, "w");
  if (!fh)
    return 0;

  /* write a short header. */
  fprintf(fh, "# %u %u\n", dat->N, dat->D);

  /* loop over each observation. */
  for (unsigned int i = 0; i < dat->N; i++) {
    /* write the observation output index. */
    fprintf(fh, "%u", dat->data[i].p);

    /* write the observation location. */
    for (unsigned int d = 0; d < dat->D; d++)
      fprintf(fh, " %le", vector_get(dat->data[i].x, d));

    /* write the observed value. */
    fprintf(fh, " %le\n", dat->data[i].y);
  }

  /* close the output file and return success. */
  fclose(fh);
  return 1;
}

