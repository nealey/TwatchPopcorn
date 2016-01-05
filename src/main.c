#include <pebble.h>


#define MINLEN 58
#define HRCIRCLER 30
#define NRINGS 17
#define MINCIRCLER 20

#define fatness 14
#define fat(x) (fatness * x)

static Window *window;
static Layer *s_hr_layer, *s_min_layer, *s_sec_layer;

GRect display_bounds;
GPoint center, mincenter;

int32_t min_angle, sec_angle;
char hrstr[3], minstr[3];
bool min_even;
bool show_seconds = false;
bool bt_vibe = true;
bool shape = 0;

bool bt_connected;

#define nringcolors 5
GColor *rings[nringcolors] = {
  &GColorVividCerulean,
  &GColorChromeYellow,
  &GColorCobaltBlue,
  &GColorDarkGray,
  &GColorWhite,
};

const GPoint SUN_POINTS[] = {
  {0, 119},{78, 162},{93, 74},{175, 40},{116, -26},{140, -112},{52, -108},{0, -180},{-52, -108},{-140, -112},{-116, -26},{-175, 40},{-93, 74},{-78, 162},{0}
};

// r = 180; tau = math.pi*2
// print ','.join(["{%s, %s}"%(int(r/(i%2+1)*sin(tau*(i/10.0))), int(r/(i%2+1)*cos(tau*(i/10.0)))) for i in range(10)])
const GPoint STAR_POINTS[] = {
  {0, 180},{52, 72},{171, 55},{85, -27},{105, -145},{0, -90},{-105, -145},{-85, -27},{-171, 55},{-52, 72},{0}
};

const GPoint SQUARE_POINTS[] = {
  {0, 180},{180, 0},{0, -180},{-180, 0},{0}
};

// print ','.join(["{%s, %s}"%(int(r*sin(tau*(i/3.0))), int(r*cos(tau*(i/3.0)))) for i in range(3)])
const GPoint TRIANGLE_POINTS[] = {
  {0, 180},{155, -90},{-155, -90},{0}
};

GPath shape_path = {
  0, 
  (GPoint [15]){{0}},
  0,
  {0}
};

static void min_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, *rings[0]);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  
  gpath_move_to(&shape_path, mincenter);
  gpath_rotate_to(&shape_path, min_angle + DEG_TO_TRIGANGLE(180));
  
  for (int i = 0; i < NRINGS; i += 1) {
   graphics_context_set_fill_color(ctx, *rings[(NRINGS - i) % nringcolors]);
    if (shape) {
      for (unsigned int j = 0; ; j += 1) {
        GPoint p;
      
        p = SUN_POINTS[j];
      
        if ((p.x == 0) && (p.y == 0)) {
          shape_path.num_points = j;
          break;
        }
        shape_path.points[j].x = p.x * (NRINGS - i) / 10;
        shape_path.points[j].y = p.y * (NRINGS - i) / 10;
      }
      gpath_draw_filled(ctx, &shape_path);
    } else {
      graphics_fill_circle(ctx, mincenter, MINCIRCLER + (NRINGS - i) * fatness);
    }
  }
  
  // center dot
  if (bt_connected) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_text_color(ctx, GColorWhite);
  } else {
    graphics_context_set_fill_color(ctx, GColorYellow);
    graphics_context_set_text_color(ctx, GColorBlack);
  }
  graphics_fill_circle(ctx, mincenter, MINCIRCLER);
  
  GRect textbox = {
    .origin = {
      .x = mincenter.x - 20,
      .y = mincenter.y - 16,
    },
    .size = {
      .w = 40,
      .h = 40,
    }
  };
  graphics_draw_text(ctx, minstr,
                     fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                     textbox, 
                     GTextOverflowModeWordWrap,
                     GTextAlignmentCenter,
                     NULL);
}

static void hr_update_proc(Layer *layer, GContext *ctx) {
  // Draw a black circle
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, HRCIRCLER);
  
  // Write the current hour text in it
  GRect textbox = {
    .origin = {
      .x = center.x - HRCIRCLER,
      .y = center.y - HRCIRCLER + 4,
    },
    .size = {
      .w = HRCIRCLER * 2,
      .h = HRCIRCLER,
    }
  };
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, (*hrstr=='0')?(hrstr+1):(hrstr),
                     fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),
                     textbox, 
                     GTextOverflowModeWordWrap,
                     GTextAlignmentCenter,
                     NULL);
}

static void sec_update_proc(Layer *layer, GContext *ctx) {
  if (! show_seconds) {
    return;
  }
  
  GRect secbox = {
    .origin = {
      .x = mincenter.x - (40+2)/2,
      .y = mincenter.y - (40+2)/2,
    },
    .size = {
      .w = 40+2,
      .h = 40+2,
    }
  };
  
  graphics_context_set_fill_color(ctx, GColorVividCerulean);
  if (min_even) {
    graphics_fill_radial(ctx, secbox, GOvalScaleModeFitCircle, 2, 0, sec_angle);
  } else {
    graphics_fill_radial(ctx, secbox, GOvalScaleModeFitCircle, 2, sec_angle, DEG_TO_TRIGANGLE(360));
  }
}


static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & HOUR_UNIT) {
    if (clock_is_24h_style()) {
      strftime(hrstr, sizeof(hrstr), "%H", tick_time);
    } else {
      strftime(hrstr, sizeof(hrstr), "%I", tick_time);
    }
    layer_mark_dirty(s_hr_layer);
  }
  
  if (units_changed & MINUTE_UNIT) {
    strftime(minstr, sizeof(minstr), "%M", tick_time);
    min_angle = DEG_TO_TRIGANGLE(tick_time->tm_min * 6);
    mincenter.x = (int16_t)(sin_lookup(min_angle) * MINLEN / TRIG_MAX_RATIO) + center.x;
    mincenter.y = (int16_t)(-cos_lookup(min_angle) * MINLEN / TRIG_MAX_RATIO) + center.y;
    min_even = (tick_time->tm_min % 2 == 0);
  
    layer_mark_dirty(s_min_layer);
  }

  if (units_changed & SECOND_UNIT) {
    sec_angle = DEG_TO_TRIGANGLE(tick_time->tm_sec * 6);
    layer_mark_dirty(s_sec_layer);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  display_bounds = layer_get_bounds(window_layer);
  center = grect_center_point(&display_bounds);
  
  // Minutes
  s_min_layer = layer_create(display_bounds);
  layer_set_update_proc(s_min_layer, min_update_proc);
  
  // Seconds
  s_sec_layer = layer_create(display_bounds);
  layer_set_update_proc(s_sec_layer, sec_update_proc);
  
  // Hours
  s_hr_layer = layer_create(display_bounds); // XXX: Perhaps this is too big.
  layer_set_update_proc(s_hr_layer, hr_update_proc);
  
  layer_add_child(window_layer, s_min_layer);
  layer_add_child(window_layer, s_hr_layer);
  layer_add_child(window_layer, s_sec_layer);
  
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  if (show_seconds) {
    handle_tick(tick_time, HOUR_UNIT | MINUTE_UNIT | SECOND_UNIT);
  } else {
    handle_tick(tick_time, HOUR_UNIT | MINUTE_UNIT);
  }
}

static void window_unload(Window *window) {
  layer_destroy(s_hr_layer);
  layer_destroy(s_min_layer);
  layer_destroy(s_sec_layer);
}

static void bt_handler(bool connected) {
  bt_connected = connected;
  if (bt_vibe && (! connected)) {
    vibes_double_pulse();
  }
  layer_mark_dirty(s_min_layer);
}

static void init() {  
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  bluetooth_connection_service_subscribe(bt_handler);
  bt_connected = bluetooth_connection_service_peek();
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
