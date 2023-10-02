#include <stdio.h>
#include "easygl/graphics.h"
#include "spdlog/spdlog.h"

// Callbacks for event-driven window handling.
void ui_drawscreen();
void ui_click_handler (float x, float y);
void ui_mouse_handler (float x, float y);
void ui_key_handler(char c);


void ui_init() {
    spdlog::info("Init UI");
    init_graphics("A1", WHITE);
    init_world(0.,0.,1000.,1000.);
    event_loop(ui_click_handler, ui_mouse_handler, ui_key_handler, ui_drawscreen);   
}

void ui_teardown() {
    close_graphics ();
}

void ui_drawscreen() {
	set_draw_mode (DRAW_NORMAL);  // Should set this if your program does any XOR drawing in callbacks.
	clearscreen();  /* Should precede drawing for all drawscreens */
	clearscreen();
	setcolor (RED);
	setlinewidth(1);
	setlinestyle (DASHED);
	drawrect (350.,550.,650.,670.); 
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
