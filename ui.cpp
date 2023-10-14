#include <stdio.h>
#include <string>
#include "easygl/graphics.h"
#include "spdlog/spdlog.h"
#include "circuit.h"
#include "ui.h"
#include <condition_variable>
using namespace std;

// Callbacks for event-driven window handling.
void ui_drawscreen();
void ui_click_handler (float x, float y);
void ui_mouse_handler (float x, float y);
void ui_key_handler(char c);

void ui_draw_switch_conns(logic_block* lb, float x0, float y0, float x1, float y1, float length);

float logic_block_width = 10.0;

circuit* circ;


void ui_pump(void (*draw)()) {
    circuit_next_step();
    draw();
}

void ui_init(circuit* circuit) {
    circ = circuit;
    spdlog::info("Init UI");
    init_graphics("A1", BLACK);
    create_button("Proceed","PUMP", ui_pump);
    //init_world(-50.,-50.,150.,150.);
    init_world(-50.,150.,150.,-50.);
    //set_keypress_input(true);
    //set_mouse_move_input(true);
    event_loop(ui_click_handler, ui_mouse_handler, ui_key_handler, ui_drawscreen);   
}

void ps_output(circuit* circuit, string file) {
    init_world(0., 0., 100., 100.);
    if (init_postscript(file.c_str())) {
        spdlog::info("Writing postscript to {}", file);
        ui_draw(circuit);
        close_postscript_noui();
        spdlog::info("Finished writing postscript to {}", file);
    } else {
        spdlog::error("could not write postscript to {}", file);
    }
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

void ui_draw_lb_conns(logic_block* lb) {
    setcolor(lb->layer == 0 ? GREEN : YELLOW);
    setlinestyle(SOLID);
	setlinewidth(1);
    float x0, y0, x1, y1;
    x0 = (2*lb->x)*(logic_block_width*1.25);
    y0 = (2*lb->y)*(logic_block_width*1.25);
    x0 += float(lb->layer)*logic_block_width*0.125;
    y0 -= float(lb->layer)*logic_block_width*0.25;
    x1 = x0 + logic_block_width;
    y1 = y0 + logic_block_width;


    for(int i = 0; i < lb->tracks_per_channel; ++i) {

        if (i < lb->north_conns->size() && (*lb->north_conns)[i] != UNUSED) {
            float xcen = x0 + logic_block_width*0.25;
            float ycen = y0 - logic_block_width*0.25 - float(i+0.5)*logic_block_width/float(lb->north_conns->size());
            fillarc(xcen, ycen, 0.5, 0.0, 360.0);
        }

        if (i < lb->south_conns->size() && (*lb->south_conns)[i] != UNUSED) {
            float xcen = x1 - logic_block_width*0.25;
            float ycen = y1 + logic_block_width*0.25 + float(i+0.5)*logic_block_width/float(lb->south_conns->size());
            fillarc(xcen, ycen, 0.5, 0.0, 360.0);
        }

        if (i < lb->east_conns->size() && (*lb->east_conns)[i] != UNUSED) {
            float ycen = y1 - logic_block_width*0.25;
            float xcen = x1 + logic_block_width*0.25 + float(i+0.5)*logic_block_width/float(lb->east_conns->size());
            fillarc(xcen, ycen, 0.5, 0.0, 360.0);
        }
        if (i < lb->west_conns->size() && (*lb->west_conns)[i] != UNUSED) {
            float ycen = y0 + logic_block_width*0.25;
            float xcen = x0 - logic_block_width*0.25 - float(i+0.5)*logic_block_width/float(lb->west_conns->size());
            fillarc(xcen, ycen, 0.5, 0.0, 360.0);
        }
    }
}
void ui_draw(logic_block* lb) {
    setcolor(lb->layer == 0 ? GREEN : YELLOW);
    setlinestyle(SOLID);
	setlinewidth(1);
    float x0, y0, x1, y1;
    x0 = (2*lb->x)*(logic_block_width*1.25);
    y0 = (2*lb->y)*(logic_block_width*1.25);
    x0 += float(lb->layer)*logic_block_width*0.125;
    y0 -= float(lb->layer)*logic_block_width*0.25;
    x1 = x0 + logic_block_width;
    y1 = y0 + logic_block_width;

    drawrect(x0, y0, x1, y1);
    char label[32] = {'\0'};
    snprintf(label,32,"(%d,%d)",lb->x,lb->y);
    drawtext((x0+x1)/2.0, (y0+y1)/2.0, label,10.0);
    
    drawline( x1 - logic_block_width*0.25, y1, x1 - logic_block_width*0.25, y1+logic_block_width*1.25 );
    drawline( x0 + logic_block_width*0.25, y0, x0 + logic_block_width*0.25, y0-logic_block_width*1.25 );

    drawline( x1 , y1 - logic_block_width*0.25, x1 + logic_block_width*1.25, y1 - logic_block_width*0.25);
    drawline( x0 , y0 + logic_block_width*0.25, x0 - logic_block_width*1.25, y0 + logic_block_width*0.25);

}

enum ui_draw_conn_mode {
    SWITCH_CONNS,
    TRACK_SEGMENTS_EMPTY,
    TRACK_SEGMENTS_USED
};

void ui_draw_switch_conns(switch_block* sb, enum ui_draw_conn_mode mode, float x0, float y0, float x1, float y1, float length) {
    
    float dx = (x1-x0)/float(sb->tracks_per_channel);
    float dy = (y1-y0)/float(sb->tracks_per_channel);

    float xoff = dx/2.0;
    float yoff = dy/2.0;

    for(int i = 0; i < sb->tracks_per_channel; ++i) {
        if (mode == TRACK_SEGMENTS_EMPTY) {
            drawline(xoff + x0 + i*dx, y0, xoff + x0 + i*dx, y0 - length);
            drawline(xoff + x0 + i*dx, y1, xoff + x0 + i*dx, y1 + length);
            drawline(x1, yoff + y0 + i*dy, x1 + length, yoff + y0 + i*dy);
            drawline(x0, yoff + y0 + i*dy, x0 - length, yoff + y0 + i*dy);
        } else if (mode == TRACK_SEGMENTS_USED) {
            if (sb->segment_used(SOUTH, i)) {
                drawline(xoff + x0 + i*dx, y0, xoff + x0 + i*dx, y0 - length);
            }
            if (sb->segment_used(NORTH, i)) {
                drawline(xoff + x0 + i*dx, y1, xoff + x0 + i*dx, y1 + length);
            }
            if (sb->segment_used(EAST, i)) {
                drawline(x1, yoff + y0 + i*dy, x1 + length, yoff + y0 + i*dy);
            }
            if (sb->segment_used(WEST, i)) {
                drawline(x0, yoff + y0 + i*dy, x0 - length, yoff + y0 + i*dy);
            }
        } else if (mode == SWITCH_CONNS) {
            for(int src_int=0; src_int != N_DIRECTIONS; ++src_int) {
                enum direction src = static_cast<direction>(src_int);    
                for(int dst_int=src+1; dst_int != N_DIRECTIONS; ++dst_int) {
                    enum direction dst = static_cast<direction>(dst_int);    
                    if ((dst != src) && sb->direct_connected(src,dst,i)) {
                        float srcx = (src == EAST ? x1 : x0);
                        float srcy = (src == NORTH ? y1 : y0);
                        float dstx = (dst == EAST ? x1 : x0);
                        float dsty = (dst == NORTH ? y1 : y0);

                        if (src == SOUTH || src == NORTH)
                            srcx += i*dx + xoff;
                        else if (src == WEST || src == EAST)
                            srcy += i*dy + yoff;

                        if (dst == SOUTH || dst == NORTH)
                            dstx += i*dx + xoff;
                        else if (dst == WEST || dst == EAST)
                            dsty += i*dy + yoff;

                        drawline(srcx,srcy,dstx,dsty);
                    }
                }
            }
        }
    }
}


void ui_draw(switch_block* sb) {
    setcolor(WHITE);
    setlinestyle(SOLID);
	setlinewidth(1);
    float x0, y0, x1, y1;
    x0 = (2*sb->x-1)*(logic_block_width*1.25);
    y0 = (2*sb->y-1)*(logic_block_width*1.25);
    x1 = x0 + logic_block_width;
    y1 = y0 + logic_block_width;
    drawrect(x0, y0, x1, y1);

    // draw the grid connections
    setlinestyle (DASHED);
    //setcolor(LIGHTGREY);
    //ui_draw_switch_conns(sb, TRACK_SEGMENTS_EMPTY, x0, y0, x1, y1, logic_block_width);
    // draw the track segments into the switchblock
    setcolor(WHITE);
    ui_draw_switch_conns(sb, TRACK_SEGMENTS_EMPTY, x0, y0, x1, y1, logic_block_width*0.25);
    // draw internal switch connections

    setcolor(RED);
    setlinestyle(SOLID);
    setlinewidth(4);
    ui_draw_switch_conns(sb, SWITCH_CONNS, x0, y0, x1, y1, .0);
    ui_draw_switch_conns(sb, TRACK_SEGMENTS_USED, x0, y0, x1, y1, logic_block_width);
}

void setline_label(int val){
    switch (val) {
        case 0:     // the initial bit
            setlinewidth(4);
            setlinestyle (SOLID);
            setcolor(BLUE);
            break;
        case TARGET:
            setlinewidth(4);
            setlinestyle (SOLID);
            setcolor(YELLOW);
            break;
        case UNUSED:
            setlinewidth(1);
            setlinestyle (DASHED);
            setcolor(LIGHTGREY);
            break;
        case USED:
            setlinewidth(4);
            setlinestyle (SOLID);
            setcolor(MAGENTA);
            break;
        default:
            setlinewidth(4);
            setlinestyle (SOLID);
            setcolor(RED);
            break;
    }
}

void ui_draw_h_segment(circuit* circ, int x, int y) {
    float dy = logic_block_width / float(circ->tracks_per_channel);
    float x0 = (2*x)*(logic_block_width*1.25);
    float y0 = (2*y-1)*(logic_block_width*1.25);
    float x1 = x0 + logic_block_width;
    for(int track = 0; track < circ->tracks_per_channel; ++track) {
        int val = circ->get_h_segment(x,y,track);
        setline_label(val);
        drawline(x0,y0 +track*dy + dy/2, x1, y0 + track*dy + dy/2);
        char label[32] = {'\0'};
        if (val == TARGET)
            label[0]='T';
        else if (val == USED)
            label[0]='U';
        else if (val == SOURCE)
            label[0]='S';
        else if (val == UNUSED)
            continue;
        else
            snprintf(label,32,"%d",val);
        drawtext((x0+x1)/2, y0 +track*dy, label,10.0);
    }
}

void ui_draw_v_segment(circuit* circ, int x, int y) {
    float dx = logic_block_width / float(circ->tracks_per_channel);
    float x0 = (2*x-1)*(logic_block_width*1.25);
    float y0 = (2*y)*(logic_block_width*1.25);
    float y1 = y0 + logic_block_width;
    for(int track = 0; track < circ->tracks_per_channel; ++track) {
        int val = circ->get_v_segment(x,y,track);
        setline_label(val);
        drawline(x0 + dx/2 + track*dx, y0, x0 + dx/2 + track*dx, y1);
        char label[32] = {'\0'};
        label[0]='T';
        if (val == TARGET)
            label[0]='T';
        else if (val == USED)
            label[0]='U';
        else if (val == SOURCE)
            label[0]='S';
        else if (val == UNUSED)
            continue;
        else
            snprintf(label,32,"%d",val);
        drawtext(x0 + track*dx, (y0 + y1)/2, label,10.0);
    }
}

void ui_draw_segments(circuit* circ) {
    for(int i = 0; i < circ->h_segs.size(); ++i) {
        int x = i%(circ->grid_size);
        int y = i/(circ->grid_size);
        ui_draw_h_segment(circ,x,y);
    }

    for(int i = 0; i < circ->v_segs.size(); ++i) {
        int x = i%(circ->grid_size+1);
        int y = i/(circ->grid_size+1);
        ui_draw_v_segment(circ,x,y);
    }
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
    ui_draw_segments(circ);

    for(auto lb : circ->logic_blocks) {
        ui_draw_lb_conns(lb);
    }
}
