// The main ray tracer. Used online source for examples.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>
//#include "ui/CubeMapChooser.h"

extern TraceUI* traceUI;

#include <iostream>
#include <fstream>
#include "globals.h"


using namespace std;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

Vec3d RayTracer::trace( double x, double y )
{
	// Clear out the ray cache in the scene for debugging purposes,
    ray r( Vec3d(0,0,0), Vec3d(0,0,0), ray::VISIBILITY );
    scene->getCamera().rayThrough( x,y,r );
    Vec3d ret = traceRay(r, Vec3d(1.f,1.f,1.f), traceUI->getDepth());
	ret.clamp();
	return ret;
}


void RayTracer::tracePixel( int i, int j )
{
    Vec3d col(0.0f,0.0f,0.0f);
    if( ! sceneLoaded() ) return;
        
    double threshold = 0.001f;
    
    /* Anti-aliasing logic */    
    double x = double(i)/double(buffer_width);
    double y = double(j)/double(buffer_height);
    int samples = traceUI->m_nSampleSize;; //anti-aliasing sample size
    double min_x = i - 0.5f;
    double min_y = j - 0.5f;
    double resample = 0.5f/(double)(samples/2);


    (*it).reserve(samples*samples); //square samples
    

    if(samples <= 1) //just normally trace
    {
        col = trace(x,y);
        //printf("normal traces\n");
    }
    else // Anti-aliasing enabled
    {

        //printf("antialiasing\n");
        std::vector<double> x_list;
        std::vector<double> y_list;
        x_list.reserve(samples);
        y_list.reserve(samples);
        int t = samples - 1;
        while(t >= 0)
        {
            x_list.push_back(min_x+t*resample);
            y_list.push_back(min_y+t*resample);
            --t;
        }
        if(traceUI->jitter()) //stochastic
        {
            Jitter<double> jitter(resample);
            std::transform(x_list.begin(), x_list.end(), x_list.begin(), jitter);
            std::transform(y_list.begin(), y_list.end(), y_list.begin(), jitter);
        }

        if(!traceUI->m_bAdaptiveSampling) // if not adaptive, supersample everything
        {
            for(std::vector<double>::iterator itX = x_list.begin();itX!=x_list.end();++itX)
            {
                for(std::vector<double>::iterator itY = y_list.begin();itY!=y_list.end();++itY)
                {
                    col+= trace(*itX/double(buffer_width),*itY/double(buffer_height));
                }
            }
            col= col / (samples*samples);
        }       
        else //supersample important areas doesn't seem to work!
        {
            int squared_samples = samples * samples; // sample squared
            std::vector<std::pair<double,double> > sample_list;
            sample_list.reserve(squared_samples);
            for (std::vector<double>::iterator ix = x_list.begin(); ix != x_list.end(); ix++)
            {
                for(std::vector<double>::iterator iy = y_list.begin(); iy != y_list.end(); iy++)
                {
                    sample_list.push_back(std::make_pair(*ix, *iy));
                }
            }
            std::vector<double> sIntensity;
            sIntensity.reserve(squared_samples);
            fillRandomIdx(sample_list.begin(),sample_list.end());

            int min_samples = 2;
            int used_samples = 0;
            double intensity_sum = 0.0f;
            for(std::vector<std::pair<double,double> >::iterator it = sample_list.begin();it!=sample_list.end();++it){
                Vec3d temp = trace((*it).first/double(buffer_width),(*it).second/double(buffer_height));
                double avg = (temp[0] + temp[1] + temp[2])/3.0f;
                sIntensity.push_back(avg);
                col = col + temp;
                intensity_sum += avg;
                ++used_samples;
                if(min_samples <= -1)
                {
                    double mean = intensity_sum/(double)used_samples;
                    ZeroMean<double> zero_mean_temp(mean);
                    std::vector<double> zeroMean(sIntensity);
                    std::transform(zeroMean.begin(),zeroMean.end(),zeroMean.begin(),zero_mean_temp);
                    double sum = std::accumulate(zeroMean.begin(),zeroMean.end(), 0.0f);
                    double tempx = sum/(double)(used_samples -1);
                    sum = sqrt(tempx);
                    if(sum < threshold)
                        break;
                }
                --min_samples;
            }
            col = col/used_samples;
        } 
    }    

    unsigned char *pixel = buffer + (i + j * buffer_width) * 3;
    col = trace(x, y);
    pixel[0] = (int)(255.0 * col[0]);
    pixel[1] = (int)(255.0 * col[1]);
    pixel[2] = (int)(255.0 * col[2]);
    it++;
    return;
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
Vec3d RayTracer::traceRay( const ray& r, const Vec3d& thresh, int depth )
{
    bool found = false;
    bool accelerate_failed = false;
    Vec3d colorC;

    isect i;

    if(!traceUI->m_accelerate) //If acceleration, use KD tree
    {
        found = kdTree.rayTreeTraversal(i, r);
        accelerate_failed = true;
        //printf("using tree\n");
    }        
    if (!found) //if not accelerate or kd tree failed, use regular
    {
        found = scene->intersect( r, i );
        /*if(accelerate_failed)
            printf("acceleration failed\n");*/
        if(found && accelerate_failed)
            printf("normal passed too\n");
    }
        
    //printf("depth %d\n", depth);

    if(found) //if there is an intersection, process it
    {
        Vec3d point = r.at(i.t);
        if(r.type() == ray::VISIBILITY)
            (*it).push_back(Descriptor(point,(-1 * r.getDirection()) * i.N));

        const Material& material = i.getMaterial();
        Vec3d total_intensity = material.shade(scene, r, i); //get initial color of intial endpoint
        if(depth < 0) //if 0 levels of recursion, we're done here
            return total_intensity;

        /* reflection logic */
        Vec3d dirA(0.0f,0.0f,0.0f);
        Vec3d dirB(0.0f,0.0f,0.0f);
        Vec3d dirC(0.0f,0.0f,0.0f);
        dirA = (((-1)*r.getDirection())*i.N)*i.N; //reverse direction
        dirB = dirA + r.getDirection(); 
        dirC = dirA + dirB;
        dirC.normalize();

        Vec3d rf_dirA(0.0f,0.0f,0.0f);
        Vec3d rf_dirB(0.0f,0.0f,0.0f);
        Vec3d rf_dirC(0.0f,0.0f,0.0f);
        bool do_refract = initialize_refractions(r, i, material, dirB, rf_dirA, rf_dirB, rf_dirC);

        ray reflectedRay(point, dirC, ray::REFLECTION);
        ray refractedRay(point, rf_dirC, ray::REFRACTION);

        Vec3d reflection_intensity = material.kr(i);
        Vec3d refraction_intensity(0.0f,0.0f,0.0f);

        //printf("Reflection intensity %f,%f,%f\n", reflection_intensity[0], reflection_intensity[1], reflection_intensity[2]);
        reflection_intensity %=  traceRay(reflectedRay, thresh, depth - 1);
        if(do_refract)
        {
            //printf("REFRACTING");
            refraction_intensity = material.kt(i);
            refraction_intensity %= traceRay(refractedRay, thresh, depth - 1);
        }
        total_intensity = total_intensity + reflection_intensity + refraction_intensity;
        colorC = total_intensity;
	} 
    else 
    {
       if(has_cube_map && cube_map != NULL) //!fix to use checkbox
        {
            return cube_map->getColor(r);
        }   

		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.
        if(r.type() == ray::VISIBILITY)
            (*it).push_back(Descriptor(Vec3d(0.0f,0.0f,0.0f),0.0f));
        colorC = Vec3d(0.0, 0.0, 0.0);   
	}
    return colorC;
}

bool RayTracer::initialize_refractions(const ray& r, const isect& i, const Material& m, const Vec3d& reflectedDirectionSi, Vec3d& refractedDirectionSt, Vec3d& refractedDirectionCt, Vec3d& refractedDir){
    if((m.kt(i)[0] <= 0.0f&&m.kt(i)[1]<=0.0f&&m.kt(i)[2]<=0.0f) || checkTotalInternal(r,i))
        return false;

    if(i.N * r.getDirection() < 0.0f){
        double ratioIndex = i.getMaterial().index(i);
        ratioIndex = 1.0f/(double)ratioIndex;
        refractedDirectionSt = ratioIndex * reflectedDirectionSi;
        double temp = refractedDirectionSt*refractedDirectionSt;
        if(temp>1)
            temp = 0;
        refractedDirectionCt = (-1.0f * i.N) * std::sqrt((1.0f - temp));
    } else if(i.N * r.getDirection() > 0.0f) {
        double ratioIndex = i.getMaterial().index(i);
        refractedDirectionSt = ratioIndex * reflectedDirectionSi;
        double temp = refractedDirectionSt*refractedDirectionSt;
        if(temp>1)
            temp = 0;
        refractedDirectionCt = i.N * std::sqrt((1.0f - temp));}

    refractedDir = refractedDirectionSt + refractedDirectionCt;
    refractedDir.normalize();
    return true;
}


bool RayTracer::checkTotalInternal(const ray &r, const isect &i){
    if(i.N * r.getDirection() > 0.0f){
        double incidentAngle = i.N * r.getDirection();
        double ratioIndex = i.getMaterial().index(i);
        if((1.0f - (ratioIndex*ratioIndex*(1.0f - incidentAngle * incidentAngle)))<0.0f)
            return true;}
    return false;
}

RayTracer::RayTracer()
	: scene( 0 ), buffer( 0 ), buffer_width( 256 ), buffer_height( 256 ), m_bBufferReady( false )
{
}


RayTracer::~RayTracer()
{
	delete scene;
	delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn )
{
	ifstream ifs( fn );
	if( !ifs ) 
    {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}
	
	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos )
		path = ".";
	else
		path = path.substr(0, path.find_last_of( "\\/" ));

	Tokenizer tokenizer( ifs, false );
    Parser parser( tokenizer, path );
	try 
    {
		delete scene;
		scene = 0;
		scene = parser.parseScene();
        if(traceUI->acceleration())
        {
            if(!kdTree.buildTree(scene->beginObjects(),scene->endObjects()))
            {
                kdTree.deleteTree();
                kdTree.buildTree(scene->beginObjects(),scene->endObjects());
            }
        }
    }
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	}
	catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	}
	catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}
	if( ! sceneLoaded() )
		return false;

	return true;
}

void RayTracer::descriptor_setup(int w, int h)
{
    _descriptors.clear();
    std::vector< std::vector<Descriptor> > temp(w * h);
    _descriptors = temp;
    it = _descriptors.begin();
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h || buffer == NULL)
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
    descriptor_setup(w, h);
	m_bBufferReady = true;
}



