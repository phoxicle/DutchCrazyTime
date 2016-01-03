#include <pebble.h>
#include <stdlib.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *flag_layer;

static const char * num_strings[] = {"nul", "één", "twee", "drie", "vier", "vijf", "zes", "zeven", "acht", "negen", 
                        "tien", "elf", "twaalf", "dertien", "viertien"};

static const char* get_hour_str(int hour) {
  int normalized_hour;
  if (hour > 12) {
    normalized_hour = hour - 12;
  } else if (hour == 0) {
    normalized_hour = 12;
  } else {
    normalized_hour = hour;
  }
  return num_strings[normalized_hour];
}

static const char* get_min_str(int min) {
  return num_strings[min];
}

static void get_time_string(char *s_buffer, int hour, int min) {
  int size = 50;
  if (min == 0) {
    // elf uur
    snprintf(s_buffer, size, "%s\nuur", get_hour_str(hour));
  } else if (0 < min && min < 15) {
    // drie over elf
    snprintf(s_buffer, size, "%s\nover\n%s", get_min_str(min), get_hour_str(hour));
  } else if (min == 15) {
    // kwart over elf
    snprintf(s_buffer, size, "kwart\nover\n%s", get_hour_str(hour));
  } else if (15 < min && min < 30) {
    // drie voor half twaalf
    snprintf(s_buffer, size, "%s\nvoor\nhalf\n%s", get_min_str(30-min), get_hour_str(hour+1));
  } else if (min == 30) { 
    // half twaalf
    snprintf(s_buffer, size, "half\n%s", get_hour_str(hour+1));
  } else if (30 < min && min < 45) {
    // drie over half twaalf
    snprintf(s_buffer, size, "%s\nover\nhalf\n%s", get_min_str(min-30), get_hour_str(hour+1));
  } else if (min == 45) {
    // kwart voor twaalf
    snprintf(s_buffer, size, "kwart\nvoor\n%s", get_hour_str(hour+1));
  } else { // 45 < min && min < 60
    // drie voor twaalf
    snprintf(s_buffer, size, "%s\nvoor\n%s", get_min_str(60-min), get_hour_str(hour+1));
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Calculate the text
  int hour = tick_time->tm_hour;
  int min = tick_time->tm_min;
  
  static char s_buffer[50];
  get_time_string(s_buffer, hour, min);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void flag_layer_draw(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const int16_t h = bounds.size.h / 3;
   
  // Red
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, h), 0, GCornerNone);
  
  // White
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, h, bounds.size.w, h), 0, GCornerNone);
  
  // Blue
  graphics_context_set_fill_color(ctx, GColorBlueMoon);
  graphics_fill_rect(ctx, GRect(0, 2*h, bounds.size.w, h), 0, GCornerNone);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the flag layer
  flag_layer = layer_create(bounds);
  layer_set_update_proc(flag_layer, flag_layer_draw);
  layer_add_child(window_layer, flag_layer);
  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(GRect(0, 25, bounds.size.w, 150));
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  // Destroy flag layer
  layer_destroy(flag_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  //window_set_background_color(s_main_window, GColorBlue);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}