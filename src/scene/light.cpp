#include "light.h"



using namespace std;

double DirectionalLight::distanceAttenuation( const Vec3d& P ) const
{
	return 1.0;
}



Vec3d DirectionalLight::shadowAttenuation( const Vec3d& P ) const
{
    ray rayToLight(P,getDirection(P),ray::SHADOW);
    isect i;
    if(scene->intersect( rayToLight, i ))
    {
        Vec3d color(0.0f,0.0f,0.0f);
        Vec3d kTransmit = i.getMaterial().kt(i);
        double point = i.t;
        double dist = 1.0f;
        isect i;
        Vec3d point_dest = rayToLight.at(point);
        if(scene->intersect(ray(point_dest,getDirection(point_dest),ray::SHADOW),i))
            dist = i.t;
        double light_attenuation = std::min(1.0,(double)1.0/(double)(0.2 + 0.2f*dist + 0.6f*dist*dist));
        if(kTransmit[0]>0||kTransmit[1]>0||kTransmit[2]>0)
        {
            color = light_attenuation * getColor(P);
            color %= kTransmit;
        }
        return color;
    }
    return Vec3d(1,1,1); //else return white
}

Vec3d DirectionalLight::getColor( const Vec3d& P ) const
{
	return color;
}

Vec3d DirectionalLight::getDirection( const Vec3d& P ) const
{
	return -orientation;
}

double PointLight::distanceAttenuation( const Vec3d& P ) const
{
	Vec3d v = position - P;
	return std::min(1.0,(double)1.0/(double)(constantTerm + linearTerm*v.length() + quadraticTerm*v.length()*v.length()));
}

Vec3d PointLight::getColor( const Vec3d& P ) const
{
	return color;
}

Vec3d PointLight::getDirection( const Vec3d& P ) const
{
	Vec3d ret = position - P;
	ret.normalize();
	return ret;
}


Vec3d PointLight::shadowAttenuation(const Vec3d& P) const
{
    Vec3d v = position - P;
    double t = v.length();
    v.normalize();
    ray rayToLight(P ,v ,ray::SHADOW);
    isect i;
    if(scene->intersect(rayToLight, i))
    {
        if(t>i.t)
        {
            Vec3d color(0.0f,0.0f,0.0f);
            Vec3d kTransmit = i.getMaterial().kt(i);
            double currentPoint = i.t;
            double distBetween = 1.0f;
            isect internal;
            Vec3d pointOnObject = rayToLight.at(currentPoint);
            if(scene->intersect(ray(pointOnObject,getDirection(pointOnObject),ray::SHADOW),internal))
                distBetween = internal.t;
            double light_attenuation = std::min(1.0,(double)1.0/(double)(constantTerm + linearTerm*distBetween + quadraticTerm*distBetween*distBetween));
            if(kTransmit[0]>0||kTransmit[1]>0||kTransmit[2]>0){
                color = light_attenuation * getColor(P);
                color %= kTransmit;}
            return color;
        }
    }
    return Vec3d(1,1,1);}
