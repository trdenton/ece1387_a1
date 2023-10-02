#ifndef __UI_H__
#define __UI_H__
#include "circuit.h"
#include <string>
using namespace std;
void ui_init(circuit*);
void ui_teardown();

//drawing functions
void ui_draw(switch_block*);
void ui_draw(logic_block*);
void ui_draw(connection*);
void ui_draw(circuit*);
void ps_output(circuit* circuit, string file);
#endif
