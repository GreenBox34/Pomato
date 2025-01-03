/* Pomato – Simple Clock/Pomodoro Timer */

/* 
 * This software is licensed under the BSD 0-Clause License; see LICENSE. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>

#include <raylib.h>
#include <libnotify/notify.h>

#include "back_png.h"
#include "clock_png.h"
#include "question_mark_png.h"

#define FONT_COLOR    (Color){235, 219, 178, 255}
#define MENU_BG       (Color){29,  32,  33,  255}
#define WORK_BG       (Color){204, 36,  29,  255}
#define PAUSE_BG      (Color){152, 151, 26,  255}
#define LONG_PAUSE_BG (Color){69,  133, 136, 255}

#define WORK_TIME 25
#define PAUSE_TIME 5
#define LONG_PAUSE_TIME 15

typedef enum
{
  MENU,
  WORK,
  PAUSE,
  LONG_PAUSE
} State;

typedef struct Convert
{
  int minutes;
  int seconds;
} Convert;

Convert
convert_seconds (int seconds)
{
  Convert time;

  time.minutes = (seconds / 60);
  time.seconds = (seconds % 60);

  return time;
}

int
get_time_diff (time_t current_time, time_t end_time)
{
  return (int) difftime (end_time, current_time);
}

time_t
calc_time (time_t current_time, int timer_time)
{
  return current_time + (timer_time * 60);
}

void
send_notifiction (char *title, char *message)
{
  NotifyNotification *notify;
  notify_init ("pomato");
  notify = notify_notification_new (title, message, NULL);

  notify_notification_show (notify, NULL);
}

bool
draw_button (Vector2 position, Texture2D texture, Color color)
{
  Rectangle rect_bounds = {position.x, position.y, 32, 32};

  DrawTextureRec (texture, (Rectangle){0, 0, 32, 32}, position, color);

  if (IsMouseButtonPressed (MOUSE_LEFT_BUTTON))
    {
      Vector2 mouse_pos = GetMousePosition();
      if (CheckCollisionPointRec(mouse_pos, rect_bounds))
	return true;
    }

  return false;

}

Texture2D
load_texture_from_header_file (const unsigned char *fileData, const int dataSize)
{

  Image raw_image = LoadImageFromMemory (".png", fileData, dataSize);
  if (raw_image.data == NULL)
    {
      fprintf (stderr, "error: cloud not load texture.\n");
      exit (1);
    }

  Texture2D texture = LoadTextureFromImage (raw_image);
  if (IsTextureReady (texture))
    {
      UnloadImage (raw_image);
    }

  return texture;

}

int
main (int argc, char **argv)
{

  static struct option long_options[] =
    {
      {"version", no_argument, 0, 'v'},
      {0, 0, 0, 0}
    };

  int c;
  int option_index = 0;
  while ((c = getopt_long (argc, argv, ":v", long_options, &option_index)) != -1)
    {
      switch (c)
	{
	case 'v':
	  printf ("Pomato 0.1\n");
	  printf ("Copyright (C) 2024 \n");
	  printf ("License: Zero-Clause BSD\n");
	  exit(EXIT_SUCCESS);
	  break;
	case '?':
	  fprintf (stderr, "unknown option: -%c\n", optopt);
	  exit(EXIT_FAILURE);
	  break;
	}
    }
  
  InitWindow (400, 400, "Pomato - Clock/Pomodoro Timer");

  struct tm *clock;
  State state = MENU;
  Convert counter = {0};
  Vector2 timer_pos = {0};
  Vector2 clock_pos = {0};
  Vector2 info_button_pos = {0};
  Vector2 start_pomodoro_pos = {0};
  char timer_str[6];
  char clock_str[6];
  const int timer_fontSize = 80;
  int until_pause = 0;
  int text_width = 0;
  int text_height = 0;
  int new_width = 0;
  int new_height = 0;
  int remaining_time = 0;
  int button_size = 32;
  bool start_pomodoro = false;
  bool info_button = false;
  time_t current_time;
  time (&current_time);
  time_t end_work_time = calc_time (current_time, WORK_TIME);
  time_t end_pause_time = calc_time (current_time, PAUSE_TIME);
  time_t end_long_pause_time = calc_time (current_time, LONG_PAUSE_TIME);

  Texture2D info_button_texture = load_texture_from_header_file(question_mark_png, question_mark_png_len);
  Texture2D back_button_texture = load_texture_from_header_file(back_png, back_png_len);
  Texture2D clock_texture = load_texture_from_header_file(clock_png, clock_png_len);

  SetTargetFPS (60);

  while (!WindowShouldClose ())
    {

      /* update current time. */
      time (&current_time);
      /* update localtime */
      clock = localtime (&current_time);

      if (state == MENU)
	{
	  /* update clock_str position. */
	  new_width = GetScreenWidth ();
	  new_height = GetScreenHeight ();

	  text_width = MeasureText (clock_str, timer_fontSize);
	  text_height = timer_fontSize;

	  clock_pos.x = (new_width / 2) - (text_width / 2);
	  clock_pos.y = (new_height / 2) - (text_height / 2);

	  /* update button positions */
	  info_button_pos.x = (new_width - button_size - 5);
	  info_button_pos.y = (new_height - new_height + 10);;

	  start_pomodoro_pos.x = (new_width - button_size*2 - 10);
	  start_pomodoro_pos.y = (new_height - new_height + 10);

	}
      else
	{
	  /* update timer_str position. */
	  new_width = GetScreenWidth ();
	  new_height = GetScreenHeight ();

	  text_width = MeasureText (timer_str, timer_fontSize);
	  text_height = timer_fontSize;

	  timer_pos.x = (new_width / 2) - (text_width / 2);
	  timer_pos.y = (new_height / 2) - (text_height / 2);

	  /* update button positions */
	  start_pomodoro_pos.x = (new_width - button_size - 5);
	  start_pomodoro_pos.y = (new_height - new_height + 5);

	}

      if (IsKeyPressed (KEY_SPACE))
	{
	  if (state != WORK)
	    send_notifiction("Pomato", "Time to Work!");
	  state = WORK;
	}
     
      /* reset */
      if (IsKeyPressed (KEY_ENTER))
	{
	  state = MENU;
	  until_pause = 0;

	  /* reset time. */
	  time (&current_time);
	  end_work_time = calc_time (current_time, WORK_TIME);
	  end_pause_time = calc_time (current_time, PAUSE_TIME);
	  end_long_pause_time = calc_time (current_time, LONG_PAUSE_TIME);

	}

      if (info_button == true)
	{
	  printf("info button clicked\n");
	}

      if (start_pomodoro == true)
	{
	  if (state == WORK || state == PAUSE || state == LONG_PAUSE)
	    {
	      start_pomodoro = false;
	      state = MENU;
	      until_pause = 0;

	      /* reset time. */
	      time (&current_time);
	      end_work_time = calc_time (current_time, WORK_TIME);
	      end_pause_time = calc_time (current_time, PAUSE_TIME);
	      end_long_pause_time = calc_time (current_time, LONG_PAUSE_TIME);
	    }
	  else
	    {
	      start_pomodoro = true;
	      state = WORK;
	    }
	}

      switch (state)
	{
	case MENU:
	  snprintf (clock_str, sizeof (clock_str), "%.2d:%.2d",
		    clock->tm_hour, clock->tm_min);
	  break;
	case WORK:
	  /* work timer */
	  remaining_time = get_time_diff (current_time, end_work_time);
	  counter = convert_seconds (remaining_time);
	  snprintf (timer_str, sizeof (timer_str), "%.2d:%.2d",
		    counter.minutes, counter.seconds);
	  snprintf (clock_str, sizeof (clock_str), "%.2d:%.2d",
		    clock->tm_hour, clock->tm_min);
	  if (remaining_time == 0)
	    {
	      until_pause += 1;
	      end_pause_time = calc_time(current_time, PAUSE_TIME);
	      if (until_pause >= 4)
		{
		  end_long_pause_time = calc_time(current_time, LONG_PAUSE_TIME);
		  send_notifiction ("Pomato", "Time for a long pause. Grab some tea!");
		  state = LONG_PAUSE;
		}
	      else
		{
		  send_notifiction ("Pomato", "Time for a pause.");
		  state = PAUSE;
		}
	    }
	  break;
	case PAUSE:
	  /* pause timer */
	  remaining_time = get_time_diff (current_time, end_pause_time);
	  counter = convert_seconds (remaining_time);
	  snprintf (timer_str, sizeof (timer_str), "%.2d:%.2d",
		    counter.minutes, counter.seconds);
	  snprintf (clock_str, sizeof (clock_str), "%.2d:%.2d",
		    clock->tm_hour, clock->tm_min);
	  if (remaining_time == 0)
	    {
	      end_work_time = calc_time(current_time, WORK_TIME);
	      send_notifiction ("Pomato", "Time to Work!");
	      state = WORK;
	    }
	  break;
	case LONG_PAUSE:
	  /* long pause */
	  remaining_time = get_time_diff (current_time, end_long_pause_time);
	  counter = convert_seconds (remaining_time);
	  snprintf (timer_str, sizeof (timer_str), "%.2d:%.2d",
		    counter.minutes, counter.seconds);
	  snprintf (clock_str, sizeof (clock_str), "%.2d:%.2d",
		    clock->tm_hour, clock->tm_min);
	  if (remaining_time == 0)
	    {
	      end_work_time = calc_time(current_time, WORK_TIME);
	      until_pause = 0;
	      send_notifiction ("Pomato", "Time to Work!");
	      state = WORK;
	    }
	  break;
	default:
	  break;
	}

      BeginDrawing ();

      switch (state)
	{
	case MENU:
	  ClearBackground (MENU_BG);
	  start_pomodoro = draw_button (start_pomodoro_pos, clock_texture, WHITE);
	  info_button = draw_button (info_button_pos, info_button_texture, WHITE);
	  DrawText (clock_str, clock_pos.x, clock_pos.y, timer_fontSize,
		    FONT_COLOR);
	  DrawText ("[SPACE] to START! - [ENTER] to RESTART!", clock_pos.x,
		    (clock_pos.y+timer_fontSize-10), 10, FONT_COLOR);
	  break;
	case WORK:
	  ClearBackground (WORK_BG);
	  start_pomodoro = draw_button (start_pomodoro_pos, back_button_texture, WHITE);
	  DrawText (timer_str, timer_pos.x, timer_pos.y, timer_fontSize,
		    FONT_COLOR);
	  DrawText (clock_str, 10, 10, 20, FONT_COLOR);
	  break;
	case PAUSE:
	  ClearBackground (PAUSE_BG);
	  start_pomodoro = draw_button (start_pomodoro_pos, back_button_texture, WHITE);
	  DrawText (timer_str, timer_pos.x, timer_pos.y, timer_fontSize,
		    FONT_COLOR);
	  DrawText (clock_str, 10, 10, 20, FONT_COLOR);
	  break;
	case LONG_PAUSE:
	  start_pomodoro = draw_button (start_pomodoro_pos, back_button_texture, WHITE);
	  ClearBackground (LONG_PAUSE_BG);
	  DrawText (timer_str, timer_pos.x, timer_pos.y, timer_fontSize,
		    FONT_COLOR);
	  DrawText (clock_str, 10, 10, 20, FONT_COLOR);
	  break;
	default:
	  break;
	}

      EndDrawing ();

    }

  UnloadTexture (clock_texture);
  UnloadTexture (back_button_texture);
  UnloadTexture (info_button_texture);

  CloseWindow ();

  return EXIT_SUCCESS;

}
