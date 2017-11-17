#version 120

attribute float a_index;

uniform sampler2D u_particles;
uniform float u_particles_res;

uniform vec3 u_corner;
uniform vec2 u_size;
uniform mat4 u_mv;
uniform mat4 u_p;

varying vec2 v_particle_pos;

void main() {
    vec4 color = texture2D(u_particles, vec2(
        fract(a_index / u_particles_res),
        floor(a_index / u_particles_res) / u_particles_res));

    // decode current particle position from the pixel's RGBA value
    v_particle_pos = vec2(color.r, color.g);

    //vec3 pos = u_topleft + vec3(v_particle_pos[0]*u_size[0], 0, v_particle_pos[1]*u_size[1]);
    vec3 pos = u_corner + vec3(color.r*u_size[0], 0, color.g*u_size[1]);
    gl_Position = u_p * u_mv * vec4(pos, 1);
    
    vec3 mvPosition = (u_mv * vec4(pos,1.0)).xyz;
    float projFactor = 2.41;
    projFactor /= length(mvPosition);
    projFactor *= 1500; //uScreenHeight / 2.0;
    float pointSize = 40 * projFactor; //uPointScale * projFactor;
    pointSize = max(pointSize, 3);
    gl_PointSize = pointSize;
}
