#include "ray.h"
#include "material.h"
#include "light.h"

#include "../fileio/bitmap.h"
#include "../fileio/pngimage.h"
#include "../ui/TraceUI.h"

using namespace std;
extern bool debugMode;
extern TraceUI* traceUI;

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
Vec3d Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
    Vec3d intensity(0.0f,0.0f,0.0f);
    Vec3d e_intensity = ke(i);
    Vec3d a_intensity = ka(i);
    a_intensity %=  scene->ambient();
	const Vec3d intsec = r.at(i.t);
    double a_x = 0.2f;
    double b_x = 0.6f;
    const Vec3d blue_temp(0.0f,0.0f,0.4f);
    const Vec3d yellow_temp(0.4f,0.4f,0.0f);
    bool is_not_real = traceUI->nonRealism();
	for ( vector<Light*>::const_iterator litr = scene->beginLights(); litr != scene->endLights(); ++litr ){
            Light* point = *litr;
            Vec3d light_direction = point->getDirection(intsec);
			Vec3d surface_normal = i.N;
			double light_normal = surface_normal * light_direction;
            Vec3d dirA = (light_direction*surface_normal)*surface_normal;
			Vec3d dirB = dirA - light_direction;
            Vec3d dirC = dirA + dirB;
			dirC.normalize();
            double ray_normal = (-1 * r.getDirection()) * dirC;
            Vec3d shadow(1.0f,1.0f,1.0f);

            if(is_not_real)
            {
                double cool = (1.0f+ (-1.0f)* light_normal)/2.0f;
                double warm = 1.0f - cool;
                Vec3d diffuse = (cool * ( blue_temp + kd(i) * a_x)) + (warm * ( yellow_temp + kd(i) * b_x));
                //Vec3d specular = ks(i) * point->getColor(intsec);
                shadow %= diffuse; //+ specular * std::pow(std::max(ray_normal,0.0),shininess(i)));
            } 
            else 
            {
                if(bump(i, traceUI->getBumpScale())[0]!= 2.0f){
                    Vec3d perturbed = 2.0f*bump(i, traceUI->getBumpScale())-1.0f;
                    perturbed.normalize();
                    light_normal = perturbed * light_direction;
            }
                Vec3d diffuse = kd(i);
                Vec3d specular = ks(i);
                diffuse%=point->getColor(intsec);
                specular%=point->getColor(intsec);
                shadow = point->shadowAttenuation(intsec);
                shadow %= (diffuse * std::max(light_normal, 0.0) + specular * std::pow(std::max(ray_normal,0.0),shininess(i)));
            }
            intensity += shadow*point->distanceAttenuation(intsec);
    }
    return (intensity + e_intensity + a_intensity);
}

TextureMap::TextureMap(string filename) {

	int start = filename.find_last_of('.');
	int end = filename.size() - 1;
	if (start >= 0 && start < end) {
		string ext = filename.substr(start, end);
		if (!ext.compare(".png")) {
			png_cleanup(1);
			if (!png_init(filename.c_str(), width, height)) {
				double gamma = 2.2;
				int channels, rowBytes;
				unsigned char* indata = png_get_image(gamma, channels, rowBytes);
				int bufsize = rowBytes * height;
				data = new unsigned char[bufsize];
				for (int j = 0; j < height; j++)
					for (int i = 0; i < rowBytes; i += channels)
						for (int k = 0; k < channels; k++)
							*(data + k + i + j * rowBytes) = *(indata + k + i + (height - j - 1) * rowBytes);
				png_cleanup(1);
			}
		}
		else
			if (!ext.compare(".bmp")) data = readBMP(filename.c_str(), width, height);
			else data = NULL;
	} else data = NULL;
	if (data == NULL) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

BumpMap::BumpMap(string filename)
{
    /* Maybe implement later */
}

Vec3d TextureMap::getMappedValue( const Vec2d& coord ) const
{


    float xcoord = coord[0] * width,
    yCor = coord[1] * height;
    int xIDX = (int)xcoord;
    int yIDX = (int)yCor;
    float dX = xcoord - xIDX, dY = yCor - yIDX;
    float a = (1-dX)*(1-dY), b = (dX)*(1-dY), c = (1-dX)*dY, d = dX*dY;
    return a * getPixelAt(xIDX,yIDX) + b*getPixelAt(xIDX+1,yIDX) + c*getPixelAt(xIDX,yIDX+1) + d*getPixelAt(xIDX+1,yIDX+1);
}

Vec3d BumpMap::getDiffValue( const Vec2d& coord , const float scale) const
{
    float xCor = coord[0] * width,
    yCor = coord[1] * height;
    int x = (int)xCor, y = (int)yCor;
    int right = (int)x + 1;
    int top = (int)y + 1;
    Vec3d diffX = getPixelAt(x,y) - getPixelAt(right, y);
    double avgX = scale*(diffX[0]+diffX[1]+diffX[2])/(double)3.0f;
    Vec3d diffY = getPixelAt(x,y) - getPixelAt(x, top);
    double avgY = scale*(diffY[0]+diffY[1]+diffY[2])/(double)3.0f;
    Vec3d perturb(avgX, avgY, 1);
    perturb.normalize();
    return perturb;
}


Vec3d TextureMap::getPixelAt( int x, int y ) const
{
    // This keeps it from crashing if it can't load
    // the texture, but the person tries to render anyway.
    if (0 == data)
      return Vec3d(1.0, 1.0, 1.0);

    if( x >= width )
       x = width - 1;
    if( y >= height )
       y = height - 1;

    // Find the position in the big data array...
    int pos = (y * width + x) * 3;
    return Vec3d( double(data[pos]) / 255.0,
       double(data[pos+1]) / 255.0,
       double(data[pos+2]) / 255.0 );
}

Vec3d BumpMap::getPixelAt( int x, int y ) const
{
    // This keeps it from crashing if it can't load
    // the texture, but the person tries to render anyway.
    if (0 == data)
      return Vec3d(1.0, 1.0, 1.0);

    if( x >= width )
       x = width - 1;
    if( y >= height )
       y = height - 1;

    // Find the position in the big data array...
    int pos = (y * width + x) * 3;
    return Vec3d( double(data[pos]) / 255.0,
       double(data[pos+1]) / 255.0,
       double(data[pos+2]) / 255.0 );
}

Vec3d MaterialParameter::value( const isect& is ) const
{
    if( 0 != _textureMap )
        return _textureMap->getMappedValue( is.uvCoordinates );
    else
        return _value;
}

Vec3d MaterialParameter::valueTMP( const isect& is , const float scale) const
{
    if( 0 != _bumpMap )
        return _bumpMap->getDiffValue( is.uvCoordinates , scale);
    else
        return Vec3d(2.0f,2.0f,2.0f);
}

double MaterialParameter::intensityValue( const isect& is ) const
{
    if( 0 != _textureMap )
    {
        Vec3d value( _textureMap->getMappedValue( is.uvCoordinates ) );
        return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
    }
    else
        return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}

