/*

  gpx.c


https://en.wikipedia.org/wiki/GPS_Exchange_Format

http://www.topografix.com/GPX/1/1/




48.635910, 8.8932 75
48.6355 84, 8.8926 85
48.635573, 8.8925 94
48.635662, 8.892685

TP.time
In: TP, Threshold [s]

if gpx file does not exist then
   create gpx file
   write header to gpx file
   write trackstart to gpx file
   write trackend to gpx file
   write footer to gpx file
   lasttime = tp.time
   last time eeprom init
else
  lasttime = get last time from eeprom
end if

if tp.time - lasttime > Threshold then
  seek to end of track in gpx file
  write trackend to gpx file
  write trackstart to gpx file
  write trackend to gpx file
  write footer to gpx file
end if

seek to end of track in gpx file
write trackpoint to gpx file
write trackend to gpx file
write footer to gpx file
store last time to eeprom

====

eeprom:
init, get last time, store new time
*/

#include <string.h>
#include <chip.h>
#include "delay.h"
#include "u8g2.h"
#include "ff.h"		/* Declarations of FatFs API */
#include "gpx.h"
#include "eeprom.h"

FATFS FatFs;		/* FatFs work area needed for each volume */
FIL gpx_file;			/* File object needed for each open file */
const char gpx_name[] = "TRACK.GPX";
const char gpx_header[] = "<gpx version=\"1.1\" "\
"creator=\"https://github.com/olikraus/lpc11u3x-gps-logger\">\n"\
" <trk><name>LOG</name>\n";
const char gpx_footer[] = " </trk>\n</gpx>\n";
const char gpx_track_start[] = "  <trkseg>\n";
const char gpx_track_end[] = "  </trkseg>\n";
const char gpx_trkpt_start[] = "   <trkpt";
const char gpx_lat_pre[] = " lat=";
const char gpx_lon_pre[] = " lon=";
const char gpx_quote[] = "\"";
const char gpx_tag_close[] = ">\n";
const char gpx_trkpt_end[] = "   </trkpt>\n";


void gpx_log(const char *msg, FRESULT fr)
{
}

int gpx_mount(void)
{
  FRESULT fr;
  fr = f_mount(&FatFs, "", 1);		/* Give a work area to the default drive and force a mount (http://elm-chan.org/fsw/ff/en/mount.html) */
  if (   fr == FR_OK )
  {
    return 1;
  }
  gpx_log("mount failed", fr);
  return 0;    
}

void gpx_unmount(void)
{
  f_mount(NULL, "", 0);
}

/* 1 for ok, 0, for does not exist, -1 for error */
int gpx_is_available(void)
{
  FRESULT fr;
  fr = f_stat(gpx_name, NULL); 
  if ( fr == FR_OK) 
  {	
    return 1;
  }
  if ( fr ==  FR_NO_FILE) 
  {	
    return 0;
  }
  gpx_log("stat failed", fr);
  return 0;
}

int gpx_open_file(void)
{
  FRESULT fr;
  fr = f_open(&gpx_file, gpx_name, FA_WRITE | FA_OPEN_EXISTING); 
  if ( fr == FR_OK) 
  {	
    return 1;
  }
  gpx_log("open failed", fr);
  return 0;
}

int gpx_create_file(void)
{
  FRESULT fr;
  fr = f_open(&gpx_file, gpx_name, FA_WRITE | FA_CREATE_ALWAYS); 
  if ( fr == FR_OK) 
  {	
    return 1;
  }
  gpx_log("create failed", fr);
  return 0;
}

int gpx_seek_for_append(void)
{
  
  FRESULT fr;
  /* sizeof could be used instead of strlen, but then \0 has to be considered */
  fr = f_lseek(&gpx_file, f_size(&gpx_file) - strlen(gpx_track_end) - strlen(gpx_footer)); 
  if ( fr == FR_OK) 
  {	
    return 1;
  }
  gpx_log("seek failed", fr);
  return 0;
}

int gpx_close_file(void)
{
  FRESULT fr;
  fr = f_close(&gpx_file);
  if ( fr == FR_OK )
  {
    return 1;
  }
  gpx_log("close failed", fr);
  return 0;
}


int gpx_write_str(const char *s, const char *log)
{
  UINT bw;
  FRESULT fr;
  fr = f_write(&gpx_file, s, strlen(s), &bw);
  if ( fr == FR_OK )
  {
    return 1;
  }
  else
  {
    char msg[16];
    strcpy(msg, "wr err: ");
    strcpy(msg+8, log);
    gpx_log(msg, fr);
  }
  return 0;  
}

int gpx_write_header(void)
{
  return gpx_write_str(gpx_header, "header");
}

int gpx_write_footer(void)
{
  return gpx_write_str(gpx_footer, "footer");
}

int gpx_write_track_start(void)
{
  return gpx_write_str(gpx_track_start, "tr start");
}

int gpx_write_track_end(void)
{
  return gpx_write_str(gpx_track_end, "tr end");
}

int gpx_write_track_point_start(void)
{
  return gpx_write_str(gpx_trkpt_start, "pt start");
}

int gpx_write_track_point_end(void)
{
  return gpx_write_str(gpx_trkpt_end, "pt end");
}

int gpx_write_gps_float(gps_float_t f)
{
  char s[16];
  gps_float_to_str(f, s);
  return gpx_write_str(s, "float");  
}

int gpx_write_gps_float_with_quote(gps_float_t f)
{
  if ( gpx_write_str(gpx_quote, gpx_quote) != 0 )
  {
    if ( gpx_write_gps_float(f) != 0 )
    {
      if ( gpx_write_str(gpx_quote, gpx_quote) != 0 )
      {
	return 1;
      }
    }
  }
  return 0;
}

int gpx_write_gps_pos(gps_pos_t *pos)
{
  if ( gpx_write_str(gpx_lat_pre, "lat") != 0 )
  {
    if ( gpx_write_gps_float_with_quote(pos->latitude) != 0 )
    {
      if ( gpx_write_str(gpx_lon_pre, "lon") != 0 )
      {
	if ( gpx_write_gps_float_with_quote(pos->longitude) != 0 )
	{
	  return 1;
	}
      }
    }
  }
  return 0;
}

int gpx_write_track_point(gps_pos_t *pos)
{
  if ( gpx_write_track_point_start() != 0 )
  {    
    if ( gpx_write_gps_pos(pos) != 0 )
    {
      if ( gpx_write_str(gpx_tag_close, ">") != 0 )
      {
	/* ... */
	if ( gpx_write_track_point_end() != 0 )
	{
	  return 1;
	}
      }
    }
  }
  return 0;
}

int gpx_create_empty_file(void)
{
  if ( gpx_create_file() != 0 )
  {
      if ( gpx_write_header() != 0 )
      {
	if ( gpx_write_track_start() != 0 )
	{
	  if ( gpx_write_track_end() != 0 )
	  {
	    if ( gpx_write_footer() != 0 )
	    {
	      gpx_close_file();
	      return 1;
	    }
	  }
	}
      }
  }
  gpx_close_file();
  return 0;
}

int gpx_append(gps_pos_t *pos)
{
  if ( gpx_open_file() != 0 )
  {
    if ( gpx_seek_for_append() != 0 )
    {
      if ( gpx_write_track_point(pos) != 0 )
      {
	if ( gpx_write_track_end() != 0 )
	{
	  if ( gpx_write_footer() != 0 )
	  {
	    gpx_close_file();
	    return 1;
	  }
	}
      }
    }
  }
  gpx_close_file();
  return 0;
}

int gpx_write(gps_pos_t *pos)
{
  if ( gpx_mount() == 0 )
    return 0;
  
  if ( gpx_is_available() == 0 )
  {
    if ( gpx_create_empty_file() == 0 )
    {
      return 0;
    }
  }
  
  if ( gpx_append(pos) == 0 )
  {
    return 0;
  }
  
  gpx_unmount();
  return 1;
}

