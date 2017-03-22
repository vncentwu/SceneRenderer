#ifndef CubeMapChooser_h
#define CubeMapChooser_h

#include <FL/Fl.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <string>
#include "../scene/cubeMap.h"

class TextureMap;
class GraphicalUI;



class CubeMapChooser {

	GraphicalUI* caller;
	Fl_Menu_Window* w;
	Fl_Button* ok;
	Fl_Button* cancel;
	Fl_File_Input* fi[6];
	Fl_Light_Button* fb[6];
	TextureMap* cubeFace[6];
	std::string fn[6];
	std::string btnMsg[6];



	typedef void (*fptrarray) (Fl_Widget*, void*);
	
	static void cb_ok(Fl_Widget* o, void* v);
	static void cb_cancel(Fl_Widget* o, void* v);
	static void cb_ffi (Fl_Widget* o, int i);
	static void cb_ffb (Fl_Widget* o, int i);
	static void cb_xpi (Fl_Widget* o, void* v);
	static void cb_xni (Fl_Widget* o, void* v);
	static void cb_ypi (Fl_Widget* o, void* v);
	static void cb_yni (Fl_Widget* o, void* v);
	static void cb_zpi (Fl_Widget* o, void* v);
	static void cb_zni (Fl_Widget* o, void* v);
	static void cb_xpb (Fl_Widget* o, void* v);
	static void cb_xnb (Fl_Widget* o, void* v);
	static void cb_ypb (Fl_Widget* o, void* v);
	static void cb_ynb (Fl_Widget* o, void* v);
	static void cb_zpb (Fl_Widget* o, void* v);
	static void cb_znb (Fl_Widget* o, void* v);
	fptrarray cb_fi[6];
	fptrarray cb_fb[6];

public:
  CubeMapChooser();
  void setCaller(GraphicalUI* c) { caller = c; }
  void show();
  void hide();

};
#endif
