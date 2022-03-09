#version 330
precision highp float;


uniform sampler2DRect tex0;
uniform sampler2DRect tex1;
uniform vec2 mouse;
uniform vec2 resolution;

in vec2 texCoordVarying;


out vec4 outputColor;

float sum(vec3 c) {
    return c.x+c.y+c.z;
}

void main()
{
	vec2 st = gl_FragCoord.xy;
	// get texture colour
	vec3 color =  texture(tex0, st).rgb;// * vec4(1., 0., 0. ,1.);


  vec3 c_u = texture(tex1, st+vec2(0, 1)).rgb;
  vec3 c_d = texture(tex1, st+vec2(0, -1)).rgb;
  vec3 c_r = texture(tex1, st+vec2(1, 0)).rgb;
  vec3 c_l = texture(tex1, st+vec2(-1, 0)).rgb;
  color += (c_u + c_d + c_r + c_l) * 0.239;
  c_u = texture(tex1, st+vec2(0, 2)).rgb;
  c_d = texture(tex1, st+vec2(0, -2)).rgb;
  c_r = texture(tex1, st+vec2(2, 0)).rgb;
  c_l = texture(tex1, st+vec2(-2, 0)).rgb;
  color += (c_u + c_d + c_r + c_l) * 0.0105;

  float c_sum = sum(color);

  float alpha = float(c_sum > 0.0) * 0.995 + 0.005;
  // avoid blowing out colors
  color *= smoothstep(3.0, 2.7, c_sum) * 0.07 + 0.93;
  // float alpha = 1.0;
  // color = vec3(st.xy, 1.0);

  outputColor = vec4(color, alpha);
}
