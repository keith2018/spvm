// from https://www.shadertoy.com/view/Nt3BzM

#define cx_arg(a) atan(a.y, a.x)


#define PI 3.14159265
#define TAU (2.*PI)

#define cx_mul(a, b) vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x)
#define cx_div(a, b) vec2(((a.x*b.x + a.y*b.y)/(b.x*b.x + b.y*b.y)),((a.y*b.x - a.x*b.y)/(b.x*b.x + b.y*b.y)))
#define cx_sin(a) vec2(sin(a.x) * cosh(a.y), cos(a.x) * sinh(a.y))
#define cx_cos(a) vec2(cos(a.x) * cosh(a.y), -sin(a.x) * sinh(a.y))

float sharp = 0.39; // delay
float b = 0.655; // brightness 0 -> dark, 1 -> bright
float nMod = 2.; // num of level curves mod
float nPhase = 20.; // num. of level curves phase
float base = 2.; // For the base of a logarithm
float nContour = 16.;

vec2 as_polar(vec2 z) {
  return vec2(
    length(z),
    atan(z.y, z.x)
  );
}

vec2 cx_tan(vec2 a) {return cx_div(cx_sin(a), cx_cos(a)); }
vec2 cx_log(vec2 a) {
    vec2 polar = as_polar(a);
    float rpart = polar.x;
    float ipart = polar.y;
    if (ipart > PI) ipart=ipart-(2.0*PI);
    return vec2(log(rpart),ipart);
}
vec2 cx_pow(vec2 v, float p) {
  vec2 z = as_polar(v);
  return pow(z.x, p) * vec2(cos(z.y * p), sin(z.y * p));
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 palette( in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d )
{
    return a + b*cos(2. * PI *(c*t+d) );
}

float funPhase(vec2 p) {
    return (PI - atan(p.y, -p.x)) / (2. * PI);
}

float funColor(vec2 p) {
    return sharp * (nContour * (PI - atan(p.y, -p.x)) / (2. * PI) -  floor(nContour * (PI - atan(p.y, -p.x)) / (2. * PI))) + 0.7;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = (fragCoord.xy - 0.5 * iResolution.xy) / min(iResolution.y, iResolution.x);
    vec2 z = uv;
    float angle = sin(iTime/5.) * TAU;
    float length = .2;
  
    // Spin our points in a circle of radius length
    float c = cos(angle);
    float s = sin(angle);
    vec2 p = vec2( s*length, c*length);
    vec2 q = vec2( s*-length, c*-length );
    
    //(z-1)/(z^3+z+1)
    
    vec2 z_minus_one = z - vec2(sin(iTime / 2.), cos(iTime));
    vec2 z_cubed_plus_one = cx_mul(cx_mul(z, z), z) + z * (sin(iTime / 10.) + 0.1) - vec2(1, 0);
    vec2 division = cx_div(z_minus_one, z_cubed_plus_one);
    

    vec3 col = palette( funPhase(division) * (8. * sin(iTime / 10.)), vec3(0.50,.52,0.53), vec3(.46,.32,.35), vec3(.82,.84,.65), vec3(0.53,0.23,0.22));
    vec3 hsv = rgb2hsv(col);
    col = vec3(hsv.r, hsv.b, funColor(division));
    //vec3 col = vec3(funPhase(division), 1., funColor(division));
    fragColor = vec4(col, 1.0);
}