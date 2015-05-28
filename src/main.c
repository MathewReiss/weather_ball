#include <pebble.h>

Window *my_window;
Layer *text_layer;

GBitmap *digits[4];
const int DIGIT_IDS[] = {
	RESOURCE_ID_RUBBER_0,
	RESOURCE_ID_RUBBER_1,
	RESOURCE_ID_RUBBER_2,
	RESOURCE_ID_RUBBER_3,
	RESOURCE_ID_RUBBER_4,
	RESOURCE_ID_RUBBER_5,
	RESOURCE_ID_RUBBER_6,
	RESOURCE_ID_RUBBER_7,
	RESOURCE_ID_RUBBER_8,
	RESOURCE_ID_RUBBER_9
};

#define SUNNY 0
#define RAIN 1
#define THUNDERSTORM 2
#define PARTLY_CLOUDY 3
#define MOSTLY_CLOUDY 4
#define SNOW 5
#define FOG 6
#define BLIZZARD 7

int CURRENT_WEATHER = 7;

#ifdef PBL_COLOR
	
void replace_gbitmap_color(GColor color_to_replace, GColor replace_with_color, GBitmap *im){
	GColor *current_palette = gbitmap_get_palette(im);

	for(int i = 0; i < 2; i++){
		if ((color_to_replace.argb & 0x3F)==(current_palette[i].argb & 0x3F)){
			current_palette[i].argb = (current_palette[i].argb & 0xC0)| (replace_with_color.argb & 0x3F);
		}
	}
}

//2 Shades of White (Light Gray + White), 2 Shades of Blue (Vivid Cerulean + Picton Blue), 2 Shades of Yellow (Icterine + Yellow)
void set_weather(){	
	switch(CURRENT_WEATHER){
		case SUNNY: //Sun, clear ground
			replace_gbitmap_color(GColorWhite, GColorYellow, digits[0]);
			replace_gbitmap_color(GColorWhite, GColorVividCerulean, digits[1]);
			replace_gbitmap_color(GColorWhite, GColorVividCerulean, digits[2]);
			replace_gbitmap_color(GColorWhite, GColorVividCerulean, digits[3]);
			break;
		
		case RAIN: //Gray clouds, rain
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[0]);
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[1]);
			replace_gbitmap_color(GColorWhite, GColorPictonBlue, digits[2]);
			replace_gbitmap_color(GColorWhite, GColorPictonBlue, digits[3]);
			break;
		
		case THUNDERSTORM: //Gray clouds, rain + lightning
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[0]);
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[1]);
			replace_gbitmap_color(GColorWhite, GColorPictonBlue, digits[2]);
			replace_gbitmap_color(GColorWhite, GColorYellow, digits[3]);
			break;
		
		case PARTLY_CLOUDY: //Sun + clouds, clear ground
			replace_gbitmap_color(GColorWhite, GColorYellow, digits[0]);
			replace_gbitmap_color(GColorWhite, GColorVividCerulean, digits[2]);
			replace_gbitmap_color(GColorWhite, GColorVividCerulean, digits[3]);
			break;
		
		case MOSTLY_CLOUDY: //Gray clouds, clear ground
			replace_gbitmap_color(GColorWhite, GColorYellow, digits[0]);
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[1]);
			replace_gbitmap_color(GColorWhite, GColorVividCerulean, digits[2]);
			replace_gbitmap_color(GColorWhite, GColorVividCerulean, digits[3]);
			break;
		
		case SNOW: //Gray clouds, white snow
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[0]);
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[1]);
			break;
		
		case FOG: //All gray
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[0]);
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[1]);
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[2]);
			replace_gbitmap_color(GColorWhite, GColorLightGray, digits[3]);
			break;
		
		case BLIZZARD: //All white
			break;
	}
}

static void send_request(){
	Tuplet request_tuple = TupletCString(0, "0");
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	dict_write_tuplet(iter, &request_tuple);
	dict_write_end(iter);
	app_message_outbox_send();
}

static void in_received_handler(DictionaryIterator *iter, void *context){
	Tuple *t = dict_read_first(iter);
	CURRENT_WEATHER = atoi(t->value->cstring);
	set_weather();
	layer_mark_dirty(text_layer);
}

#endif	

static void draw_text(Layer *layer, GContext *ctx){
	graphics_draw_bitmap_in_rect(ctx, digits[0], GRect(0,0,72,84));
	graphics_draw_bitmap_in_rect(ctx, digits[1], GRect(72,0,72,84));
	graphics_draw_bitmap_in_rect(ctx, digits[2], GRect(0,84,72,84));
	graphics_draw_bitmap_in_rect(ctx, digits[3], GRect(72,84,72,84));
}

void clear_bitmaps(){
	gbitmap_destroy(digits[0]);
	gbitmap_destroy(digits[1]);
	gbitmap_destroy(digits[2]);
	gbitmap_destroy(digits[3]);	
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units){
	int display_hour = tick_time->tm_hour;
	if(!clock_is_24h_style() && (display_hour == 0 || display_hour > 12)){
		display_hour = display_hour % 12;
		display_hour = display_hour ? display_hour : 12;
	}
	
	clear_bitmaps();
	
	digits[0] = gbitmap_create_with_resource(DIGIT_IDS[display_hour/10]);
	digits[1] = gbitmap_create_with_resource(DIGIT_IDS[display_hour%10]);
	digits[2] = gbitmap_create_with_resource(DIGIT_IDS[tick_time->tm_min/10]);
	digits[3] = gbitmap_create_with_resource(DIGIT_IDS[tick_time->tm_min%10]);
		
	#ifdef PBL_COLOR
		//CURRENT_WEATHER = tick_time->tm_sec%7;
		set_weather();
		if((tick_time->tm_min+1)%15 == 0) send_request();
	#endif
		
	layer_mark_dirty(text_layer);
}

void handle_init(void) {
	#ifdef PBL_COLOR
		app_message_open(32,32);
	    app_message_register_inbox_received(in_received_handler);			
	#endif
	
	my_window = window_create();
	
	text_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(text_layer, draw_text);
	
	digits[0] = gbitmap_create_with_resource(RESOURCE_ID_RUBBER_0);
	digits[1] = gbitmap_create_with_resource(RESOURCE_ID_RUBBER_0);
	digits[2] = gbitmap_create_with_resource(RESOURCE_ID_RUBBER_0);
	digits[3] = gbitmap_create_with_resource(RESOURCE_ID_RUBBER_0);
	
	layer_add_child(window_get_root_layer(my_window), text_layer);
 	window_stack_push(my_window, true);
	
	time_t now = time(NULL);
	struct tm *tick_time = localtime(&now);
	handle_minute_tick(tick_time, MINUTE_UNIT);
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	
	#ifdef PBL_COLOR
		app_timer_register(500, send_request, NULL);
	#endif
}

void handle_deinit(void) {
	layer_destroy(text_layer);
	window_destroy(my_window);
	
	clear_bitmaps();
	
	tick_timer_service_unsubscribe();
	
	#ifdef PBL_COLOR
		app_message_deregister_callbacks();
	#endif
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}