#version 150
attribute vec2 pos;
attribute vec2 speed;
attribute float time;
attribute vec2 uv;

attribute vec2 prevpos;
attribute vec2 prevspeed;
attribute float prevtime;

uniform float uCurrentTime;
uniform mat3 uViewMatrix;

varying float t;
varying float y;

void main(void) 
{
	float dt = uCurrentTime - time;
	
    float shift = 0.04;
    
    vec2 curPos = pos + ( dt  + min(dt, shift) ) * speed;
    
	float prevdt = uCurrentTime - prevtime;
    vec2 prevPos = prevpos + ( prevdt  + min(prevdt, shift) )* prevspeed;
    
    
    float rotation = atan( prevPos.y - curPos.y, prevPos.x - curPos.x);
    
	float s = sin(rotation);
	float c = cos(rotation);
    
	vec3 worldPos = vec3(curPos + vec2(0.0, uv.y - 0.5) * mat2(c, -s, s, c) * 30.0, 1.0);
	vec3 viewPos = uViewMatrix * worldPos;
	gl_Position = vec4(viewPos.x, viewPos.y, 0.0, 1.0);
    
    t = dt / 0.1;
    y =  (uv.y - 0.5) * 2.0;
}