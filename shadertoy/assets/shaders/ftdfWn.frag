// from https://www.shadertoy.com/view/ftdfWn

#define ITER_MAX 13
#define TIME_SCALE 6
#define ZOOM 4
//#define CUSTOM_EXPONENT vec2(-2., 0.)



// conversions between cartesian (x+yi) and polar (xe^(iy)) forms of complex numbers
vec2 cartesian_of_polar(vec2 polar) {
    return vec2(polar.x * cos(polar.y), polar.x * sin(polar.y));
}

vec2 polar_of_cartesian(vec2 cartesian) {
    return vec2(length(cartesian), atan(cartesian.y, cartesian.x));
}

// multiplication of two complex numbers in cartesian form
vec2 cmul(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

// exponential of a complex number by a complex number, both in cartesian form
vec2 cexp(vec2 b, vec2 e) {
    vec2 b_polar = polar_of_cartesian(b);
    vec2 logb = vec2(log(b_polar.x), b_polar.y);
    vec2 new_e = cmul(logb, e);
    vec2 ans_polar = vec2(exp(new_e.x), new_e.y);
    return cartesian_of_polar(ans_polar);
}

// cexp, specialized to e = vec2(-2., 0.)
vec2 cexp_m2(vec2 b) {
    vec2 b_invmag = b / vec2(dot(b, b));
    return vec2((b_invmag.x*b_invmag.x - b_invmag.y*b_invmag.y), -2.*b_invmag.x*b_invmag.y);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // Normalized pixel coordinates (from -ZOOM to ZOOM)
    vec2 uv = vec2(2. * float(ZOOM)) * (fragCoord - iResolution.xy/vec2(2.)) / vec2(max(iResolution.x, iResolution.y));

    // Animate the constant c
    float t = iTime/float(TIME_SCALE);
    vec2 c = (vec2(cos(t)*abs(cos(t)), sin(t)*abs(sin(t)))) * vec2(0.7665);

    // Computation of the Julia set defined by the iteration x -> x^-2 + c
    vec2 x = uv;
    int iter = 0;
    for (iter = 0; iter < ITER_MAX; ++iter) {
        
        if (dot(x, x) > 40.) break;
#ifdef CUSTOM_EXPONENT
            x = cexp(x, CUSTOM_EXPONENT);
#else
            x = cexp_m2(x);
#endif
        x += c;
    }
    vec3 col = vec3(float(iter) / float(ITER_MAX)) * vec3(c, 1.0);

    // Output to screen
    fragColor = vec4(col,1.0);
}
