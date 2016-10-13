////////////////////////////////////////////
// Written by Jacob Rusch
// 10/3/2016
// code for watch that simulates the intro 
// load screen on Tom Clancey's The Division
// only for Dorite
////////////////////////////////////////////

#include <pebble.h>

///////////////////////
// weather variables //
///////////////////////
#define KEY_TEMP
#define KEY_CITY

////////////////////
// font variables //
////////////////////
#define DATE_FONT RESOURCE_ID_BORDA_BOLD_FONT_12
#define AM_PM_FONT RESOURCE_ID_BORDA_BOLD_FONT_10
#define COLON_FONT RESOURCE_ID_BORDA_BOLD_FONT_24
#define CLOCK_FONT RESOURCE_ID_BORDA_FONT_36

static Window *s_main_window;
static Layer *s_circle, *s_seconds_circle, *s_battery_layer;
static GFont s_font, s_small_font, s_colon_font, s_clock_font;
static GBitmap *s_icon, *s_lightening, *s_bluetooth;
static BitmapLayer *s_icon_layer, *s_lightening_layer, *s_bluetooth_layer;
static TextLayer *s_date_layer, *s_colon_layer, *s_hour_layer, *s_minute_layer, *s_ampm_layer, *s_temp_layer, *s_city_layer;
static BatteryChargeState battery_state;
static int battery_percent;
static bool charging;
static int buf[3] = {-16, -8, 7};

///////////////////////////
// update battery status //
// horizontal            //
///////////////////////////
static void battery_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  // draw horizontal battery icon (dynamic)
  int start_x = 18;
  int start_y = 70;
  int batt_width = 4; // 3 is slim, 5 is fat
  int batt = battery_percent/10;
  graphics_draw_round_rect(ctx, GRect(start_x, start_y, 14, batt_width+4), 1);
  graphics_fill_rect(ctx, GRect(start_x+2, start_y+2, batt, batt_width), 1, GCornerNone);
  graphics_fill_rect(ctx, GRect(start_x+14, start_y+2, 1, batt_width), 0, GCornerNone); 
  
  // set visibility of charging icon
  layer_set_hidden(bitmap_layer_get_layer(s_lightening_layer), !charging);  
}

////////////////////
// update circles //
////////////////////
static void circle_update_proc(Layer *layer, GContext *ctx) {
  // outer ring
  GRect bounds = GRect(buf[0], buf[0], 144-(buf[0]*2), 168-(buf[0]*2));
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 3, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));
  
  // inner 6 ring
  bounds = GRect(buf[1], buf[1], 144-(buf[1]*2), 168-(buf[1]*2));
  for(int i=0; i<6; i++) {
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 10, DEG_TO_TRIGANGLE((i*60)+2), DEG_TO_TRIGANGLE((i*60)+58));
  }
  
  // inner solid ring
  bounds = GRect(buf[2], buf[2], 144-(buf[2]*2), 168-(buf[2]*2));
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 6, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));  
}

//////////////////////////////
// update seconds in circle //
//////////////////////////////
static void seconds_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  GRect bounds = GRect(buf[1], buf[1], 144-(buf[1]*2), 168-(buf[1]*2));
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  int32_t second_angle = t->tm_sec%6;
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 10, DEG_TO_TRIGANGLE((second_angle*60)+2), DEG_TO_TRIGANGLE((second_angle*60)+58));
}

//////////////////////
// load main window //
//////////////////////
static void main_window_load(Window *window) {
  // set background color
  window_set_background_color(window, GColorBlack); // default GColorWhite
  
  // get bounds of face
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //////////////////
  // create fonts //
  //////////////////
  s_font = fonts_load_custom_font(resource_get_handle(DATE_FONT));
  s_small_font = fonts_load_custom_font(resource_get_handle(AM_PM_FONT));
  s_colon_font = fonts_load_custom_font(resource_get_handle(COLON_FONT));
  s_clock_font = fonts_load_custom_font(resource_get_handle(CLOCK_FONT));
  
  ////////////////////
  // create circles //
  ////////////////////
  s_circle = layer_create(bounds);
  layer_set_update_proc(s_circle, circle_update_proc);
  layer_add_child(window_layer, s_circle); // add layer to window  
  
  /////////////////////////
  // draw seconds circle //
  /////////////////////////
  s_seconds_circle = layer_create(bounds);
  layer_set_update_proc(s_seconds_circle, seconds_update_proc);
  layer_add_child(window_layer, s_seconds_circle); // add layer to window     
  
  /////////////////
  // create icon //
  /////////////////
  s_icon = gbitmap_create_with_resource(RESOURCE_ID_DIVISION_ICON_BLACK);
  s_icon_layer = bitmap_layer_create(GRect(56, 28, 32, 32));
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_icon_layer, s_icon);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer)); // add layer to window
  
  ///////////////////////
  // create date layer //
  ///////////////////////
  s_date_layer = text_layer_create(GRect(24, 54, 96, 15));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite); 
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  text_layer_set_font(s_date_layer, s_font);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer)); 
  
  ///////////////////////////////////////////
  // create battery layer for dynamic icon //
  ///////////////////////////////////////////
  s_battery_layer = layer_create(bounds);
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);    
  
  ////////////////////
  // bluetooth icon //
  ////////////////////
  s_bluetooth = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_WHITE_ICON);
  s_bluetooth_layer = bitmap_layer_create(GRect(24, 79, 14, 14));
  bitmap_layer_set_compositing_mode(s_bluetooth_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bluetooth_layer, s_bluetooth); 
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bluetooth_layer));  
      
  ///////////////////
  // charging icon //
  ///////////////////
  s_lightening = gbitmap_create_with_resource(RESOURCE_ID_LIGHTENING_WHITE_ICON);
  s_lightening_layer = bitmap_layer_create(GRect(14, 79, 14, 14));
  bitmap_layer_set_compositing_mode(s_lightening_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_lightening_layer, s_lightening); 
  layer_add_child(window_layer, bitmap_layer_get_layer(s_lightening_layer));   
  
  ///////////////////////
  // create hour layer //
  ///////////////////////
  s_hour_layer = text_layer_create(GRect(32, 58, 33, 36));
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorWhite); 
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentRight);
  text_layer_set_font(s_hour_layer, s_clock_font);
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));    
  
  ////////////////////////
  // create colon layer //
  ////////////////////////
  s_colon_layer = text_layer_create(GRect(0, 68, 144, 30));
  text_layer_set_background_color(s_colon_layer, GColorClear);
  text_layer_set_text_color(s_colon_layer, GColorWhite); 
  text_layer_set_text_alignment(s_colon_layer, GTextAlignmentCenter);
  text_layer_set_font(s_colon_layer, s_colon_font);
  text_layer_set_text(s_colon_layer, ":");
  layer_add_child(window_layer, text_layer_get_layer(s_colon_layer)); 

  /////////////////////////
  // create minute layer //
  /////////////////////////
  s_minute_layer = text_layer_create(GRect(74, 58, 33, 36));
  text_layer_set_background_color(s_minute_layer, GColorClear);
  text_layer_set_text_color(s_minute_layer, GColorWhite); 
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentLeft);
  text_layer_set_font(s_minute_layer, s_clock_font);
  layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));  
  
  ////////////////////////
  // create am/pm layer //
  ////////////////////////
  s_ampm_layer = text_layer_create(GRect(108, 69, 19, 13));
  text_layer_set_text_alignment(s_ampm_layer, GTextAlignmentCenter);
  text_layer_set_font(s_ampm_layer, s_small_font);
  layer_add_child(window_layer, text_layer_get_layer(s_ampm_layer)); 
  
  ///////////////////////
  // create temp layer //
  ///////////////////////
  s_temp_layer = text_layer_create(GRect(107, 82, 20, 14));
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text_color(s_temp_layer, GColorWhite);   
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
  text_layer_set_font(s_temp_layer, s_font);
  layer_add_child(window_layer, text_layer_get_layer(s_temp_layer));  
  
  ///////////////////////
  // create city layer //
  ///////////////////////
  s_city_layer = text_layer_create(GRect(17, 96, 110, 14));
  text_layer_set_background_color(s_city_layer, GColorClear);
  text_layer_set_text_color(s_city_layer, GColorWhite);
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);
  text_layer_set_font(s_city_layer, s_font);
  layer_add_child(window_layer, text_layer_get_layer(s_city_layer));        
}

///////////////////
// unload window //
///////////////////
static void main_window_unload(Window *window) {
  layer_destroy(s_circle);
  layer_destroy(s_seconds_circle);
}

static void update_time() {
  // get a tm strucutre
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // %a = Day (Mon)
  // %m = Month (01..12)
  // %d = Day (01..31)
  static char s_date_buffer[32];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %m-%d", tick_time); // %n = line break

  // write the hour into a buffer
  static char s_hour_buffer[8];
//   strftime(s_hour_buffer, sizeof(s_hour_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
  strftime(s_hour_buffer, sizeof(s_hour_buffer), "%I", tick_time);
  
  // write the minutes into a buffer
  static char s_minute_buffer[8];
  strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
  
  static char s_ampm_buffer[8];
  strftime(s_ampm_buffer, sizeof(s_ampm_buffer), "%p", tick_time);
  
  // display this date, hour, and minute on the text layers
  text_layer_set_text(s_date_layer, s_date_buffer);
  text_layer_set_text(s_hour_layer, s_hour_buffer);
  text_layer_set_text(s_minute_layer, s_minute_buffer);
  text_layer_set_text(s_ampm_layer, s_ampm_buffer);  
}

static void time_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  layer_mark_dirty(s_seconds_circle);
  
  /////////////////////////////////////////
  // Get weather update every 30 minutes //
  /////////////////////////////////////////
  if(tick_time->tm_min % 30 == 0) {
    
    //////////////////////
    // Begin dictionary //
    //////////////////////
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    //////////////////////////
    // Add a key-value pair //
    //////////////////////////
    dict_write_uint8(iter, 0, 0);

    ///////////////////////
    // Send the message! //
    ///////////////////////
    app_message_outbox_send();
  }    
}

/////////////////////////////////////
// registers battery update events //
/////////////////////////////////////
static void battery_handler(BatteryChargeState charge_state) {
  battery_percent = charge_state.charge_percent;
  if(charge_state.is_charging || charge_state.is_plugged) {
    charging = true;
  } else {
    charging = false;
  }
  // force update to circle
  layer_mark_dirty(s_battery_layer);
}

////////////////////////////////////
// registers for bluetooth events //
////////////////////////////////////
static void bluetooth_callback(bool connected) {
  // show icon if connected
  layer_set_hidden(bitmap_layer_get_layer(s_bluetooth_layer), !connected); 
  // shake if disconnected from bluetooth
  if(!connected) {
    vibes_double_pulse();
  }
}

///////////////////////
// for weather calls //
///////////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  ////////////////////////////////
  // Store incoming information //
  ////////////////////////////////
  static char temp_buf[8];
  static char city_buf[32];
  
  //////////////////////////
  // Read tuples for data //
  //////////////////////////
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_KEY_TEMP);
  Tuple *city_tuple = dict_find(iterator, MESSAGE_KEY_KEY_CITY);

  //////////////////////////////////////
  // If all data is available, use it //
  //////////////////////////////////////
  if(temp_tuple && city_tuple) {
    /////////////////
    // temperature //
    /////////////////
    snprintf(temp_buf, sizeof(temp_buf), "%d°", (int)temp_tuple->value->int32);
    
    //////////
    // city //
    //////////
    snprintf(city_buf, sizeof(city_buf), "%s", city_tuple->value->cstring);
    
    text_layer_set_text(s_temp_layer, temp_buf);
    text_layer_set_text(s_city_layer, city_buf);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

////////////////////
// initialize app //
////////////////////
static void init() {
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // show window on the watch with animated=true
  window_stack_push(s_main_window, true);
  
  // start with updated time
  update_time();  
  
  // register with second ticks
  tick_timer_service_subscribe(SECOND_UNIT, time_handler);  
  
  // register with Battery State Service
  battery_state_service_subscribe(battery_handler);
  // force initial update
  battery_handler(battery_state_service_peek()); 
  
  // register with bluetooth state service
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  bluetooth_callback(connection_service_peek_pebble_app_connection());

  // Register weather callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);  
  
  // Open AppMessage for weather callbacks
  const int inbox_size = 40;
  const int outbox_size = 40;
  app_message_open(inbox_size, outbox_size);   
}

///////////////////////
// de-initialize app //
///////////////////////
static void deinit() {
  window_destroy(s_main_window);
}

/////////////
// run app //
/////////////
int main(void) {
  init();
  app_event_loop();
  deinit();
}