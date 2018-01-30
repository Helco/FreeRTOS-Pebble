/* action_bar_layer.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Carson Katri <me@carsonkatri.com>
 */

#include "librebble.h"
#include "utils.h"
#include "status_bar_layer.h"
#include "bitmap_layer.h"
#include "graphics.h"
#include "platform.h"

static struct tm s_last_time;

static void status_tick(struct tm *tick_time, TimeUnits tick_units)
{
    memcpy(&s_last_time, tick_time, sizeof(struct tm));
}

StatusBarLayer *status_bar_layer_create(void)
{
    StatusBarLayer *status_bar = (StatusBarLayer*)app_calloc(1, sizeof(StatusBarLayer));
    
    GRect frame = GRect(0, 0, DISPLAY_COLS, STATUS_BAR_LAYER_HEIGHT);
    layer_ctor(&status_bar->layer, frame);

    status_bar->background_color = GColorDarkGray;
    status_bar->foreground_color = GColorWhite;
    status_bar->separator_mode = StatusBarLayerSeparatorModeDotted;
    
    layer_set_update_proc(&status_bar->layer, draw);
    // TODO: We should have a better concept than to use the App's handler slot, then put s_last_time into struct
    tick_timer_service_subscribe(MINUTE_UNIT, status_tick);
    layer_mark_dirty(&status_bar->layer);

    memcpy(&s_last_time, rebble_time_get_tm(), sizeof(struct tm));
    
    return status_bar;
}

void status_bar_layer_destroy(StatusBarLayer *status_bar)
{
    layer_destroy(&status_bar->layer);
}

Layer *status_bar_layer_get_layer(StatusBarLayer *status_bar)
{
    return &status_bar->layer;
}

GColor status_bar_layer_get_background_color(StatusBarLayer *status_bar)
{
    return status_bar->background_color;
}

GColor status_bar_layer_get_foreground_color(StatusBarLayer * status_bar)
{
    return status_bar->foreground_color;
}

void status_bar_layer_set_colors(StatusBarLayer * status_bar, GColor background, GColor foreground)
{
    if (status_bar->background_color.argb != background.argb || status_bar->foreground_color.argb != foreground.argb) {
        status_bar->background_color = background;
        status_bar->foreground_color = foreground;
        
        layer_mark_dirty(&status_bar->layer);
    }
}

void status_bar_layer_set_separator_mode(StatusBarLayer * status_bar, StatusBarLayerSeparatorMode mode)
{
    if (status_bar->separator_mode != mode) {
        status_bar->separator_mode = mode;

        layer_mark_dirty(&status_bar->layer);
    }
}

static void draw(Layer *layer, GContext *context)
{
    StatusBarLayer *status_bar = (StatusBarLayer *) layer;
    GRect full_bounds = layer_get_bounds(layer);
    
    // Draw the background
    graphics_context_set_fill_color(context, status_bar->background_color);
    graphics_fill_rect(context, full_bounds, 0, GCornerNone);
    
    // Draw the time
    graphics_context_set_text_color(context, status_bar->foreground_color);
    GFont time_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    
    char time_string[8] = "";
    rcore_strftime(time_string, 8, "%R", &s_last_time);
    printf("%s", time_string);
    
    GRect text_bounds = full_bounds;
    text_bounds.origin.y = -2;
    graphics_draw_text_app(context, time_string, time_font, text_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, 0);
    
    // TODO: Draw the separator
    if (status_bar->separator_mode == StatusBarLayerSeparatorModeDotted)
    {
        
    }
}
