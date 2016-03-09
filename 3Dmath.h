#ifndef _3DMATH_H
#define _3DMATH_H
#include "math.h"
#include "assert.h"
#include <iostream>
#define PI 3.1415926535897932f					// This is our famous PI
#define BEHIND		0
#define INTERSECTS	1
#define FRONT		2


#include <vector>
using namespace std;

struct Vec3
{
	inline Vec3() {}
	//adding it would damage performace. ~Vec3() {}
	inline Vec3(float X, float Y, float Z){ x = X; y = Y; z = Z;}
	
	//Vec3(const Vec3& v){ x = v.x; y = v.y; z = v.z;}
	
	inline Vec3 operator +(const Vec3& v) const {return Vec3(v.x + x, v.y + y, v.z + z);}

	inline Vec3 operator -(const Vec3& v) const {return Vec3(x - v.x, y - v.y, z - v.z);}

	inline Vec3 operator -(){return Vec3(-x, -y, -z);}

	inline Vec3& operator +=(const Vec3& v2) {x += v2.x; y += v2.y; z += v2.z; return(*this);}

	inline Vec3& operator -=(const Vec3& v2) {x -= v2.x; y -= v2.y; z -= v2.z; return(*this);}

	//dot
	//float operator &(const Vec3& v2) {return ( (x * v2.x) + (y * v2.y) + (z * v2.z) );}

	//cross
	//Vec3 operator *(const Vec3& v2){return Vec3((y * v2.z) - (z * v2.y), (z * v2.x) - (x * v2.z), (x * v2.y) - (y * v2.x));} 
	
	//scalar
	inline Vec3 operator +(const float& s){ return Vec3(x + s, y + s, z + s);}

	inline Vec3 operator -(const float& s){ return Vec3(x - s, y - s, z - s);}

	inline Vec3 operator *(const float& s){ return Vec3(x * s, y * s, z * s);}

	inline Vec3 operator /(const float& num){	return Vec3(x / num, y / num, z / num);	}

	inline friend Vec3 operator /(float f, Vec3 v){return Vec3(f / v.x, f / v.y, f / v.z);} 

	inline float* GetAddress(const int idx){
		switch(idx) {
		case 0:
			return &x;
			break;
		case 1:
			return &y;
			break;
		case 2:
			return &z;
			break;
		default:
			assert(0);
			return 0;
			break;
		}
	}
	//normalize
	inline void Normalize()
	{	
		float magnitude = sqrtf( (x * x) + (y * y) + (z * z) );
		if(fabs(magnitude) < 1e-5) 
		{
			cout<<"fabs(magnitude) < 1e-5"<<endl;
			//assert(0);
		}//	x = 0; 	y = 0; z = 1;}
		else 
			{ x /= magnitude; 	y /= magnitude;  z /= magnitude;}		
	}

//////////////////////////////////////////////////////////////////////////
	float x, y, z;
};


inline Vec3 FABS(const Vec3& v){return Vec3(fabs(v.x), fabs(v.y), fabs(v.z));}

inline Vec3 EXPF(const Vec3& v){return Vec3(expf(v.x), expf(v.y), expf(v.z));}

inline Vec3 SQRTF(const Vec3& v){return Vec3(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z));}	//should assert v.xyz all are nonneg!!

inline Vec3 Cross(const Vec3& v1, const Vec3& v2) {return Vec3((v1.y * v2.z) - (v1.z * v2.y), (v1.z * v2.x) - (v1.x * v2.z), (v1.x * v2.y) - (v1.y * v2.x));} 

inline float Norm(const Vec3& v) {	return sqrtf(max(0, (v.x * v.x) + (v.y * v.y) + (v.z * v.z)));}

inline Vec3 Normalize(const Vec3& v){float n = Norm(v); return Vec3(v.x / n, v.y / n, v.z / n);}

inline float Dot(const Vec3& v1, const Vec3& v2){return ( (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) );}

inline Vec3 DP(const Vec3& v1, const Vec3& v2){return Vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);}

inline Vec3 DP(const Vec3& v1, const Vec3& v2, const Vec3& v3){return Vec3(v1.x * v2.x * v3.x, v1.y * v2.y * v3.y, v1.z * v2.z * v3.z);}

inline Vec3 DD(const Vec3& v1, const Vec3& v2){return Vec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);}//Direct divide	

inline float Distance(const Vec3& v1, const Vec3& v2){return sqrtf( (v1.x-v2.x)*(v1.x-v2.x)+(v1.y-v2.y)*(v1.y-v2.y)+(v1.z-v2.z)*(v1.z-v2.z) );}

//untested!! flip side0's memory s to res-1-s. 
inline void Fliplr(byte* mat, int row, int col)
{
	byte temp = 0;
	for(int j = 0; j < row; ++j)
		for(int i = 0; i < col / 2; ++i)//NOTE res/2!!!
		{
			temp = *(mat + j * col + i);
			*(mat + j * col + i) =  *(mat + j * col + col - 1 - i);
			*(mat + j * col + col - 1 - i) = temp;
		}
}

inline void Fliplr(Vec3* mat, int row, int col)
{
	Vec3 temp;
	for(int j = 0; j < row; ++j)
		for(int i = 0; i < col / 2; ++i)//NOTE res/2!!!
		{
			temp = *(mat + j * col + i);
			*(mat + j * col + i) =  *(mat + j * col + col - 1 - i);
			*(mat + j * col + col - 1 - i) = temp;
		}
}

inline void Fliplr(float* mat, int row, int col)
{
	float temp = 0;
	for(int j = 0; j < row; ++j)
		for(int i = 0; i < col / 2; ++i)//NOTE res/2!!!
		{
			temp = *(mat + j * col + i);
			*(mat + j * col + i) =  *(mat + j * col + col - 1 - i);
			*(mat + j * col + col - 1 - i) = temp;
		}
}

//yksse
struct Vec4
{
	inline Vec4() {}
	//adding it would damage performace. ~Vec3() {}
	inline Vec4(float X, float Y, float Z, float W){ x = X; y = Y; z = Z; w = W;}
	inline Vec4 operator +(const Vec4& v) const {return Vec4(v.x + x, v.y + y, v.z + z, v.w + w);}

	inline Vec4 operator -(const Vec4& v) const {return Vec4(x - v.x, y - v.y, z - v.z, w - v.w);}

	inline Vec4 operator -(){return Vec4(-x, -y, -z, -w);}

	inline Vec4& operator +=(const Vec4& v2) {x += v2.x; y += v2.y; z += v2.z; w += v2.w; return(*this);}

	inline Vec4& operator -=(const Vec4& v2) {x -= v2.x; y -= v2.y; z -= v2.z; w -= v2.w; return(*this);}

	//scalar
	inline Vec4 operator +(const float& s){ return Vec4(x + s, y + s, z + s, w + s);}

	inline Vec4 operator -(const float& s){ return Vec4(x - s, y - s, z - s, w - s);}

	inline Vec4 operator *(const float& s){ return Vec4(x * s, y * s, z * s, w * s);}

	inline Vec4 operator /(const float& num){return Vec4(x / num, y / num, z / num, w / num);}

	inline friend Vec4 operator /(float f, Vec4 v){return Vec4(f / v.x, f / v.y, f / v.z, f / v.w);} 

	//////////////////////////////////////////////////////////////////////////
	float x, y, z, w;						
};
//yksse

//
//inline Vec3 FABS(const Vec3& v);
//
//inline Vec3 SQRTF(const Vec3& v);	//should assert v.xyz all are nonneg!!
//
//inline Vec3 EXPF(const Vec3& v);
//
//inline Vec3 Cross(const Vec3& v1, const Vec3& v2);
//
//inline float Norm(const Vec3& v);
//
//inline Vec3 Normalize(const Vec3& v);
//
//float Distance(const Vec3& v1, const Vec3& v2);
//
//inline float Dot(const Vec3& v1, const Vec3& v2);
//
//inline Vec3 DP(const Vec3& v1, const Vec3& v2);	//Direct product 
//
//inline Vec3 DP(const Vec3& v1, const Vec3& v2, const Vec3& v3); //Direct product of 3
//
//inline Vec3 DD(const Vec3& v1, const Vec3& v2);	//Direct divide	
//
//inline Vec3 VectorByMatrix(Vec3* v, float* m);//m = float[16]
//
//inline void Fliplr(byte* mat, int row, int col);
//
//inline void Fliplr(float* mat, int row, int col);
//
//inline void Fliplr(Vec3* mat, int row, int col);

//	This returns the normal of a polygon (The direction the polygon is facing)
Vec3 Normal(Vec3 vPolygon[]);

// This returns the point on the line segment vA_vB that is closest to point v
Vec3 ClosestPointOnLine(Vec3 vA, Vec3 vB, Vec3 v);

// This returns the distance the plane is from the origin (0, 0, 0)
// It takes the normal to the plane, along with ANY point that lies on the plane (any corner)
float PlaneDistance(Vec3 Normal, Vec3 Point);

// This takes a triangle (plane) and line and returns true if they intersected
bool IntersectedPlane(Vec3 vPoly[], Vec3 vLine[], Vec3 &vNormal, float &originDistance);

// This returns the angle between 2 vectors
double AngleBetweenVectors(Vec3 Vector1, Vec3 Vector2);

// This returns an intersection point of a polygon and a line (assuming intersects the plane)
Vec3 IntersectionPoint(Vec3 vNormal, Vec3 vLine[], double distance);

// This returns true if the intersection point is inside of the polygon
bool InsidePolygon(Vec3 vIntersection, Vec3 Poly[], long verticeCount);

// Use this function to test collision between a line and polygon
bool IntersectedPolygon(Vec3 vPoly[], Vec3 vLine[], int verticeCount);

// This function classifies a sphere according to a plane. (BEHIND, in FRONT, or INTERSECTS)
int ClassifySphere(Vec3 &vCenter, 
				   Vec3 &vNormal, Vec3 &v, float radius, float &distance);

// This takes in the sphere center, radius, polygon vertices and vertex count.
// This function is only called if the intersection point failed.  The sphere
// could still possibly be intersecting the polygon, but on it's edges.
bool EdgeSphereCollision(Vec3 &vCenter, 
						 Vec3 vPolygon[], int vertexCount, float radius);

// This returns true if the sphere is intersecting with the polygon.
bool SpherePolygonCollision(Vec3 vPolygon[], 
							Vec3 &vCenter, int vertexCount, float radius);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// This returns the offset the sphere needs to move in order to not intersect the plane
Vec3 GetCollisionOffset(Vec3 &vNormal, float radius, float distance);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

#endif 


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// Nothing was added to this header file except GetCollisionOffset().  This allows
// us to determine the offset that the sphere needs to move away from the plane.
//
//
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// Co-Web Host of www.GameTutorials.com
//
//


