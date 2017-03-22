//
// GraphicalUI.cpp
//
// Handles FLTK integration and other user interface tasks
//
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#ifndef COMMAND_LINE_ONLY

#include <FL/fl_ask.H>
#include "debuggingView.h"

#include "GraphicalUI.h"
#include "../RayTracer.h"
#include "../globals.h"

bool GraphicalUI::stopTrace = false;
bool GraphicalUI::doneTrace = true;
GraphicalUI* pUI;
CubeMapChooser* cube;
int m_nCube_filter;


//------------------------------------- Help Functions --------------------------------------------
GraphicalUI* GraphicalUI::whoami(Fl_Menu_* o)	// from menu item back to UI itself
{
	return ( (GraphicalUI*)(o->parent()->user_data()) );
}

//--------------------------------- Callback Functions --------------------------------------------
void GraphicalUI::cb_load_scene(Fl_Menu_* o, void* v) 
{
	GraphicalUI* pUI=whoami(o);
	
	static char* lastFile = 0;
	char* newfile = fl_file_chooser("Open Scene?", "*.ray", NULL );

	if (newfile != NULL) {
		char buf[256];

		if (pUI->raytracer->loadScene(newfile)) {
			sprintf(buf, "Ray <%s>", newfile);
			stopTracing();	// terminate the previous rendering
		} else{
			sprintf(buf, "Ray <Not Loaded>");
		}

		pUI->m_mainWindow->label(buf);
		pUI->m_debuggingWindow->m_debuggingView->setDirty();

		if( lastFile != 0 && strcmp(newfile, lastFile) != 0 )
			pUI->m_debuggingWindow->m_debuggingView->resetCamera();

		pUI->m_debuggingWindow->redraw();
	}
}

void GraphicalUI::cb_save_image(Fl_Menu_* o, void* v) 
{
	char* savefile = fl_file_chooser("Save Image?", "*.bmp", "save.bmp" );
	if (savefile != NULL)
		whoami(o)->m_traceGlWindow->saveImage(savefile);
}

void GraphicalUI::useCubeMap(bool b)
{

}

void GraphicalUI::setCubeMap(bool b)
{

}

void GraphicalUI::cb_cube_map(Fl_Menu_* o, void* v) 
{ 
    cube->show();
}


void GraphicalUI::cb_exit(Fl_Menu_* o, void* v)
{
	pUI=whoami(o);

	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
}

void GraphicalUI::cb_exit2(Fl_Widget* o, void* v) 
{
	pUI=(GraphicalUI *)(o->user_data());
	
	// terminate the rendering
	stopTracing();

	pUI->m_traceGlWindow->hide();
	pUI->m_mainWindow->hide();
	pUI->m_debuggingWindow->hide();
}

void GraphicalUI::cb_about(Fl_Menu_* o, void* v) 
{
	fl_message("CS354 Raytracter Project 6 - Vincent Wu");
}


void GraphicalUI::cb_sizeSlides(Fl_Widget* o, void* v)
{
    pUI=(GraphicalUI*)(o->user_data());

    // terminate the rendering so we don't get crashes
    stopTracing();

    pUI->m_nSize=int( ((Fl_Slider *)o)->value() ) ;
    int	height = (int)(pUI->m_nSize / pUI->raytracer->aspectRatio() + 0.5);
    pUI->m_traceGlWindow->resizeWindow( pUI->m_nSize, height );
    // Need to call traceSetup before trying to render
    pUI->raytracer->setReady(false);
}

void GraphicalUI::cb_depthSlides(Fl_Widget* o, void* v)
{
	((GraphicalUI*)(o->user_data()))->m_nDepth=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_jitterSamplingRadioButton(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_bJitter= true ;
    ((GraphicalUI*)(o->user_data()))->m_bUniform= false ;
}

void GraphicalUI::cb_uniformSamplingRadioButton(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_bJitter= false ;
    ((GraphicalUI*)(o->user_data()))->m_bUniform= true ;
}

void GraphicalUI::cb_sampleSizeSlides(Fl_Widget* o, void* v)
{
    ((GraphicalUI*)(o->user_data()))->m_nSampleSize=int( ((Fl_Slider *)o)->value() ) ;
}

void GraphicalUI::cb_debuggingDisplayCheckButton(Fl_Widget* o, void* v)
{
	GraphicalUI* pUI=(GraphicalUI*)(o->user_data());
	pUI->m_displayDebuggingInfo = (((Fl_Check_Button*)o)->value() == 1);
	if( pUI->m_displayDebuggingInfo )
		pUI->m_debuggingWindow->show();
	else
		pUI->m_debuggingWindow->hide();
}

void GraphicalUI::cb_adaptiveSamplingCheckButton(Fl_Widget* o, void* v)
{
    GraphicalUI* pUI=(GraphicalUI*)(o->user_data());
    pUI->m_bAdaptiveSampling = (((Fl_Check_Button*)o)->value() == 1);
}

void GraphicalUI::cb_accelerateCheckButton(Fl_Widget* o, void* v)
{
    GraphicalUI* pUI=(GraphicalUI*)(o->user_data());
    pUI->m_accelerate = (((Fl_Check_Button*)o)->value() != 1);
}

void GraphicalUI::cb_render(Fl_Widget* o, void* v)
    {
	char buffer[256];

	pUI=((GraphicalUI*)(o->user_data()));
	
	if (pUI->raytracer->sceneLoaded()) 
	{
		int width = pUI->getSize();
		int	height = (int)(width / pUI->raytracer->aspectRatio() + 0.5);
		pUI->m_traceGlWindow->resizeWindow( width, height );
		pUI->m_traceGlWindow->show();
		pUI->raytracer->traceSetup(width, height);
		const char *old_label = pUI->m_traceGlWindow->label();
		clock_t prev, now;
		prev=clock();
		pUI->m_traceGlWindow->refresh();
		Fl::check();
		Fl::flush();
		doneTrace = false;
		stopTrace = false;
		for (int y=0; y<height; y++) 
		{
			for (int x=0; x<width; x++) 
			{
				if (stopTrace) 
					break;
				now = clock();
				if (((double)(now-prev)/CLOCKS_PER_SEC)>0.5) {
					prev=now;
					pUI->m_traceGlWindow->refresh();
					Fl::check();
					if (Fl::damage())
						Fl::flush();
                }
				pUI->raytracer->tracePixel( x, y );
				pUI->m_debuggingWindow->m_debuggingView->setDirty();
            }
			if (stopTrace) 
				break;
			pUI->m_traceGlWindow->refresh();
			Fl::check();
			if (Fl::damage())
				Fl::flush();
			sprintf(buffer, "(%d%%) %s", (int)((double)y / (double)height * 100.0), old_label);
			pUI->m_traceGlWindow->label(buffer);			
		}
        if(!stopTrace)
        {
            pUI->m_traceGlWindow->refresh();
            Fl::check();

            if (Fl::damage()) {
                Fl::flush();
            }
            //pUI->raytracer->drawEdges();
            pUI->m_traceGlWindow->refresh();
            Fl::check();

            if (Fl::damage()) {
                Fl::flush();
            }
            pUI->m_traceGlWindow->label(buffer);}
		doneTrace=true;
		stopTrace=false;
		pUI->m_traceGlWindow->refresh();
		pUI->m_traceGlWindow->label(old_label);		
	}
}

void GraphicalUI::cb_cubemap_checkbox(Fl_Widget* o, void* v)
{
	has_cube_map = (((Fl_Check_Button*)o)->value() == 1);
}

void GraphicalUI::cb_stop(Fl_Widget* o, void* v)
{
	stopTrace = true;
}

void GraphicalUI::cb_filter(Fl_Widget* o, void* v)
{
	m_nCube_filter = int( ((Fl_Slider *)o)->value() ) ;	
}

int GraphicalUI::run()
{
	Fl::visual(FL_DOUBLE|FL_INDEX);

    cube = new CubeMapChooser();
    cube->setCaller(this);
	m_mainWindow->show();

	return Fl::run();
}

void GraphicalUI::alert( const string& msg )
{
	fl_alert( "%s", msg.c_str() );
}

void GraphicalUI::setRayTracer(RayTracer *tracer)
{
	TraceUI::setRayTracer( tracer );
	m_traceGlWindow->setRayTracer(tracer);
	m_debuggingWindow->m_debuggingView->setRayTracer(tracer);
}

// menu definition
Fl_Menu_Item GraphicalUI::menuitems[] = {
	{ "&File",		0, 0, 0, FL_SUBMENU },
		{ "&Load Scene...",	FL_ALT + 'l', (Fl_Callback *)GraphicalUI::cb_load_scene },
		{ "&Save Image...",	FL_ALT + 's', (Fl_Callback *)GraphicalUI::cb_save_image },
        { "&Cubemap...", FL_ALT + 'C', (Fl_Callback *)GraphicalUI::cb_cube_map },
		{ "&Exit",			FL_ALT + 'e', (Fl_Callback *)GraphicalUI::cb_exit },
		{ 0 },

	{ "&Help",		0, 0, 0, FL_SUBMENU },
		{ "&About",	FL_ALT + 'a', (Fl_Callback *)GraphicalUI::cb_about },
		{ 0 },

	{ 0 }
};

void GraphicalUI::stopTracing()
{
	stopTrace = true;
}

GraphicalUI::GraphicalUI() {
	// init.

    m_mainWindow = new Fl_Window(100, 40, 500, 300, "Ray Tracer<EMPTY>");
		m_mainWindow->user_data((void*)(this));	// record self to be used by static callback functions
		// install menu bar
		m_menubar = new Fl_Menu_Bar(0, 0, 500, 25);
		m_menubar->menu(menuitems);

		// install depth slider
        m_nDepth = 2;
		m_depthSlider = new Fl_Value_Slider(10, 55, 180, 20, "Recursion Depth");
		m_depthSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_depthSlider->type(FL_HOR_NICE_SLIDER);
        m_depthSlider->labelfont(FL_HELVETICA);
        m_depthSlider->labelsize(12);
		m_depthSlider->minimum(0);
		m_depthSlider->maximum(10);
		m_depthSlider->step(1);
		m_depthSlider->value(m_nDepth);
		m_depthSlider->align(FL_ALIGN_RIGHT);
		m_depthSlider->callback(cb_depthSlides);

		// install size slider
		m_sizeSlider = new Fl_Value_Slider(10, 30, 180, 20, "Screen Size");
		m_sizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
		m_sizeSlider->type(FL_HOR_NICE_SLIDER);
        m_sizeSlider->labelfont(FL_HELVETICA);
        m_sizeSlider->labelsize(12);
		m_sizeSlider->minimum(64);
		m_sizeSlider->maximum(1024);
		m_nSize = 256;
		m_sizeSlider->step(1);
		m_sizeSlider->value(m_nSize);
		m_sizeSlider->align(FL_ALIGN_RIGHT);
		m_sizeSlider->callback(cb_sizeSlides);

        // install sample size slider
        m_nSampleSize = 1;
        m_sampleSizeSlider = new Fl_Value_Slider(10, 80, 180, 20, "Anti-aliasing Sampling Size");
        m_sampleSizeSlider->user_data((void*)(this));	// record self to be used by static callback functions
        m_sampleSizeSlider->type(FL_HOR_NICE_SLIDER);
        m_sampleSizeSlider->labelfont(FL_HELVETICA);
        m_sampleSizeSlider->labelsize(12);
        m_sampleSizeSlider->minimum(1);
        m_sampleSizeSlider->maximum(8);
        m_sampleSizeSlider->step(1);
        m_sampleSizeSlider->value(m_nSampleSize);
        m_sampleSizeSlider->align(FL_ALIGN_RIGHT);
        m_sampleSizeSlider->callback(cb_sampleSizeSlides);

        // turn uniform sampling
        m_bUniform = true;
        m_uniformSamplingButton = new Fl_Round_Button(0, 110, 180, 20, "Full Sampling");
        m_uniformSamplingButton->user_data((void*)(this));
        m_uniformSamplingButton->callback(cb_jitterSamplingRadioButton);
        m_uniformSamplingButton->type(FL_RADIO_BUTTON);
        m_uniformSamplingButton->labelfont(FL_HELVETICA);
        m_uniformSamplingButton->setonly();
        m_uniformSamplingButton->labelsize(12);

        // turn jitter sampling
        m_bJitter = false;
        m_jitterSamplingButton = new Fl_Round_Button(0, 140, 180, 20, "Stochastic Sampling");
        m_jitterSamplingButton->user_data((void*)(this));
        m_jitterSamplingButton->callback(cb_uniformSamplingRadioButton);
        m_jitterSamplingButton->type(FL_RADIO_BUTTON);
        m_jitterSamplingButton->labelfont(FL_HELVETICA);
        m_jitterSamplingButton->labelsize(12);


        // set up cubemap checkbox
        has_cube_map = false;
        m_cubeMapCheckButton = new Fl_Check_Button(0, 170, 120, 20, "Cubemap");
        m_cubeMapCheckButton->user_data((void*)(this));
        m_cubeMapCheckButton->callback(cb_cubemap_checkbox);
        //m_cubeMapCheckButton->set();
        m_cubeMapCheckButton->labelfont(FL_HELVETICA);
        m_cubeMapCheckButton->labelsize(12);

        m_nCube_filter = 2;
		cubeSlider = new Fl_Value_Slider(130, 170, 180, 20, "Filter");
		cubeSlider->user_data((void*)(this));	// record self to be used by static callback functions
		cubeSlider->type(FL_HOR_NICE_SLIDER);
        cubeSlider->labelfont(FL_HELVETICA);
        cubeSlider->labelsize(12);
		cubeSlider->minimum(1);
		cubeSlider->maximum(17);
		cubeSlider->step(1);
		cubeSlider->value(m_nCube_filter);
		cubeSlider->align(FL_ALIGN_RIGHT);
		cubeSlider->callback(cb_filter);


        //m_bSurfaceHeuristic = true;

        // set up adaptive checkbox
/*        m_bAdaptiveSampling = true;
        m_adaptiveSamplingButton = new Fl_Check_Button(0, 260, 180, 20, "Adaptive Sampling");
        m_adaptiveSamplingButton->user_data((void*)(this));
        m_adaptiveSamplingButton->callback(cb_adaptiveSamplingCheckButton);
        m_adaptiveSamplingButton->set();
        m_adaptiveSamplingButton->labelfont(FL_HELVETICA);
        m_adaptiveSamplingButton->labelsize(12);*/

        // set up acceleration checkbox
        m_accelerate = false;
        m_accelerateCheckButton = new Fl_Check_Button(0, 200, 180, 20, "Acceleration(K-d tree) (Toggle before load)");
        m_accelerateCheckButton->user_data((void*)(this));
        m_accelerateCheckButton->callback(cb_accelerateCheckButton);
        m_accelerateCheckButton->set();
        m_accelerateCheckButton->labelfont(FL_HELVETICA);
        m_accelerateCheckButton->labelsize(12);

        // set up debugging display checkbox
        m_debuggingDisplayCheckButton = new Fl_Check_Button(0, 230, 180, 20, "Debugging display");
        m_debuggingDisplayCheckButton->user_data((void*)(this));
        m_debuggingDisplayCheckButton->callback(cb_debuggingDisplayCheckButton);
        m_debuggingDisplayCheckButton->value(m_displayDebuggingInfo);
        m_debuggingDisplayCheckButton->labelfont(FL_HELVETICA);
        m_debuggingDisplayCheckButton->labelsize(12);

		// set up "render" button
        m_renderButton = new Fl_Button(400, 55, 70, 25, "&Render");
		m_renderButton->user_data((void*)(this));
        m_renderButton->callback(cb_render);
        m_renderButton->labelfont(FL_HELVETICA);
        m_renderButton->labelsize(12);

        // set up "stop" button
        m_stopButton = new Fl_Button(400, 82, 70, 25, "&Stop");
        m_stopButton->user_data((void*)(this));
        m_stopButton->callback(cb_stop);
        m_stopButton->labelfont(FL_HELVETICA);
        m_stopButton->labelsize(12);

		m_mainWindow->callback(cb_exit2);
		m_mainWindow->when(FL_HIDE);

    m_mainWindow->end();

	// image view
	m_traceGlWindow = new TraceGLWindow(100, 150, m_nSize, m_nSize, "Rendered Image");
	m_traceGlWindow->end();
	m_traceGlWindow->resizable(m_traceGlWindow);

	// debugging view
	m_debuggingWindow = new DebuggingWindow();
}

#endif

