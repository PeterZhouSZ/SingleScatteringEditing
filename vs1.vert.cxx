varying vec3 Ni;
varying float Si;

void main()
{
	gl_Position = ftransform(); //gl_ModelViewProjectionMatrix * gl_Vertex;
	
	Ni = vec3(gl_Normal);
	Si = (gl_ModelViewMatrix * gl_Vertex).z;
}
