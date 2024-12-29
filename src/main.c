/*******************************************************************************
** GRAPHICSPACK (GUI Graphics Packer) - No Shinobi Knows Me 2024
*******************************************************************************/

/*******************************************************************************
** main.c
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI      3.14159265358979323846f
#define TWO_PI  6.28318530717958647693f

#if 0
/* integer types */
typedef signed char   sint8;
typedef signed short  sint16;
typedef signed long   sint32;

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned long   uint32;
#endif

#define GRAPHICS_PACK_INTO_15_BIT_RGB(r, g, b, value)                          \
  value = ((r << 7) & 0x7C00) | ((g << 2) & 0x03E0) | ((b >> 3) & 0x001F);

#define GRAPHICS_UNPACK_FROM_15_BIT_RGB(value, r, g, b)                        \
  r = ((value >> 7) & 0xF8) | ((value >> 12) & 0x07);                          \
  g = ((value >> 2) & 0xF8) | ((value >> 7) & 0x07);                           \
  b = ((value << 3) & 0xF8) | ((value >> 2) & 0x07);

#define GRAPHICS_NUM_SHADES  4
#define GRAPHICS_NUM_HUES   15

#define GRAPHICS_NUM_PALETTES       (GRAPHICS_NUM_HUES + 1)
#define GRAPHICS_COLORS_PER_PALETTE 16

#define GRAPHICS_PALETTES_BUFFER_SIZE (GRAPHICS_NUM_PALETTES * GRAPHICS_COLORS_PER_PALETTE)

#define GRAPHICS_NUM_CELLS       160 /* 16 * 10 */
#define GRAPHICS_PIXELS_PER_CELL  64 /*  8 *  8 */

#define GRAPHICS_CELLS_BUFFER_SIZE (GRAPHICS_NUM_CELLS * GRAPHICS_PIXELS_PER_CELL)

/* tables */
static float  S_lum_table[GRAPHICS_NUM_SHADES] = 
              { 1 / 6.0f, 2 / 6.0f, 4 / 6.0f, 5 / 6.0f };

static float  S_sat_table[GRAPHICS_NUM_SHADES] = 
              { 1 / 6.0f, 2 / 6.0f, 2 / 6.0f, 1 / 6.0f };

/* buffers */
static unsigned short S_graphics_palettes_buffer[GRAPHICS_PALETTES_BUFFER_SIZE];
static unsigned char  S_graphics_cells_buffer[GRAPHICS_CELLS_BUFFER_SIZE];

/*******************************************************************************
** graphics_reset_buffers()
*******************************************************************************/
short int graphics_reset_buffers()
{
  int m;

  /* reset palettes */
  for (m = 0; m < GRAPHICS_PALETTES_BUFFER_SIZE; m++)
    S_graphics_palettes_buffer[m] = 0;

  /* reset cells */
  for (m = 0; m < GRAPHICS_CELLS_BUFFER_SIZE; m++)
    S_graphics_cells_buffer[m] = 0;

  return 0;
}

/*******************************************************************************
** graphics_generate_palettes()
*******************************************************************************/
short int graphics_generate_palettes()
{
  int m;
  int n;

  unsigned short value;

  float y;
  float i;
  float q;

  int r;
  int g;
  int b;

  int index;

  /* reset palettes */
  for (m = 0; m < GRAPHICS_PALETTES_BUFFER_SIZE; m++)
    S_graphics_palettes_buffer[m] = 0;

  /* black & white in all palettes */
  for (m = 0; m < GRAPHICS_NUM_PALETTES; m++)
  {
    index = m * GRAPHICS_COLORS_PER_PALETTE + 0;

    r = 0;
    g = 0;
    b = 0;

    GRAPHICS_PACK_INTO_15_BIT_RGB(r, g, b, value)

    S_graphics_palettes_buffer[index + 1] = value;

    r = 255;
    g = 255;
    b = 255;

    GRAPHICS_PACK_INTO_15_BIT_RGB(r, g, b, value)

    S_graphics_palettes_buffer[index + 2] = value;
  }

  /* generate greys in palette 0 */
  for (n = 0; n < GRAPHICS_NUM_SHADES; n++)
  {
    index = 0 * GRAPHICS_COLORS_PER_PALETTE + 4;

    /* compute rgb values */
    r = (int) ((S_lum_table[n] * 255) + 0.5f);
    g = (int) ((S_lum_table[n] * 255) + 0.5f);
    b = (int) ((S_lum_table[n] * 255) + 0.5f);

    /* convert color to 15 bit rgb */
    GRAPHICS_PACK_INTO_15_BIT_RGB(r, g, b, value)

    /* insert this color into the palette */
    S_graphics_palettes_buffer[index + n] = value;
  }

  /* copy greys to other palettes */
  for (m = 1; m < GRAPHICS_NUM_PALETTES; m++)
  {
    index = m * GRAPHICS_COLORS_PER_PALETTE + 4;

    for (n = 0; n < GRAPHICS_NUM_SHADES; n++)
      S_graphics_palettes_buffer[index + n] = S_graphics_palettes_buffer[0 + 4 + n];
  }

  /* generate hues */
  for (m = 0; m < GRAPHICS_NUM_HUES; m++)
  {
    for (n = 0; n < GRAPHICS_NUM_SHADES; n++)
    {
      index = m * GRAPHICS_COLORS_PER_PALETTE + 8;

      /* compute color in yiq */
      y = S_lum_table[n];
      i = S_sat_table[n] * cos((TWO_PI * m) / GRAPHICS_NUM_HUES);
      q = S_sat_table[n] * sin((TWO_PI * m) / GRAPHICS_NUM_HUES);

      /* convert from yiq to rgb */
      r = (int) (((y + (i * 0.956f) + (q * 0.619f)) * 255) + 0.5f);
      g = (int) (((y - (i * 0.272f) - (q * 0.647f)) * 255) + 0.5f);
      b = (int) (((y - (i * 1.106f) + (q * 1.703f)) * 255) + 0.5f);

      /* hard clipping at the bottom */
      if (r < 0)
        r = 0;
      if (g < 0)
        g = 0;
      if (b < 0)
        b = 0;

      /* hard clipping at the top */
      if (r > 255)
        r = 255;
      if (g > 255)
        g = 255;
      if (b > 255)
        b = 255;

      /* convert color to 15 bit rgb */
      GRAPHICS_PACK_INTO_15_BIT_RGB(r, g, b, value)

      /* insert this color into the palette */
      S_graphics_palettes_buffer[index + n] = value;
    }
  }

  /* copy greys for hue in last palette */
  index = (GRAPHICS_NUM_PALETTES - 1) * GRAPHICS_COLORS_PER_PALETTE + 8;

  for (n = 0; n < GRAPHICS_NUM_SHADES; n++)
    S_graphics_palettes_buffer[index + n] = S_graphics_palettes_buffer[0 + 4 + n];

  return 0;
}

/*******************************************************************************
** graphics_read_texture_tga_file()
*******************************************************************************/
short int graphics_read_texture_tga_file(char* filename)
{
  int m;
  int n;

  int i;
  int j;
  int k;

  FILE* fp;

  unsigned char image_id_field_length;
  unsigned char color_map_type;
  unsigned char image_type;
  unsigned char image_descriptor;
  short         x_origin;
  short         y_origin;
  short         image_width;
  short         image_height;
  unsigned char pixel_bpp;

  unsigned char*  pixel_data;
  short           pixel_num_bytes;

  unsigned short value;

  unsigned short r;
  unsigned short g;
  unsigned short b;

  int num_rows;
  int num_cols;

  int pixel_corner;
  int pixel_index;

  int cell_corner;
  int cell_index;

  int color_index;

  /* make sure filename is valid */
  if (filename == NULL)
  {
    printf("No filename specified.\n");
    return 1;
  }

  /* open tga file */
  fp = fopen(filename, "rb");

  if (fp == NULL)
  {
    printf("Failed to open TGA file: %s\n", filename);
    return 1;
  }

  /* read image id field length */
  if (fread(&image_id_field_length, 1, 1, fp) < 1)
  {
    fclose(fp);
    return 1;
  }

  /* make sure there is no colormap */
  if (fread(&color_map_type, 1, 1, fp) < 1)
  {
    fclose(fp);
    return 1;
  }

  if (color_map_type != 0)
  {
    fclose(fp);
    return 1;
  }

  /* make sure this is a type 2 tga file (uncompressed truecolor image) */
  if (fread(&image_type, 1, 1, fp) < 1)
  {
    fclose(fp);
    return 1;
  }

  if (image_type != 2)
  {
    fclose(fp);
    return 1;
  }

  /* skip 5 byte colormap specification */
  if (fseek(fp, 5, SEEK_CUR))
  {
    fclose(fp);
    return 1;
  }

  /* read x and y origin, image width, height, bpp */
  if (fread(&x_origin, 2, 1, fp) < 1)
  {
    fclose(fp);
    return 1;
  }

  if (fread(&y_origin, 2, 1, fp) < 1)
  {
    fclose(fp);
    return 1;
  }

  if (fread(&image_width, 2, 1, fp) < 1)
  {
    fclose(fp);
    return 1;
  }

  if (fread(&image_height, 2, 1, fp) < 1)
  {
    fclose(fp);
    return 1;
  }

  if (fread(&pixel_bpp, 1, 1, fp) < 1)
  {
    fclose(fp);
    return 1;
  }

  /* set number of bytes and type based on bpp */
  if (pixel_bpp == 24)
    pixel_num_bytes = 3;
  else if (pixel_bpp == 32)
    pixel_num_bytes = 4;
  else
  {
    printf("Error: Invalid pixel bpp.\n");
    fclose(fp);
    return 1;
  }

  /* read image descriptor */
  if (fread(&image_descriptor, 1, 1, fp) < 1)
  {
    fclose(fp);
    return 1;
  }

  /* skip image id field */
  if (image_id_field_length != 0)
  {
    if (fseek(fp, image_id_field_length, SEEK_CUR))
    {
      fclose(fp);
      return 1;
    }
  }

  /* make sure width and height are correct */
  if (((image_width % 8) != 0) || ((image_height % 8) != 0))
  {
    printf("Error: Invalid image dimensions.\n");
    fclose(fp);
    return 1;
  }

  /* generate pixel data buffer */
  pixel_data = malloc(sizeof(unsigned char) * pixel_num_bytes * image_width * image_height);

  if (pixel_data == NULL)
  {
    fclose(fp);
    return 1;
  }

  /* if origin is at top left, fill rows forwards */
  if (image_descriptor & 0x20)
  {
    for (m = 0; m < image_height; m++)
    {
      if (fread(&(pixel_data[m * image_width * pixel_num_bytes]),
                1, image_width * pixel_num_bytes, fp) < 
                (unsigned int) (image_width * pixel_num_bytes))
      {
        if (pixel_data != NULL)
          free(pixel_data);

        fclose(fp);
        return 1;
      }
    }
  }
  /* if origin is at bottom left, fill rows backwards */
  else
  {
    for (m = image_height - 1; m >= 0; m--)
    {
      if (fread(&(pixel_data[m * image_width * pixel_num_bytes]), 
                1, image_width * pixel_num_bytes, fp) < 
                (unsigned int) (image_width * pixel_num_bytes))
      {
        if (pixel_data != NULL)
          free(pixel_data);

        fclose(fp);
        return 1;
      }
    }
  }

  /* close tga file */
  fclose(fp);

  /* determine rows & columns (of cells) in the image */
  num_rows = image_height / 8;
  num_cols = image_width / 8;

  if ((num_rows < 1) || (num_cols < 1))
  {
    printf("Error: Need at least 1 row and 1 column in the image.\n");
    return 1;
  }

  if ((num_rows * num_cols) > GRAPHICS_NUM_CELLS)
  {
    printf("Error: Too many rows / columns in the image.\n");
    return 1;
  }

  /* convert pixel data to palette indices and store it in the cells buffer */
  for (m = 0; m < num_rows; m++)
  {
    for (n = 0; n < num_cols; n++)
    {
      pixel_corner = (64 * m * num_cols) + (8 * n);
      cell_corner = 64 * ((m * num_cols) + n);

      for (i = 0; i < 8; i++)
      {
        for (j = 0; j < 8; j++)
        {
          pixel_index = pixel_corner + (8 * i * num_cols) + j;
          cell_index = cell_corner + (8 * i) + j;

          /* read the rgb data for this pixel   */
          /* the tga format stores this as bgr  */
          r = pixel_data[pixel_index * pixel_num_bytes + 2];
          g = pixel_data[pixel_index * pixel_num_bytes + 1];
          b = pixel_data[pixel_index * pixel_num_bytes + 0];

          GRAPHICS_PACK_INTO_15_BIT_RGB(r, g, b, value)

          /* determine the index in palette 0 for this rgb color */
          color_index = -1;

          for (k = 0; k < GRAPHICS_COLORS_PER_PALETTE; k++)
          {
            /* transparency (cyan) */
            if (k == 0)
            {
              if ((r == 0) && (g == 255) && (b == 255))
              {
                color_index = k;
                break;
              }
            }
            /* other colors */
            else
            {
              if (value == S_graphics_palettes_buffer[0 * GRAPHICS_COLORS_PER_PALETTE + k])
              {
                color_index = k;
                break;
              }
            }
          }

          if (color_index == -1)
          {
            printf( "Unknown color encountered: (%d, %d, %d)\n", 
                    r, g, b);

            continue;
          }

          /* write this color index to the cells buffer */
          S_graphics_cells_buffer[cell_index] = (unsigned char) color_index;
        }
      }
    }
  }

  /* free pixel data */
  if (pixel_data != NULL)
    free(pixel_data);

  return 0;
}

/*******************************************************************************
** graphics_write_texture_dat_file()
*******************************************************************************/
short int graphics_write_texture_dat_file(char* filename)
{
  int m;

  FILE* fp;

  char signature[17];

  unsigned short value;

  unsigned char output_byte;

  /* reset signature */
  for (m = 0; m < 17; m++)
    signature[m] = '\0';

  /* make sure filename is valid */
  if (filename == NULL)
  {
    printf("No filename specified.\n");
    return 1;
  }

  /* open dat file */
  fp = fopen(filename, "wb");

  /* if file did not open, return */
  if (fp == NULL)
  {
    printf("Unable to open output file. Exiting...\n");
    return 1;
  }

  /* write signature */
  strcpy(signature, "NSKM");
  strcat(signature, "GRAPHICS");
  strcat(signature, "v1.0");

  fwrite(signature, 1, 16, fp);

  /* write palette */
  for (m = 0; m < GRAPHICS_PALETTES_BUFFER_SIZE; m++)
  {
    value = S_graphics_palettes_buffer[m];

    /* high byte */
    output_byte = (value >> 8) & 0x7F;

    fwrite(&output_byte, 1, 1, fp);

    /* low byte */
    output_byte = value & 0xFF;

    fwrite(&output_byte, 1, 1, fp);
  }

  /* write cells */
  for (m = 0; m < (GRAPHICS_CELLS_BUFFER_SIZE / 2); m++)
  {
    output_byte =   (S_graphics_cells_buffer[2 * m + 0] & 0x0F) << 4;
    output_byte |=  (S_graphics_cells_buffer[2 * m + 1] & 0x0F);

    fwrite(&output_byte, 1, 1, fp);
  }

  /* close output file */
  fclose(fp);

  return 0;
}

/*******************************************************************************
** graphics_write_palette_gpl_file()
*******************************************************************************/
short int graphics_write_palette_gpl_file(char* filename)
{
  int m;

  FILE* fp;

  unsigned short value;

  unsigned short r;
  unsigned short g;
  unsigned short b;

  /* make sure filename is valid */
  if (filename == NULL)
  {
    printf("No filename specified.\n");
    return 1;
  }

  /* open output file */
  fp = fopen(filename, "w");

  /* if file did not open, return */
  if (fp == NULL)
  {
    printf("Unable to open output text file. Exiting...\n");
    return 1;
  }

  /* write out header info */
  fprintf(fp, "GIMP Palette\n");
  fprintf(fp, "Name: NSKM GUI Colors\n");
  fprintf(fp, "Columns: 16\n\n");

  /* write out palette colors */
  for (m = 0; m < GRAPHICS_PALETTES_BUFFER_SIZE; m++)
  {
    value = S_graphics_palettes_buffer[m];

    GRAPHICS_UNPACK_FROM_15_BIT_RGB(value, r, g, b)

    if (r < 10)
      fprintf(fp, "  %d ", r);
    else if (r < 100)
      fprintf(fp, " %d ", r);
    else
      fprintf(fp, "%d ", r);

    if (g < 10)
      fprintf(fp, "  %d ", g);
    else if (g < 100)
      fprintf(fp, " %d ", g);
    else
      fprintf(fp, "%d ", g);

    if (b < 10)
      fprintf(fp, "  %d", b);
    else if (b < 100)
      fprintf(fp, " %d", b);
    else
      fprintf(fp, "%d", b);

    fprintf(fp, "\t(%d, %d, %d)\n", r, g, b);
  }

  /* close file */
  fclose(fp);

  return 0;
}

/*******************************************************************************
** main()
*******************************************************************************/
int main(int argc, char *argv[])
{
  /* reset buffers */
  graphics_reset_buffers();

  /* generate palettes */
  graphics_generate_palettes();

  /* write palette file */
  graphics_write_palette_gpl_file("nskm_gui_graphics.gpl");

  /* read tga file */
  graphics_read_texture_tga_file("graphics.tga");

  /* write output file */
  graphics_write_texture_dat_file("graphics.dat");

  return 0;
}

