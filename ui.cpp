#include <stdio.h>
#include "easygl/graphics.h"
#include "spdlog/spdlog.h"
#include "circuit.h"
#include "ui.h"

// Callbacks for event-driven window handling.
void ui_drawscreen();
void ui_click_handler (float x, float y);
void ui_mouse_handler (float x, float y);
void ui_key_handler(char c);

float gap = 10.0;

circuit* circ;


void ui_init(circuit* circuit) {
    circ = circuit;
    spdlog::info("Init UI");
    init_graphics("A1", BLACK);
    init_world(0.,0.,1000.,1000.);
    //set_keypress_input(true);
    //set_mouse_move_input(true);
    event_loop(ui_click_handler, ui_mouse_handler, ui_key_handler, ui_drawscreen);   
}

void ui_teardown() {
    close_graphics ();
}

void ui_drawscreen() {
    clearscreen();
	set_draw_mode (DRAW_NORMAL);  // Should set this if your program does any XOR drawing in callbacks.
    ui_draw(circ);
}

void ui_click_handler (float x, float y) {
	spdlog::debug("user clicked at {},{}",x,y);
}

void ui_mouse_handler (float x, float y) {
	spdlog::debug("mouse move {},{}",x,y);
}

void ui_key_handler(char c) {
	spdlog::debug("keypress {}",c);
}

void ui_draw(switch_block* sb) {
    setcolor(GREEN);
	setlinewidth(1);
    float x0, y0, x1, y1;
    x0 = sb->x*10. + gap;
    y0 = sb->y*10. + gap;
    x1 = (sb->x+1)*10. + gap;
    y1 = (sb->y+1)*10. + gap;
    drawrect(x0, y0, x1, y1);
}
void ui_draw(logic_block* lb) {
}
void ui_draw(connection* conn) {
}

void ui_draw(circuit* circ) {
    for(auto sb : circ->switch_blocks) {
        ui_draw(sb);
    }
}
