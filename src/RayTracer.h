#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

// The main ray tracer.

#include "scene/ray.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include "kdtree.h"
#include <iterator>
#include "scene/cubeMap.h"


class Scene;
class RayTracer
{
public:
    struct Descriptor{
        Vec3d _point;
        double _viewAngle;
        Descriptor(Vec3d point = Vec3d(0.0f,0.0f,0.0f), double viewAngle = -1.0f):_point(point),_viewAngle(viewAngle){}};

    RayTracer();
    ~RayTracer();

    Vec3d trace( double x, double y );
	Vec3d traceRay( const ray& r, const Vec3d& thresh, int depth );

	void getBuffer( unsigned char *&buf, int &w, int &h );
	double aspectRatio();
	void traceSetup( int w, int h );
    void descriptor_setup( int w, int h );
	void tracePixel( int i, int j );
	bool loadScene( char* fn );
	bool sceneLoaded() { return scene != 0; }
    void setReady( bool ready )
      { m_bBufferReady = ready; }
    bool isReady() const
      { return m_bBufferReady; }
	const Scene& getScene() { return *scene; }
    void setCubeMap(CubeMap* m) {
        if (cubemap) delete cubemap;
        cubemap = m;
    }
    CubeMap *getCubeMap() {return cubemap;}
    bool haveCubeMap() { return cubemap != 0; }
    


private:
    std::vector<std::vector<Descriptor> > _descriptors;
    std::vector<std::vector<Descriptor> >::iterator it;
    bool initialize_refractions(const ray&, const isect&, const Material&, const Vec3d&, Vec3d&, Vec3d&, Vec3d&);
	bool checkTotalInternal(const ray&, const isect&);
    unsigned char *buffer;
	int buffer_width, buffer_height;
	int bufferSize;
	Scene* scene;;
    bool m_bBufferReady;
    KdTree<Geometry> kdTree;
    CubeMap* cubemap;
};

/* Stochastic logic */
template<typename T>
struct Jitter{
    T jitterMax;
    Jitter(T jitterMax)
    {
        this->jitterMax = jitterMax;
    }
    T operator ()(T baseVal)
    {
        float randVal = (float)rand()/RAND_MAX;
        int sign = randVal>0.5?1:-1;
        randVal = (float)rand()/RAND_MAX;
        randVal*=jitterMax;
        return (randVal*sign + baseVal);
    }
};

template<typename T>
struct UFRand
{
    unsigned int operator()(unsigned int val){
        float randVal = (float)rand()/RAND_MAX;
        randVal *= (double)val;
        return (unsigned int)(randVal + 0.5f);
    }
};

template<typename RI>
void fillRandomIdx(RI itBegin, RI itEnd)
{
    unsigned int size = itEnd - itBegin;
    std::vector<unsigned int> swapIdxOne(size,size-1);
    std::vector<unsigned int> swapIdxTwo(size,size-1);
    std::transform(swapIdxOne.begin(), swapIdxOne.end(), swapIdxOne.begin(), UFRand<unsigned int>());
    std::transform(swapIdxTwo.begin(), swapIdxTwo.end(), swapIdxTwo.begin(), UFRand<unsigned int>());

    std::vector<unsigned int>::iterator itOne = swapIdxOne.begin();
    std::vector<unsigned int>::iterator itTwo = swapIdxTwo.begin();

    RI initTemp = itBegin;
    while(itBegin!=itEnd){
        typename RI::value_type temp;
        temp = initTemp[*itOne];
        initTemp[*itOne] = initTemp[*itTwo];
        initTemp[*itTwo] = temp;
        ++itBegin;
        ++itOne;
        ++itTwo;
    }
}

template<typename T>
struct ZeroMean{
    T _mean;
    ZeroMean(T mean):_mean(mean){}
    T operator()(T val){
        T temp = val - _mean;
        return (temp*temp);}};

template <typename RI, typename BII>
void loadNeighbours(int currY, int currX, int knlWidth, int knlHeight, int srcBufferWidth, int srcBufferHeight, RI begin, BII neighbours){
    int top = -knlHeight/2;
    int bottom = knlHeight/2;
    int left = -knlWidth/2;
    int right = knlWidth/2;

    if(knlHeight%2 == 0){
        --bottom;}

    if(knlWidth%2 == 0){
        --right;}

    int neighbourX = -1;
    int neighbourY = -1;

    for (int y = top;y<=bottom;++y){
        for(int x = left;x<=right;++x){
            neighbourY = currY + y;
            neighbourX = currX + x;
            if(neighbourX<0||neighbourY<0||(x==0&&y==0)||neighbourX>=srcBufferWidth||neighbourY>=srcBufferHeight){
                continue;}
            *neighbours = begin + (neighbourY*srcBufferWidth+neighbourX);}}}

template <typename RI>
void loadAvgVals(RI begin, RI end, Vec3d& point, double& viewAngle){
    int numSamples = 0;
    while(begin!=end){
        Vec3d thisPoint = (*begin)._point;
        double thisAngle = (*begin)._viewAngle;
        point += thisPoint;
        viewAngle += thisAngle;
        ++begin;
        ++numSamples;}
    point = point/(double)numSamples;
    viewAngle = viewAngle/(double)numSamples;}

#endif // __RAYTRACER_H__
