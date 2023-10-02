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

float logic_block_width = 10.0;

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
    setcolor(WHITE);
	setlinewidth(1);
    float x0, y0, x1, y1;
    x0 = (2*sb->x+1)*(logic_block_width*1.25);
    y0 = (2*sb->y+1)*(logic_block_width*1.25);
    x1 = x0 + logic_block_width;
    y1 = y0 + logic_block_width;
    drawrect(x0, y0, x1, y1);
}
void ui_draw(logic_block* lb) {
    setcolor(GREEN);
	setlinewidth(1);
    float x0, y0, x1, y1;
    x0 = (2*lb->x)*(logic_block_width*1.25);
    y0 = (2*lb->y)*(logic_block_width*1.25);
    x1 = x0 + logic_block_width;
    y1 = y0 + logic_block_width;
    drawrect(x0, y0, x1, y1);
}
void ui_draw(connection* conn) {
}

void ui_draw(circuit* circ) {
    for(auto lb : circ->logic_blocks) {
        ui_draw(lb);
    }
    for(auto sb : circ->switch_blocks) {
        ui_draw(sb);
    }
}
