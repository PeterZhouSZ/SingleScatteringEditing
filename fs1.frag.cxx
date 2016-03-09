varying vec3 Ni;
varying float Si;

void main(void)
{
	gl_FragColor = vec4(normalize(Ni), -Si);
}