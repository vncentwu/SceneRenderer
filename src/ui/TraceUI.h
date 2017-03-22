//
// rayUI.h
//
// The header file for the UI part
//

#ifndef __rayUI_h__
#define __rayUI_h__

// who the hell cares if my identifiers are longer than 255 characters:
#pragma warning(disable : 4786)

#include <math.h>
#include "../vecmath/vec.h"
#include "../vecmath/mat.h"

#include <string>

using std::string;

class RayTracer;

class TraceUI {
public:
	TraceUI()
		: m_nDepth(0), m_nSize(150), 
		m_displayDebuggingInfo( false ),
		raytracer( 0 )
	{ }

	virtual int		run() = 0;

	// Send an alert to the user in some manner
    virtual void		alert(const string& msg) = 0;

	// setters
	virtual void		setRayTracer( RayTracer* r )
		{ raytracer = r; }

	// accessors:
	int		getSize() const { return m_nSize; }
    int		getDepth() const { return m_nDepth; }
    bool    acceleration() const { return !m_accelerate; }
    bool    jitter() const { return m_bJitter; }
    bool    uniform() const { return m_bUniform; }
    bool    useSurface() const { return m_bSurfaceHeuristic; }
    int     getSampleSize() const { return m_nSampleSize; }
    bool    getAdapativeSampling() const { return m_bAdaptiveSampling;}
    float   getBumpScale() const { return m_fBumpScale;}
    float   getDepthThreshold() const { return m_fDepthThreshold;}
    float   getAngleThresholdA() const { return m_fAngleThresholdA;}
    float   getAngleThresholdB() const { return m_fAngleThresholdB;}
    bool    nonRealism() const { return m_bNonRealism;}
    bool    edgeRedraw() const { return m_bEdgeRedraw;}
    int     getFilterWidth() const {return m_nFilterWidth; }

	RayTracer*	raytracer;

    bool m_shadows;  // compute shadows?
    bool m_smoothshade;  // turn on/off smoothshading?
    bool m_usingCubeMap;  // render with cubemap
    bool m_gotCubeMap;  // cubemap defined
    int m_nFilterWidth;  // width of cubemap filter

	int			m_nSize;				// Size of the traced image
    int			m_nDepth;				// Max depth of recursion
    bool        m_accelerate;           // acceleration preprocess is on?
    int         m_nSampleSize;          // super sample size
    bool        m_bJitter;
    bool        m_bUniform;
    bool        m_bSurfaceHeuristic;
    bool        m_bAdaptiveSampling;
    float       m_fBumpScale;
    float       m_fDepthThreshold;
    float       m_fAngleThresholdA;
    float       m_fAngleThresholdB;
    bool        m_bNonRealism;
    bool        m_bEdgeRedraw;




	// Determines whether or not to show debugging information
	// for individual rays.  Disabled by default for efficiency
    // reasons.
    bool		m_displayDebuggingInfo;

};

#endif
