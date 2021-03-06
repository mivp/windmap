#version 120

uniform sampler2D u_wind;
uniform vec2 u_wind_min;
uniform vec2 u_wind_max;
uniform sampler2D u_color_ramp;

varying vec2 v_particle_pos;

void main() {
    vec2 velocity = mix(u_wind_min, u_wind_max, texture2D(u_wind, v_particle_pos).rg);
    float speed_t = length(velocity) / length(u_wind_max);
    speed_t = clamp(speed_t, 0.001, 0.999);

    // color ramp is encoded in a 16x16 texture
    //vec2 ramp_pos = vec2( fract(16.0 * speed_t), floor(16.0 * speed_t) / 16.0);
    vec2 ramp_pos = vec2( speed_t, 0.5);

    gl_FragColor = texture2D(u_color_ramp, ramp_pos);

    //if(speed_t < 0.05)
    //	gl_FragColor[3] = 0.5;
}
