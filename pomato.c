/* Pomato -- Simple Clock/Pomodoro Timer */

/* 
 * This software is licensed under the BSD 0-Clause License; see LICENSE. 
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <raylib.h>
#include <libnotify/notify.h>

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
timer (time_t current_time, time_t end_time)
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

int
main (int argc, char **argv)
{

  int c;
  while ((c = getopt (argc, argv, ":v")) != -1)
    {
      switch (c)
	{
	case 'v':
	  printf ("Pomato 0.1\n");
	  printf ("Copyright (C) 2024 \n");
	  printf ("License: Zero-Clause BSD\n");
	  return 0;
	  break;
	case '?':
	  fprintf (stderr, "unknown option: -%c\n", optopt);
	  return 1;
	  break;
	}
    }
  
  InitWindow (400, 400, "Pomato - Pomodoro Timer");

  SetTargetFPS (60);

  struct tm *clock;
  State state = MENU;
  Convert counter = {0};
  Vector2 timer_pos = {0};
  Vector2 clock_pos = {0};
  char timer_str[6];
  char clock_str[6];
  const int timer_fontSize = 86;
  int until_pause = 0;
  int text_width = 0;
  int text_height = 0;
  int new_width = 0;
  int new_height = 0;
  int time_diff = 0;
  time_t current_time;
  time (&current_time);
  time_t end_work_time = calc_time (current_time, WORK_TIME);
  time_t end_pause_time = calc_time (current_time, PAUSE_TIME);
  time_t end_long_pause_time = calc_time (current_time, LONG_PAUSE_TIME);

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

      switch (state)
	{
	case MENU:
	  snprintf (clock_str, sizeof (clock_str), "%.2d:%.2d",
		    clock->tm_hour, clock->tm_min);
	  break;
	case WORK:
	  /* work timer */
	  time_diff = timer (current_time, end_work_time);
	  counter = convert_seconds (time_diff);
	  snprintf (timer_str, sizeof (timer_str), "%.2d:%.2d",
		    counter.minutes, counter.seconds);
	  snprintf (clock_str, sizeof (clock_str), "%.2d:%.2d",
		    clock->tm_hour, clock->tm_min);
	  if (time_diff == 0)
	    {
	      until_pause += 1;
	      end_pause_time = current_time + (PAUSE_TIME * 60);
	      if (until_pause >= 4)
		{
		  end_long_pause_time = current_time + (LONG_PAUSE_TIME * 60);
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
	  time_diff = timer (current_time, end_pause_time);
	  counter = convert_seconds (time_diff);
	  snprintf (timer_str, sizeof (timer_str), "%.2d:%.2d",
		    counter.minutes, counter.seconds);
	  snprintf (clock_str, sizeof (clock_str), "%.2d:%.2d",
		    clock->tm_hour, clock->tm_min);
	  if (time_diff == 0)
	    {
	      end_work_time = current_time + (WORK_TIME * 60);
	      send_notifiction ("Pomato", "Time to Work!");
	      state = WORK;
	    }
	  break;
	case LONG_PAUSE:
	  /* long pause */
	  time_diff = timer (current_time, end_long_pause_time);
	  counter = convert_seconds (time_diff);
	  snprintf (timer_str, sizeof (timer_str), "%.2d:%.2d",
		    counter.minutes, counter.seconds);
	  snprintf (clock_str, sizeof (clock_str), "%.2d:%.2d",
		    clock->tm_hour, clock->tm_min);
	  if (time_diff == 0)
	    {
	      end_work_time = current_time + (WORK_TIME * 60);
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
	  DrawText (clock_str, clock_pos.x, clock_pos.y, timer_fontSize,
		    FONT_COLOR);
	  DrawText ("[SPACE] to START! - [ENTER] to RESTART!", clock_pos.x,
		    (clock_pos.y+timer_fontSize-10), 10, FONT_COLOR);
	  break;
	case WORK:
	  ClearBackground (WORK_BG);
	  DrawText (timer_str, timer_pos.x, timer_pos.y, timer_fontSize,
		    FONT_COLOR);
	  DrawText (clock_str, 10, 10, 20, FONT_COLOR);
	  break;
	case PAUSE:
	  ClearBackground (PAUSE_BG);
	  DrawText (timer_str, timer_pos.x, timer_pos.y, timer_fontSize,
		    FONT_COLOR);
	  DrawText (clock_str, 10, 10, 20, FONT_COLOR);
	  break;
	case LONG_PAUSE:
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

  CloseWindow ();

  return 0;

}
