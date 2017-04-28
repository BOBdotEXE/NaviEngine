#include <pebble.h>
#include "main.h"
static Window *s_main_window; //make a new slide
static TextLayer *s_time_layer; //make a new text layer for time
static TextLayer *s_title_layer; //mew text layer for 'pebble terminal' text
static TextLayer *s_battery_layer; //text layer for battery percentage
static TextLayer *s_date_layer; //Layer layer for current date 
static TextLayer *s_dow_layer; //Layer layer for current date 
static BitmapLayer *s_background_layer; //make an (background)image layer
static GBitmap *s_background_bitmap; //image for the (background) layer
static BitmapLayer *s_hp_layer; //make an (background)image layer
static GBitmap *s_hp_bitmap; //image for the (background) layer
static BitmapLayer *s_clock_bg_layer; //make an image layer
static GBitmap *s_clock_bg_bitmap; //image for the layer
static GBitmap     *s_disconnected;
static TextLayer *s_disconnected_layer;
static GBitmap *s_white_bitmap, *s_black_bitmap;  //black bmp (part of transparency)
static BitmapLayer *s_white_layer, *s_black_layer,*s_ani_layer; //white bmp (part of tansparency)
static GBitmap *s_frame_1,*s_frame_2_6,*s_frame_7_8,*s_frame_9; //bitmap container for animation frames
static GBitmap *s_clock_2,*s_clock_3,*s_clock_4,*s_clock_closed; //bitmap container for animation, and frames
static int frame =0,background_frame=0; //set up frame counter for link and background
static int blink_sec; //seconds since last blink

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//Here's what you CAN edit for your custom navi! (DO NOT EDIT ABOVE THIS LINE)--------------------------------------------------------------------------------
static int eyes_X=24,eyes_Y=64,eyes_s_x=49,eyes_s_y=30; //EYE Position and size!|Eye position X (<>) /Eye position Y (^v) / eye width (<>) / Eye Height (^v) [default 24,64,49,30]
bool vib_hour =false, vib_on_no_bt=false; // vibrate on hour + Vibrate on bluetooth disconnect (default false / (default 18))
bool post_game =false;                    //change how battery precent looks [when true 100% battery =1000 in the hp box] (default false)
static int jackin_time =600;             //how long dose the jack in animation last in 1/1000sec (default 600)
bool blink =true;                       // Should megaman blink at all? (default= true)
bool show_dis_icon=true;              // Show the red icon when Bluetooth is disconnected? (default true)
int blink_timer = 18;                    // Blink every XX seconds [Affects battery life] (default 18)
bool show_date=true, show_day_of_week =true, show_peb_term_text=true,animate_clock=true; // Show the date?/ Show the Day of the week? / Show the text 'Pebble Terminal'? / Animate the clock (default [ALL] true)
// Dont Edit anything below!-----------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
int last_hour =24;
static int jack_in_progress=0;
static Layer *s_teleport_back,*s_teleport_mid,*s_teleport_front;



  //handler for battery
  static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display  
  int batt = battery_state_service_peek().charge_percent;
  static char s_battery_buffer[5];  //char for the battery percent
  if (batt<31)
    {
    text_layer_set_text_color(s_battery_layer, GColorRed); //red text
    layer_mark_dirty(text_layer_get_layer(s_battery_layer));
    }
  else
    {
    text_layer_set_text_color(s_battery_layer, GColorCeleste); //blue text
    layer_mark_dirty(text_layer_get_layer(s_battery_layer));
    }
    
    
  if (post_game==false)
    {
    if (new_state.is_charging==false)
      snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d", batt); //get the battery info
    else
       snprintf(s_battery_buffer, sizeof(s_battery_buffer), "---");
    }
  if (post_game ==true)
    {
    if (new_state.is_charging==false)
      snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d0", batt); //get he battery info, and add a zero
     else
       snprintf(s_battery_buffer, sizeof(s_battery_buffer), "----");
  }
  text_layer_set_text(s_battery_layer, s_battery_buffer); //set the layer battery text (percent + text) to the battery layer
  layer_mark_dirty(text_layer_get_layer(s_battery_layer));
  }



static void animate() {
  // Move boxes
  int x_pos=41;
  
  
  //size for teleport's back layer
  //part '1' beam enters the screen
  GRect start = GRect(x_pos, -168, 9, 168);
  GRect finish = GRect(x_pos, 22, 9, 168);
  PropertyAnimation *prop_beam_enter_back = property_animation_create_layer_frame(s_teleport_back, &start, &finish);
  Animation *anim_beam_enter_back = property_animation_get_animation(prop_beam_enter_back);
  animation_set_duration(anim_beam_enter_back, jackin_time*0.8);
 
  //size for teleport's mid layer
  //part '1' beam enters the screen  
  start = GRect(x_pos+1, -168, 7, 168);
  finish = GRect(x_pos+1, 22, 7, 168); //it's 2 pixel thinner on both sizes, so start it one pixel to the right, and make it 2pxl smaller
  PropertyAnimation *prop_beam_enter_mid = property_animation_create_layer_frame(s_teleport_mid, &start, &finish);
  Animation *anim_beam_enter_mid = property_animation_get_animation(prop_beam_enter_mid);
  animation_set_duration(anim_beam_enter_mid, jackin_time*0.8);
   
  //size for teleport's front layer
  //part '1' beam enters the screen  
  start = GRect(x_pos+2, -168, 5, 168);
  finish = GRect(x_pos+2, 22, 5, 168); //it's 2 pixel thinner on both sizes, so start it one pixel to the right, and make it 2pxl smaller
  PropertyAnimation *prop_beam_enter_front = property_animation_create_layer_frame(s_teleport_front, &start, &finish);
  Animation *anim_beam_enter_front = property_animation_get_animation(prop_beam_enter_front);
  animation_set_duration(anim_beam_enter_front, jackin_time*0.8);
  
  
  //Part '2' beam widens (back)
  start =GRect(x_pos, 22, 9, 168); //start where part 'A' left off.
  finish = GRect(10,22, 60,146); //and become wider
  PropertyAnimation *prop_beam_widen_back = property_animation_create_layer_frame(s_teleport_back, &start, &finish);
  Animation *anim_beam_widen_back = property_animation_get_animation(prop_beam_widen_back);
  animation_set_duration(anim_beam_widen_back, jackin_time*0.2);
  //Part '2' beam widens (mid)
  start =GRect(x_pos+1, 22, 7, 168); //start where part 'A' left off.
  finish = GRect(15,22, 50,146); //and become wider,  //it's now 4 pixel thinner on both sizes, so start it one pixel to the right, and make it 1pxl smaller
  PropertyAnimation *prop_beam_widen_mid = property_animation_create_layer_frame(s_teleport_mid, &start, &finish);
  Animation *anim_beam_widen_mid = property_animation_get_animation(prop_beam_widen_mid);
  animation_set_duration(anim_beam_widen_mid, jackin_time*0.2);
  //Part '2' beam widens (front)
  start =GRect(x_pos+2, 22, 5, 168); //start where part 'A' left off.
  finish = GRect(20,22, 45,146); //and become wider,  //it's now 4 pixel thinner on both sizes, so start it one pixel to the right, and make it 1pxl smaller
  PropertyAnimation *prop_beam_widen_front = property_animation_create_layer_frame(s_teleport_front, &start, &finish);
  Animation *anim_beam_widen_front = property_animation_get_animation(prop_beam_widen_front);
  animation_set_duration(anim_beam_widen_front, jackin_time*0.2);  
  
  // Create two sequence animations
  Animation *seq_back = animation_sequence_create(anim_beam_enter_back,anim_beam_widen_back, NULL);
  Animation *seq_mid = animation_sequence_create(anim_beam_enter_mid,anim_beam_widen_mid, NULL);
  Animation *seq_front = animation_sequence_create(anim_beam_enter_front,anim_beam_widen_front, NULL);
  
  
  // Create a spawn animation using the two sequences, set play count and begin
  Animation *spawn = animation_spawn_create(seq_back, seq_mid,seq_front, NULL);
  animation_set_play_count(spawn, 1);
  animation_schedule(spawn);
}




static void update_back(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorMidnightGreen);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);
}

static void update_mid(Layer *layer, GContext *ctx) {

    graphics_context_set_fill_color(ctx, GColorCyan);

  graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);
}
static void update_front(Layer *layer, GContext *ctx) {

    graphics_context_set_fill_color(ctx, GColorCeleste);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);
}



static void main_window_load(Window *window) { //main window created
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
  //Background
  // Create GBitmap, then set to created BitmapLayer  (Backgound HAS TO BE BEFORE text)
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_pet_bg);  //RESOURCE_ID_"Name of image file"
  s_background_layer = bitmap_layer_create(window_bounds);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap); //set the background to the image layer
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  //tranparent foreground
  //https://github.com/pebble/pebble-sdk-examples/blob/master/watchapps/feature_image_transparent/src/feature_image_transparent.c#L68
 /** #ifdef PBL_PLATFORM_APLITE //if running on older pebble
     s_black_bitmap = gbitmap_create_with_resource(RESOURCE_ID_megaman_old_BLACK);
     s_white_bitmap = gbitmap_create_with_resource(RESOURCE_ID_megaman_old_WHITE);
  #elif PBL_PLATFORM_BASALT  //if running on pebble time
    s_white_bitmap = gbitmap_create_with_resource(RESOURCE_ID_megaman);
 #endif
 **/
  
    

  
/**
    //Foreground
  // Create GBitmap, then set to created BitmapLayer  (Backgound HAS TO BE BEFORE text)
  s_foreground_bitmap = gbitmap_create_with_resource(RESOURCE_ID_pet_fg_WHITE);  //RESOURCE_ID_"Name of image file"
  s_foreground_layer = bitmap_layer_create(GRect(0, 0, 144, 168)); //set the size and poz of the bg layer
  bitmap_layer_set_bitmap(s_foreground_layer, s_foreground_bitmap); //set the image to the layer
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_foreground_layer));
**/
  

  //create clock background
    s_clock_bg_layer = bitmap_layer_create(GRect(41, 79, 130, 30));
    s_clock_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_clock_bg);
    bitmap_layer_set_bitmap(s_clock_bg_layer , s_clock_bg_bitmap);
    #if PBL_PLATFORM_BASALT  //if running on pebble time
      bitmap_layer_set_compositing_mode(s_clock_bg_layer, GCompOpSet);
    #endif
            //set up the clock background animation frames
        s_clock_2 =  gbitmap_create_with_resource(RESOURCE_ID_clock_2);
        s_clock_3 = gbitmap_create_with_resource(RESOURCE_ID_clock_3);
        s_clock_4= gbitmap_create_with_resource(RESOURCE_ID_clock_4);
        s_clock_closed =  gbitmap_create_with_resource(RESOURCE_ID_clock_closed);
      
    layer_add_child(window_layer,  bitmap_layer_get_layer(s_clock_bg_layer));
 
  
  // Create time Layer
  // OLD  POSISTION!!!!----  s_time_layer = text_layer_create(GRect(43, -6, 130, 30)); //create a text layer at 0x55 that's 144x50 [box size] Y-X
  s_time_layer = text_layer_create(GRect(41, 75, 130, 30)); //create a text layer at 0x55 that's 144x50 [box size] Y-X
  text_layer_set_background_color(s_time_layer, GColorClear); //clear background
    #ifdef PBL_PLATFORM_APLITE //if running on older pebble
    text_layer_set_text_color(s_time_layer, GColorBlack); /// black text
   #elif PBL_PLATFORM_BASALT  //if running on pebble time
    text_layer_set_text_color(s_time_layer, GColorOxfordBlue); //dark blue text
  #endif
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));//time of day
  
   

 // Create title TextLayer
  s_title_layer = text_layer_create(GRect(32, 10, 130, 30)); //create a text layer at 0x55 that's 144x50 [box size] Y-X
  text_layer_set_background_color(s_title_layer, GColorClear); //clear background
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
  text_layer_set_text_color(s_title_layer, GColorBlack); /// black text
  #elif PBL_PLATFORM_BASALT  //if running on pebble timeg
  text_layer_set_text_color(s_title_layer, GColorDukeBlue); //blue text
  #endif
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD)); //change the font
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_title_layer));//title text

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD)); //change the font
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter); //center align

  // Add it as a child layer to the Window's root layer
    if (show_peb_term_text==true)
  text_layer_set_text(s_title_layer, "PEbble Terminal");
  
  
   //set up hp box
  s_hp_bitmap = gbitmap_create_with_resource(RESOURCE_ID_hp_box);
  s_hp_layer = bitmap_layer_create(GRect(102,0, 41, 15));
  bitmap_layer_set_bitmap(s_hp_layer, s_hp_bitmap);                                    
  
  layer_add_child(window_layer, bitmap_layer_get_layer(s_hp_layer));
  
  // Create TextLayer For battery level
  // OLD  POSISTION!!!!----  s_battery_layer = text_layer_create(GRect(80, 48, window_bounds.size.w, window_bounds.size.h));
  s_battery_layer = text_layer_create(GRect(101, -2, 40,16));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight); //left align the text in this layer
  text_layer_set_background_color(s_battery_layer, GColorClear); //clear background
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
    text_layer_set_text_color(s_battery_layer, GColorBlack); /// black text
   #elif PBL_PLATFORM_BASALT  //if running on pebble time
    text_layer_set_text_color(s_battery_layer, GColorCeleste); //blue text
  #endif
  text_layer_set_font(s_battery_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HP_15))); //change the font
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));//Add battery layer to our window
  // Get the current battery level
  battery_handler(battery_state_service_peek());
  
  //Create Date layer:
  
  s_date_layer = text_layer_create(GRect(82,29, 136, 100));
  text_layer_set_background_color(s_date_layer, GColorClear);
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
  text_layer_set_text_color(s_date_layer, GColorBlack); /// black text
  #elif PBL_PLATFORM_BASALT  //if running on pebble time
  text_layer_set_text_color(s_date_layer, GColorElectricBlue); //blueish text
  #endif

  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  if (show_date== true)
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  
  
  
  
  //Create 'DAY OF WEEK layer:
  s_dow_layer = text_layer_create(GRect(83,46, 136, 100));
  text_layer_set_background_color(s_dow_layer, GColorClear);
   #ifdef PBL_PLATFORM_APLITE //if running on older pebble
    text_layer_set_text_color(s_dow_layer, GColorBlack); /// black text
   #elif PBL_PLATFORM_BASALT  //if running on pebble time
    text_layer_set_text_color(s_dow_layer, GColorDukeBlue); //dark blue text
  #endif
  
  text_layer_set_font(s_dow_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  if (show_day_of_week ==true)
    layer_add_child(window_layer, text_layer_get_layer(s_dow_layer));
  
  
  
  //MEGAMAN WILL BE THE LAST THING RENDERED
  //set up the 'white' portion
  s_white_layer = bitmap_layer_create(window_bounds); //render inline, but off screen
  s_white_bitmap = gbitmap_create_with_resource(RESOURCE_ID_megaman);
  bitmap_layer_set_bitmap(s_white_layer, s_white_bitmap); //set the white bitmap to it's layer
  
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
    bitmap_layer_set_compositing_mode(s_white_layer, GCompOpOr);
  
  #elif PBL_PLATFORM_BASALT  //if running on pebble time
    bitmap_layer_set_compositing_mode(s_white_layer, GCompOpSet);
  #endif
  layer_set_hidden( (Layer *) s_white_layer,true); 
  layer_add_child(window_layer, bitmap_layer_get_layer(s_white_layer));
  
  
  
  #ifdef PBL_PLATFORM_APLITE //classic pebble only, 
    //set up 'black portion
    s_black_layer = bitmap_layer_create(window_bounds);
    bitmap_layer_set_bitmap(s_black_layer, s_black_bitmap);
    bitmap_layer_set_compositing_mode(s_black_layer, GCompOpClear);      
    layer_add_child(window_layer, bitmap_layer_get_layer(s_black_layer));
  #endif
///DO NOT RENDER AFTER MEGAMAN! (only blank layers allowed)

    //Animation (blink) layer
        //set up the blink frames
        s_frame_1 =  gbitmap_create_with_resource(RESOURCE_ID_frame_1);
        s_frame_2_6 = gbitmap_create_with_resource(RESOURCE_ID_frame_2_6);
        s_frame_7_8= gbitmap_create_with_resource(RESOURCE_ID_frame_7_8);
        s_frame_9 =  gbitmap_create_with_resource(RESOURCE_ID_frame_9);
  
    s_ani_layer = bitmap_layer_create(GRect(eyes_X,eyes_Y, eyes_s_x, eyes_s_y));
    #ifdef PBL_PLATFORM_APLITE //if running on older pebble
      bitmap_layer_set_compositing_mode(s_ani_layer, GCompOpOr);
    #elif PBL_PLATFORM_BASALT  //if running on pebble time
      bitmap_layer_set_compositing_mode(s_ani_layer, GCompOpSet);
    #endif

  layer_add_child(window_layer, bitmap_layer_get_layer(s_ani_layer));//add the EMPTY animation layer!
  layer_set_hidden( (Layer *)s_ani_layer,true); //hide it.
  
  
  //'disconnected' icon layer 
  s_disconnected_layer = text_layer_create(GRect(2, -2, 130, 30)); //create a text layer at 0x55 that's 144x50 [box size] Y-X
  text_layer_set_background_color(s_disconnected_layer, GColorClear); //clear background
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
  text_layer_set_text_color(s_disconnected_layer, GColorBlack); /// black text
  #elif PBL_PLATFORM_BASALT  //if running on pebble timeg
  text_layer_set_text_color(s_disconnected_layer, GColorDarkCandyAppleRed); //blue text
  #endif
  text_layer_set_text_alignment(s_disconnected_layer, GTextAlignmentLeft); //center align
  text_layer_set_font(s_disconnected_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD)); //change the font
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_disconnected_layer));//title text
  

  // Add it as a child layer to the Window's root layer
    if (show_dis_icon==true)
  text_layer_set_text(s_disconnected_layer, "/!\\");
  
  layer_add_child(window_layer, text_layer_get_layer(s_disconnected_layer));
  layer_set_hidden( (Layer *) s_disconnected_layer,true); 
  
  

  
  //jack in annimation setup

  //set up all three layers for the jack-in annimation:
  s_teleport_back = layer_create(GRectZero);
  s_teleport_mid  = layer_create(GRectZero);
  s_teleport_front= layer_create(GRectZero);
  
  //color them
 //graphics_context_set_fill_color(s_teleport_back, GColorMidnightGreen);
  
  
  //get them ready for updates
  layer_set_update_proc(s_teleport_back, update_back);
  layer_set_update_proc(s_teleport_mid, update_mid);
  layer_set_update_proc(s_teleport_front, update_front);

  layer_add_child(window_layer, s_teleport_back);
  layer_add_child(window_layer, s_teleport_mid);
  layer_add_child(window_layer, s_teleport_front);  
}

  //handler for bluetooth
  static void bt_handler(bool connected)
  {
    // Show current connection state
    if (connected) {
      layer_set_hidden( (Layer *) s_disconnected_layer,true); 
      layer_mark_dirty(text_layer_get_layer(s_disconnected_layer));
    } else {
      layer_set_hidden( (Layer *) s_disconnected_layer,false); 
      layer_mark_dirty(text_layer_get_layer(s_disconnected_layer));
    }
}


  
  


static void update_time(){ // WHEN CALLED!!! it updates the time string
    // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char time[] = "00:00";  //'string' to hold the time

  // get the time, and put it in the string
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(time, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(time, sizeof("00:00"), "%I:%M", tick_time);
  }

  // push the string to the layer
  text_layer_set_text(s_time_layer, time);
  
  //--- DATE CODE---
  // Need to be static because they're used by the system later.
  static char s_date_text[] = "Xxxx 00";
  
  strftime(s_date_text, sizeof(s_date_text), "%b %e", tick_time);
 
  text_layer_set_text(s_date_layer, s_date_text);  
  
  //update day of week!
  static char week_day[] = "Xxx";
  strftime(week_day,sizeof(week_day),"%a",tick_time);
  text_layer_set_text(s_dow_layer,week_day);
}




/**handles animation frames.
  *is called by the second timer, every 25 seconds
  * when it's first called 'frame' should be 0
  *the first line of code advances us to frame #1
  * the 'if' statments will handle the details for each frame
  * the last one is our stop point.
  * Frame 1 is displayed INSTALLY, as soon as frame_counter is first called.
  * and if we have not reached the final frame (last_frame)
  * then , yes, this method WILL END! but it(frame_counter) will be called again in 'delay'(100) milliseconds
  * when it's called a secod time (after the delay) 'frame' will be increased to 2! and this process will repeat (with the same delay)
  * until we reach 'last_frame', that frame will be shown, but we WILL NOT be setting up a new timer after that 
**/
void frame_counter(void * data){  
frame++;
int last_frame = 10; //unnessasary? yes. but it will help me remember what the last if statment is for
int delay = 80; //delay between frames
  
  if (frame == 1){
   
    bitmap_layer_set_bitmap(s_ani_layer,  s_frame_1);
    layer_set_hidden( (Layer *)s_ani_layer,false); //hide the blink layer
    layer_mark_dirty(bitmap_layer_get_layer(s_ani_layer)); 
    }

  if (frame == 2){
      bitmap_layer_set_bitmap(s_ani_layer,  s_frame_2_6);
       layer_mark_dirty(bitmap_layer_get_layer(s_ani_layer));    
    }
  
  /** What, no frame == 3? yeah, we are emulatating frames 
  by looping through frames 3-6 without doing anything,
  we still get the delay as if we were displaying frames,
  but without doing any work!!!
  **/
  
  if (frame == 7){
    bitmap_layer_set_bitmap(s_ani_layer,  s_frame_7_8);
    layer_mark_dirty(bitmap_layer_get_layer(s_ani_layer)); 
    }
  
  
  if (frame == 9){
    bitmap_layer_set_bitmap(s_ani_layer,  s_frame_9);
    layer_mark_dirty(bitmap_layer_get_layer(s_ani_layer)); 
    }
  

  if (frame != last_frame)
    app_timer_register(delay, frame_counter, NULL);
  
    if (frame == last_frame){ //we're done! pack it up!
    layer_set_hidden( (Layer *)s_ani_layer,true); //hide the blink layer
    frame =0; // Set frame back to zero. NOTE: THIS MUST BE THE LAS LINE IN THE FRAME COUNTER, otherwise: if (frame != last_frame) will be tripped.
  }
  
}

//same as the last one, but this ones for the clock backround
void clock_bg_counter(void * data){  
background_frame++;
int c_last_frame = 11; //unnessasary? yes. but it will help me remember what the last if statment is for
int c_delay = 150; //delay between frames
  
  if (background_frame == 1){
    bitmap_layer_set_bitmap(s_clock_bg_layer,  s_clock_2);
    layer_mark_dirty(bitmap_layer_get_layer(s_clock_bg_layer)); 
    }

  if (background_frame == 2){
      bitmap_layer_set_bitmap(s_clock_bg_layer,  s_clock_3);
       layer_mark_dirty(bitmap_layer_get_layer(s_clock_bg_layer));
      layer_set_hidden( (Layer *)s_time_layer,true); //hide the time (will be unhidden when clock bg animation finishes)
    }
  
  
  if (background_frame == 3){
    bitmap_layer_set_bitmap(s_clock_bg_layer,  s_clock_4);
    layer_mark_dirty(bitmap_layer_get_layer(s_clock_bg_layer)); 
    }
  
  
  if (background_frame == 4){
    bitmap_layer_set_bitmap(s_clock_bg_layer,  s_clock_closed);
    layer_mark_dirty(bitmap_layer_get_layer(s_clock_bg_layer)); 
    }
  
  //pause for 3 frames
  
    if (background_frame == 8){
    bitmap_layer_set_bitmap(s_clock_bg_layer,  s_clock_4);
    layer_mark_dirty(bitmap_layer_get_layer(s_clock_bg_layer)); 
    }

    if (background_frame == 9){
      bitmap_layer_set_bitmap(s_clock_bg_layer,  s_clock_3);
       layer_mark_dirty(bitmap_layer_get_layer(s_clock_bg_layer));    
    }
  
  
    if (background_frame == 10){
    layer_mark_dirty(bitmap_layer_get_layer(s_clock_bg_layer)); 
    layer_set_hidden( (Layer *)s_time_layer,false); //dispaly the time again. 
    bitmap_layer_set_bitmap(s_clock_bg_layer,  s_clock_2);
    layer_mark_dirty(bitmap_layer_get_layer(s_clock_bg_layer)); 
     
    }

    if (background_frame == 11){
    bitmap_layer_set_bitmap(s_clock_bg_layer, s_clock_bg_bitmap);

    }
  
  
  
  if (background_frame != c_last_frame)
    app_timer_register(c_delay, clock_bg_counter, NULL);
  
    if (background_frame== c_last_frame){ //we're done! pack it up! 
      background_frame = 0; // Set frame back to zero. NOTE: THIS MUST BE THE LAS LINE IN THE FRAME COUNTER, otherwise: if (frame != last_frame) will be tripped.
  }
  
}


//jackin annimation
void jackin(void * data){  
jack_in_progress++;
int j_last_frame = 2; //unnessasary? yes. but it will help me remember what the last if statment is for
int j_delay = jackin_time; //delay between frames

  
  


  if (jack_in_progress != j_last_frame)
    app_timer_register(j_delay, jackin, NULL);
  
  if (jack_in_progress == j_last_frame){ //we're done! pack it up!
    layer_set_hidden( (Layer *)s_teleport_back,true);//hide jack-in screen
    layer_set_hidden( (Layer *)s_teleport_mid,true);
    layer_set_hidden( (Layer *)s_teleport_front,true);
    layer_set_hidden( (Layer *)s_white_layer,false); //show megaman
    jack_in_progress =0; // Set frame back to zero. NOTE: THIS MUST BE THE LAS LINE IN THE FRAME COUNTER, otherwise: if (frame != last_frame) will be tripped.
  }
  
}


//runs every seconds
void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  if (blink ==true)
    blink_sec ++;//time has passed since last blink
  int seconds = tick_time->tm_sec;
   int hours = tick_time ->tm_hour;
    
    if (last_hour == 24) //if at the last update the hour was 24, then we just started the watch
      last_hour=hours; //our new 'last hour' will be the current time
    
    if (hours != last_hour && vib_hour ==true) //if hour has changed (and vibrate is enabled)
     {
      vibes_short_pulse(); //vibrate (short) 
      last_hour=hours;//our new 'last hour' will be the current time
      }
  
  

  
  if(blink_sec == blink_timer&&blink ==true) 
    {
     app_timer_register(0, frame_counter, NULL);
     blink_sec=0;//yo, we just blinked, check it!
    }
   
    else if(seconds == 59)
    {
      if (animate_clock==true)
        app_timer_register(0, clock_bg_counter, NULL); //start the clock blink

      // Get the current battery level, and update the string
      battery_handler(battery_state_service_peek());
    }
  if (seconds == 00)
          //update time
      update_time();
 
}



static void main_window_unload(Window *window) { //main window closed SO KILL EVERYTHING
   // Destroy TextLayers
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_title_layer);
    text_layer_destroy(s_battery_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_dow_layer);
    text_layer_destroy(s_disconnected_layer);
  // Destroy GBitmaps
  gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(s_white_bitmap);
  gbitmap_destroy(s_black_bitmap);
  gbitmap_destroy(s_frame_1);
  gbitmap_destroy(s_frame_2_6);
  gbitmap_destroy(s_frame_7_8);
  gbitmap_destroy(s_frame_9);
  gbitmap_destroy(s_clock_bg_bitmap);
  gbitmap_destroy(s_clock_2);
  gbitmap_destroy(s_clock_3);
  gbitmap_destroy(s_clock_4);
  gbitmap_destroy(s_clock_closed);
  gbitmap_destroy(s_disconnected);
  gbitmap_destroy(s_hp_bitmap);

  
  // Destroy BitmapLayer (background)
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_white_layer);
  bitmap_layer_destroy(s_black_layer);
  bitmap_layer_destroy(s_ani_layer);
  bitmap_layer_destroy(s_clock_bg_layer);
  app_timer_cancel( (AppTimer *) frame_counter);
  app_timer_cancel((AppTimer *) clock_bg_counter);
  app_timer_cancel((AppTimer *) jackin);
  layer_destroy(s_teleport_back);
  layer_destroy(s_teleport_mid);
  layer_destroy(s_teleport_front);
  bitmap_layer_destroy(s_hp_layer);
  

}

static void init() {

 // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,  //sets the startup
    .unload = main_window_unload  //sets the close 
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  

 //  Register with TickTimerService seconds for animation
  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
  battery_state_service_subscribe((BatteryStateHandler) battery_handler);
    // Make sure the time is displayed from the start
  update_time();
  animate();
 app_timer_register(0, jackin, NULL);
  
    // Subscribe to Bluetooth updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bt_handler
  });

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