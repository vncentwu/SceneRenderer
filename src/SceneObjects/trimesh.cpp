#include <cmath>
#include <float.h>
#include "trimesh.h"

#include "../ui/TraceUI.h"

extern TraceUI* traceUI;

using namespace std;

Trimesh::~Trimesh()
{
	for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
		delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const Vec3d &v )
{
    vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
    materials.push_back( m );
}

void Trimesh::addNormal( const Vec3d &n )
{
    normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
    int vcnt = vertices.size();

    if( a >= vcnt || b >= vcnt || c >= vcnt ) return false;

    TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material), this, a, b, c );
    newFace->setTransform(this->transform);
    faces.push_back( newFace );
    return true;
}

char *
Trimesh::doubleCheck()
// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
{
    if( !materials.empty() && materials.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of materials.";
    if( !normals.empty() && normals.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

bool Trimesh::intersectLocal(const ray&r, isect&i) const
{
    double tmin = 0.0;
	double tmax = 0.0;
	typedef Faces::const_iterator iter;
    bool have_one = false;
    if(traceUI->acceleration())
        have_one = const_cast<Trimesh*>(this)->kdTree.rayTreeTraversal(i,r);
    else
        for( iter j = faces.begin(); j != faces.end(); ++j ) {
            isect cur;
            if( (*j)->intersectLocal( r, cur ) )
            {
                if( !have_one || (cur.t < i.t) )
                {
                    i = cur;
                    have_one = true;
                }
            }
        }
	if( !have_one ) i.setT(1000.0);
	return have_one;
}

void Trimesh::constructKDTree()
{
    this->kdTree.buildTree(this->faces.begin(),this->faces.end());
}


bool TrimeshFace::intersect(const ray &r, isect &i) const {
    return intersectLocal(r,i);}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and puts the t parameter, barycentric coordinates, normal, object id,
// and object material in the isect object
bool TrimeshFace::intersectLocal( const ray& r, isect& i ) const
{
    //Mostly done
    const Vec3d& a = parent->vertices[ids[0]];
    const Vec3d& b = parent->vertices[ids[1]];
    const Vec3d& c = parent->vertices[ids[2]];
    Vec3d planeNormal = (a-c)^(b-c);
    planeNormal.normalize();
    
    int cordToDrop = updateCordsToKeep(3, planeNormal);

    double planeDistance = -(planeNormal*a);
    double normalProj = (planeNormal * r.getDirection()); //dot product
    double intersectionWt = 0;

    if(normalProj == 0)
        return false;

    intersectionWt = - (planeNormal*r.getPosition() + planeDistance)/normalProj;
    if(intersectionWt <= RAY_EPSILON )
        return false;

    Vec3d intersectionPoint = r.at(intersectionWt);
    Mat3d temp(a.n[0],b.n[0],c.n[0],a.n[1],b.n[1],c.n[1],1,1,1);
    Vec3d sol(intersectionPoint.n[0],intersectionPoint.n[1],1);

    if(cordToDrop == 0){
        Vec3d tempSol(1,intersectionPoint.n[1],intersectionPoint.n[2]);
        Mat3d tempMat(1,1,1,a.n[1],b.n[1],c.n[1],a.n[2],b.n[2],c.n[2]);
        sol = tempSol;
        temp = tempMat; 
    } else if(cordToDrop == 1){
        Vec3d tempSol(intersectionPoint.n[0],1,intersectionPoint.n[2]);
        Mat3d tempMat(a.n[0],b.n[0],c.n[0],1,1,1,a.n[2],b.n[2],c.n[2]);
        sol = tempSol;
        temp = tempMat;}



    Vec3d barycentricCords = temp.inverse() * sol;

    if(barycentricCords[0]==0&&barycentricCords[1]==0&&barycentricCords[2]==0)
        return false;
    if(!(barycentricCords[0]>=0&&barycentricCords[1]>=0&&barycentricCords[2]>=0))
        return false;

    // phong interpolation
    if(parent->vertNorms){
        Vec3d weightFromA(parent->normals[ids[0]][0]*barycentricCords[0] , parent->normals[ids[0]][1]*barycentricCords[0] , parent->normals[ids[0]][2]*barycentricCords[0] );
        Vec3d weightFromB(parent->normals[ids[1]][0]*barycentricCords[1] , parent->normals[ids[1]][1]*barycentricCords[1] , parent->normals[ids[1]][2]*barycentricCords[1] );
        Vec3d weightFromC(parent->normals[ids[2]][0]*barycentricCords[2] , parent->normals[ids[2]][1]*barycentricCords[2] , parent->normals[ids[2]][2]*barycentricCords[2] );
        planeNormal = weightFromA + weightFromB + weightFromC;
        planeNormal.normalize();
    }

    i.setN(planeNormal);
    i.setT(intersectionWt);
    i.setBary(barycentricCords);
    i.setMaterial(getMaterial());
    //i.setUVCoordinates(Vec2d(barycentricCords[0],barycentricCords[1]));
    return true;}

void Trimesh::generateNormals()
// Once you've loaded all the verts and faces, we can generate per
// vertex normals by averaging the normals of the neighboring faces.
{
    int cnt = vertices.size();
    normals.resize( cnt );
    int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
    memset( numFaces, 0, sizeof(int)*cnt );
    
    for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
    {
		Vec3d faceNormal = (**fi).getNormal();
        
        for( int i = 0; i < 3; ++i )
        {
            normals[(**fi)[i]] += faceNormal;
            ++numFaces[(**fi)[i]];
        }
    }

    for( int i = 0; i < cnt; ++i )
    {
        if( numFaces[i] )
            normals[i]  /= numFaces[i];
    }

    delete [] numFaces;
    vertNorms = true;
}
