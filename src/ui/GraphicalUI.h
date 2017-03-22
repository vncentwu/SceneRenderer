//
// GraphicalUI.h
//
// The header file for the graphical UI
//

#ifndef __GraphicalUI_h__
#define __GraphicalUI_h__

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Float_Input.H>

#include <FL/Fl_File_Chooser.H>		// FLTK file chooser

#include "TraceUI.h"
#include "TraceGLWindow.h"
#include "debuggingWindow.h"
#include "CubeMapChooser.h"




class ModelerView;

class GraphicalUI : public TraceUI {
public:
	GraphicalUI();

	int run();

	void		alert( const string& msg );

	// The FLTK widgets
	Fl_Window*			m_mainWindow;
	Fl_Menu_Bar*		m_menubar;
    Fl_Box*           m_lineLabel;

	Fl_Slider*			m_sizeSlider;
	Fl_Slider*			m_depthSlider;
    Fl_Slider*			m_sampleSizeSlider;
    Fl_Slider*			m_filterSlider;
    Fl_Slider*          cubeSlider;

	Fl_Check_Button*	m_debuggingDisplayCheckButton;
    Fl_Check_Button*    m_surfaceHeuristicButton;
    Fl_Check_Button*    m_adaptiveSamplingButton;
    Fl_Check_Button*	m_cubeMapCheckButton;

    Fl_Group*           m_heuristicGroup;

	Fl_Button*			m_renderButton;
	Fl_Button*			m_stopButton;
    Fl_Round_Button*	m_jitterSamplingButton;
    Fl_Round_Button*	m_uniformSamplingButton;
    Fl_Check_Button*    m_accelerateCheckButton;

    Fl_Float_Input*     m_depthDenominator;
    Fl_Float_Input*     m_angleDenominatorA;
    Fl_Float_Input*     m_angleDenominatorB;
    Fl_Float_Input*     m_depthNumerator;
    Fl_Float_Input*     m_angleNumeratorA;
    Fl_Float_Input*     m_angleNumeratorB;
    Fl_Slider*			m_bumpScaleSlider;
    Fl_Check_Button*    m_nonRealismButton;
    Fl_Check_Button*    m_edgeRedraw;

	TraceGLWindow*		m_traceGlWindow;

	DebuggingWindow*	m_debuggingWindow;

	// member functions
	void		setRayTracer(RayTracer *tracer);
	RayTracer* getRayTracer() { return raytracer; }
    static void stopTracing();
    void useCubeMap(bool);
    void setCubeMap(bool);


private:

// static class members
	static Fl_Menu_Item menuitems[];

	static GraphicalUI* whoami(Fl_Menu_* o);

	static void cb_load_scene(Fl_Menu_* o, void* v);
	static void cb_save_image(Fl_Menu_* o, void* v);
	static void cb_cube_map(Fl_Menu_* o, void* v);
	static void cb_exit(Fl_Menu_* o, void* v);
	static void cb_about(Fl_Menu_* o, void* v);

	static void cb_exit2(Fl_Widget* o, void* v);

	static void cb_sizeSlides(Fl_Widget* o, void* v);
    static void cb_sampleSizeSlides(Fl_Widget* o, void* v);
	static void cb_depthSlides(Fl_Widget* o, void* v);
    static void cb_cubemap_checkbox(Fl_Widget* o, void* v);

	static void cb_render(Fl_Widget* o, void* v);
	static void cb_stop(Fl_Widget* o, void* v);
	static void cb_debuggingDisplayCheckButton(Fl_Widget* o, void* v);
    static void cb_accelerateCheckButton(Fl_Widget* o, void* v);
    static void cb_jitterSamplingRadioButton(Fl_Widget* o, void* v);
    static void cb_uniformSamplingRadioButton(Fl_Widget* o, void* v);
    static void cb_heuristicCheckButton(Fl_Widget* o, void* v);
    static void cb_adaptiveSamplingCheckButton(Fl_Widget* o, void* v);

    static void cb_bumpScaleSlides(Fl_Widget* o, void* v);
    static void cb_edgeRedrawCheckButton(Fl_Widget *o, void *v);
    static void cb_nonRealismCheckButton(Fl_Widget* o, void* v);
    static void cb_updateThresholds(Fl_Widget* o, void* v);

    static void cb_filter(Fl_Widget *o, void* v);

	static bool doneTrace;		// Flag that gets set when the trace is done
	static bool stopTrace;		// Flag that gets set when the trace should be stopped
};

#endif
