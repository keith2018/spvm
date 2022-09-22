
// License - CC0 or use as you wish

vec3 colorMinutes=vec3(.98222,.98222,.98222);
vec3 colorHours=vec3(.98222,.98222,.98222);
vec3 colorTextM=vec3(.98222,.98222,.98222);
vec3 colorTextH=vec3(.98222,.98222,.98222);
vec3 colorTri=vec3(.98222,.98222,.98222)/2.5;

//https://www.shadertoy.com/view/Xsy3zG
float segment(vec2 uv)
{
    uv = abs(uv);
    return (1.0-smoothstep(-0.257,0.40,uv.x))
    * (1.0-smoothstep(0.25,0.66,uv.y));
}

float sevenSegment(vec2 uv,int num)
{
    float seg= 0.0;
    if (num>=2 && num!=7 || num==-2)
    seg = max(seg,segment(uv.yx));
    if (num==0 ||
    (uv.y<0.?((num==2)==(uv.x<0.) || num==6 || num==8):
    (uv.x>0.?(num!=5 && num!=6):(num>=4 && num!=7) )))
    seg = max(seg,segment(abs(uv)-0.5));
    if (num>=0 && num!=1 && num!=4 && (num!=7 || uv.y>0.))
    seg = max(seg,segment(vec2(abs(uv.y)-1.0,uv.x)));
    return seg;
}

float showNum(vec2 uv,float nr)
{
    float digit = floor(-uv.x / 1.5);
    nr /= pow(10.,digit);
    nr = mod(floor(nr+0.000001),10.0);
    if (uv.x<-2.90 )
    return 0.;
    return sevenSegment(uv+vec2( 0.75 + digit*1.5,0.0),int(nr));
}

float drawminutes(vec2 uv){
    float t;
    t=floor(mod((uv.x-0.0035)/.01,12.)/2.)*10.+10.;
    uv.y+=0.0652;
    uv.y/=0.25;
    uv.x+=0.0573;
    uv/=2.25;
    uv.x=-uv.x;
    return showNum(vec2(-mod(uv.x/.01,2./2.25)/0.15,uv.y*80.),t);
}

float drawhours(vec2 uv){
    float t;
    uv.y-=0.12852;
    uv.y/=0.221;
    uv.x+=0.0313024;
    uv/=2.;
    t=(floor(mod((uv.x-0.0035)/.01,144.)/6.)*10.)/10.;
    uv/=2.85;
    uv.x=-uv.x;
    return showNum(vec2(-mod((uv.x)/(.01),2./2.85)/(0.15),uv.y*80.),t);
}

    //https://www.shadertoy.com/view/XlBBWt
    #define PI 3.14159265359
    #define TWO_PI 6.28318530718
float drawtriangle(vec2 uv){
    uv+=0.5;
    uv.y-=0.18;
    uv = uv *2.-1.;
    uv.x/=1.7;
    uv*=20.;
    int N = 3;
    float a = atan(uv.x,uv.y);
    float r = TWO_PI/float(N);
    return 1.0-smoothstep(.4,.541,cos(floor(.5+a/r)*r-a)*length(uv));
}

float moveto(){
    float timeval=iDate.w;
    //timeval=iMouse.x/iResolution.x*86400.;//test1
    //timeval=iTime*70.;//test2
    //timeval=3600.*5.;//test3 5 hours
    float h = floor(timeval/3600.);
    float m = floor(mod(timeval/60.,60.));
    float s = floor(mod(timeval,60.));
    float size=1./50.;//<screen res>/<num of 10min marks>
    return h*6.*size+(m/60.)*size*6.+((s/60.)/10.)*size;
}

//https://iquilezles.org/articles/filterableprocedurals
float filteredGrid( in vec2 p, in vec2 dpdx, in vec2 dpdy )
{
    const float N = 8.0;
    vec2 w = max(abs(dpdx), abs(dpdy));
    vec2 a = p + 0.5*w;
    vec2 b = p - 0.5*w;
    vec2 i = (floor(a)+min(fract(a)*N,1.0)-
    floor(b)-min(fract(b)*N,1.0))/(N*w);
    return (1.0-i.x);//*(1.0-i.y);
}

void mainImage( out vec4 fragColor, vec2  fragCoord)
{
    vec2 uv = ((fragCoord-.5*iResolution.xy) / iResolution.y)*vec2(9./16.,1.);//vec2 uv = fragCoord/iResolution.xy-0.5;
    uv*=0.75;
    float px=2./iResolution.y; //pixel size for AA in smoothstep
    float gradient=smoothstep(0.31,0.,abs(uv.x));//gradient to black on left right
    float tri=drawtriangle(uv);//triangle
    uv.x+=24.*6.*(1./50.)+px*0.5; //to avoid negative in mod and fract
    uv.x+=moveto();
    float hline1=1.-smoothstep(0.001,0.001+px,abs(uv.y)); //horizontal line
    float hline2=1.-smoothstep(0.05,0.05+px,abs(uv.y)); //mins horizontal
    float hline3=1.-smoothstep(0.1,0.1+px,abs(uv.y)); //hours horizontal

    vec2 tuv=(uv+px*0.5)*50.;
    float hdiv=1.-step(.85,abs(mod(tuv.x+3.,6.)-3.)); //hours
    float hdiv2=1.-step(.5,abs(mod(tuv.x+3.,6.)-3.)); //mins
    hline3*=hdiv;
    float vline1_aa=1.-filteredGrid(tuv,dFdx(tuv),dFdy(tuv));

    vline1_aa=max(vline1_aa,hline1);
    vline1_aa*=max(hline2,hline3);
    float dmins=drawminutes(uv)*(1.-hdiv2);
    dmins=smoothstep(0.01,0.5,dmins);
    float dhours=drawhours(uv)*hdiv;
    dhours=smoothstep(0.1,0.5,dhours);
    fragColor = vec4(colorMinutes*vline1_aa,1.);
    fragColor += vec4(tri*colorTri,1.);
    fragColor += vec4(dmins*colorTextM,1.);
    fragColor+=vec4(dhours*colorTextH,1.);
    fragColor.w=max(max(vline1_aa,dmins),max(dhours,tri))*gradient;
    fragColor.w=clamp(fragColor.w,0.,1.);

    fragColor.rgb*=gradient;

    //fragColor.rgb=fragColor.rgb*fragColor.w+texture(iChannel0,fragCoord/iResolution.xy).rgb*(1.-fragColor.w);



}










