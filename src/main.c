/*******************************************************************************
** GRAPHICSPACK (GUI Graphics Packer) - No Shinobi Knows Me 2025
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

enum
{
  GRAPHICS_PALETTE_TEXT_GREY = 0, 
  GRAPHICS_PALETTE_TEXT_DARK, 
  GRAPHICS_PALETTE_TEXT_HUE_01, 
  GRAPHICS_PALETTE_TEXT_HUE_02, 
  GRAPHICS_PALETTE_TEXT_HUE_03, 
  GRAPHICS_PALETTE_TEXT_HUE_04, 
  GRAPHICS_PALETTE_TEXT_HUE_05, 
  GRAPHICS_PALETTE_TEXT_HUE_06, 
  GRAPHICS_PALETTE_TEXT_HUE_07, 
  GRAPHICS_PALETTE_BG_HUE_01, 
  GRAPHICS_PALETTE_BG_HUE_02, 
  GRAPHICS_PALETTE_BG_HUE_03, 
  GRAPHICS_PALETTE_BG_HUE_04, 
  GRAPHICS_PALETTE_BG_HUE_05, 
  GRAPHICS_PALETTE_BG_HUE_06, 
  GRAPHICS_PALETTE_BG_HUE_07, 
  GRAPHICS_NUM_PALETTES 
};

/* 9 bit rgb */
#define GRAPHICS_PACK_INTO_9_BIT_RGB(r, g, b, value)                          \
  value = ((r << 1) & 0x01C0) | ((g >> 2) & 0x0038) | ((b >> 5) & 0x0007);

/* each 3-bit component expands to a multiple of 34 (0, 34, 68, ..., 238) */
/* the 8-bit value is in the form xxx0 xxx0, where xxx is the 3-bit value */
#define GRAPHICS_UNPACK_FROM_9_BIT_RGB(value, r, g, b)                        \
  r = ((value >> 1) & 0xE0) | ((value >> 5) & 0x0E);                          \
  g = ((value << 2) & 0xE0) | ((value >> 2) & 0x0E);                          \
  b = ((value << 5) & 0xE0) | ((value << 1) & 0x0E);

/* 12 bit rgb */
#define GRAPHICS_PACK_INTO_12_BIT_RGB(r, g, b, value)                         \
  value = ((r << 4) & 0x0F00) | (g & 0x00F0) | ((b >> 4) & 0x000F);

#define GRAPHICS_UNPACK_FROM_12_BIT_RGB(value, r, g, b)                       \
  r = ((value >> 4) & 0xF0) | ((value >> 8) & 0x0F);                          \
  g = (value & 0xF0)        | ((value >> 4) & 0x0F);                          \
  b = ((value << 4) & 0xF0) | (value & 0x0F);

#define GRAPHICS_NUM_SHADES 4
#define GRAPHICS_NUM_HUES   7

#define GRAPHICS_PALETTES_BUFFER_SIZE (GRAPHICS_NUM_PALETTES * GRAPHICS_NUM_SHADES)

#define GRAPHICS_NUM_CELLS       128 /* 16 x 8 */
#define GRAPHICS_PIXELS_PER_CELL  64 /*  8 x 8 */

#define GRAPHICS_CELLS_BUFFER_SIZE (GRAPHICS_NUM_CELLS * GRAPHICS_PIXELS_PER_CELL)

/* tables */
static float  S_text_lum_table[GRAPHICS_NUM_SHADES] = 
              { 0.0f, 0.7f, 0.85f, 1.0f };

static float  S_text_sat_table[GRAPHICS_NUM_SHADES] = 
              { 0.0f, 0.3f, 0.15f, 0.0f };

static float  S_dark_lum_table[GRAPHICS_NUM_SHADES] = 
              { 0.0f, 0.0f, 0.15f, 0.3f };

static float  S_dark_sat_table[GRAPHICS_NUM_SHADES] = 
              { 0.0f, 0.0f, 0.15f, 0.3f };

static float  S_bg_lum_table[GRAPHICS_NUM_SHADES] = 
              { 0.1f, 0.2f, 0.3f, 0.7f };

static float  S_bg_sat_table[GRAPHICS_NUM_SHADES] = 
              { 0.1f, 0.2f, 0.3f, 0.3f };

static float  S_hue_table[GRAPHICS_NUM_HUES] = 
              {  30.0f,  90.0f, 120.0f, 180.0f, 210.0f, 300.0f, 330.0f };

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

  float phi;

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

  /* generate palettes */
  for (m = 0; m < GRAPHICS_NUM_PALETTES; m++)
  {
    for (n = 0; n < GRAPHICS_NUM_SHADES; n++)
    {
      index = m * GRAPHICS_NUM_SHADES + n;

      switch(m)
      {
        case GRAPHICS_PALETTE_TEXT_GREY:
        {
          y = S_text_lum_table[n];
          i = 0.0f;
          q = 0.0f;
          break;
        }
        case GRAPHICS_PALETTE_TEXT_DARK:
        {
          y = S_dark_lum_table[n];
          i = 0.0f;
          q = 0.0f;
          break;
        }
        case GRAPHICS_PALETTE_TEXT_HUE_01:
        case GRAPHICS_PALETTE_TEXT_HUE_02:
        case GRAPHICS_PALETTE_TEXT_HUE_03:
        case GRAPHICS_PALETTE_TEXT_HUE_04:
        case GRAPHICS_PALETTE_TEXT_HUE_05:
        case GRAPHICS_PALETTE_TEXT_HUE_06:
        case GRAPHICS_PALETTE_TEXT_HUE_07:
        {
          y = S_text_lum_table[n];
          i = S_text_sat_table[n] * cos(((TWO_PI * S_hue_table[m - GRAPHICS_PALETTE_TEXT_HUE_01]) / 360.0f) + PI);
          q = S_text_sat_table[n] * sin(((TWO_PI * S_hue_table[m - GRAPHICS_PALETTE_TEXT_HUE_01]) / 360.0f) + PI);
          break;
        }
        case GRAPHICS_PALETTE_BG_HUE_01:
        case GRAPHICS_PALETTE_BG_HUE_02:
        case GRAPHICS_PALETTE_BG_HUE_03:
        case GRAPHICS_PALETTE_BG_HUE_04:
        case GRAPHICS_PALETTE_BG_HUE_05:
        case GRAPHICS_PALETTE_BG_HUE_06:
        case GRAPHICS_PALETTE_BG_HUE_07:
        {
          if (n == GRAPHICS_NUM_SHADES - 1)
            phi = PI;
          else
            phi = 0.0f;

          y = S_bg_lum_table[n];
          i = S_bg_sat_table[n] * cos(((TWO_PI * S_hue_table[m - GRAPHICS_PALETTE_BG_HUE_01]) / 360.0f) + phi);
          q = S_bg_sat_table[n] * sin(((TWO_PI * S_hue_table[m - GRAPHICS_PALETTE_BG_HUE_01]) / 360.0f) + phi);
          break;
        }
        default:
        {
          y = 0.0f;
          i = 0.0f;
          q = 0.0f;
          break;
        }
      }

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

      /* convert color to 12 bit rgb */
      GRAPHICS_PACK_INTO_12_BIT_RGB(r, g, b, value)

      /* insert this color into the palette */
      S_graphics_palettes_buffer[index] = value;
    }
  }

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

          GRAPHICS_PACK_INTO_12_BIT_RGB(r, g, b, value)

          /* determine the index in palette 0 for this rgb color */
          color_index = -1;

          for (k = 0; k < GRAPHICS_NUM_SHADES; k++)
          {
            if (value == S_graphics_palettes_buffer[0 * GRAPHICS_NUM_SHADES + k])
            {
              color_index = k;
              break;
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

  unsigned short value[2];
  unsigned char  byte[3];

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
  for (m = 0; m < (GRAPHICS_PALETTES_BUFFER_SIZE / 2); m++)
  {
    value[0] = S_graphics_palettes_buffer[2 * m + 0];
    value[1] = S_graphics_palettes_buffer[2 * m + 1];

    /* packing 2 palette colors into 3 bytes */
    /* if the colors are "a" and "b", then the    */
    /* bytes are: aaaa aaaa, aaaa bbbb, bbbb bbbb */

    byte[0] = (value[0] >> 4) & 0xFF;
    byte[1] = ((value[0] << 4) & 0xF0) | ((value[1] >> 8) & 0x0F);
    byte[2] = value[1] & 0xFF;

    fwrite(&byte[0], 1, 1, fp);
    fwrite(&byte[1], 1, 1, fp);
    fwrite(&byte[2], 1, 1, fp);
  }

  /* write cells */
  for (m = 0; m < (GRAPHICS_CELLS_BUFFER_SIZE / 4); m++)
  {
    byte[0] =   (S_graphics_cells_buffer[4 * m + 0] & 0x03) << 6;
    byte[0] |=  (S_graphics_cells_buffer[4 * m + 1] & 0x03) << 4;
    byte[0] |=  (S_graphics_cells_buffer[4 * m + 2] & 0x03) << 2;
    byte[0] |=  (S_graphics_cells_buffer[4 * m + 3] & 0x03);

    fwrite(&byte[0], 1, 1, fp);
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

    GRAPHICS_UNPACK_FROM_12_BIT_RGB(value, r, g, b)

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

