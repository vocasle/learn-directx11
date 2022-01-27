# Topics to cover

- [X] Initialize D3D11 context

- [X] Process input and camera control

- [ ] Load simple model without animation (teapot, statue, car, plane, tank etc.)

- [ ] Load mip-mapped textures (diffuse, normal, specular, gloss)

- [ ] Calculate Phong lighting from direct and point lights in the pixel shader

- [ ] Particle System (10000+ particles)

- [ ] Make soft shadows (SM+PCF, VSM or ESM)

- [ ] Make dynamic environment reflections via render to cube map

- [ ] Add fog effect (pixel shader or post-process)

- [ ] Add bloom post-process effect

# Notes

## Lighting
In *local illumination model* we calculate lighting for an object as if there is no
other objects. Thus light ray from light source does not bounces from any objects
except the one we calculate lighting for.

### Calculating Phong lighting

#### Calculating Diffuse lighting
We assume that light scatters equally in all directions above the surface.
Thus we can ignore the eye position.

We calculate diffuse lighting by first multiplying component wise diffuse light
color and diffuce material color:

	D = l_d ⊗ m_d;

Example:

	D = l_d ⊗ m_d = (0.8,0.8,0.8) ⊗ (0.5,1.0,0.75) = (0.4,0.8,0.6)

Here ⊗ denotes component wise vector multiplication.

After that we include Lambert's cosine law:

	f(θ) = max(cos θ, 0) = max (L * n, 0)

where **L** is a unit vector in direction of light vector (light vector is the vector *from*
the surface to the light source) and **n** is the normal vector.

Let **l_d** be the diffuse light color, **m_d** be the diffuse material color, and **k_d** = f(θ).
Then the amout of diffuse light reflected off a point is given by:

	c_d = k_d * l_d ⊗ m_d = k_d * D

#### Calculating the Ambient lighting
Ambient lighting is an emulation of light bouncing from other objects.
Let **l_a** be the ambient light color, **m_a** be the ambient material color, then the
amount of ambient light reflected off a point is given by:

	A = l_a ⊗ m_a

#### Calculating the Specular lighting

	c_s = k_s * l_s ⊗ m_s = k_s * S

where *l_s* is the specular light color, *m_s* is the specular material component, and

	k_s = max (v*r, 0)^p, when L * n > 0, or k_s = 0, when L * n <= 0.

Here **L** is the unit light vector, **n** is the normal, k_s - coeficient that depends on
exponent *p*. The more the *p* is the more sharply the light reflects of the surface.

**v** is the unit vector that is given by

	v = E - P / || E - P ||,

where **E** is the position of eye and **P** is the surface point.

#### Phong lighting formula

	LitColor = l_a ⊗ m_a + k_d * l_d ⊗ m_d + k_s * l_s ⊗ m_s = A + k_d * D + k_s * S
