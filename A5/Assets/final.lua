-- A simple scene with some miscellaneous geometry.
-- This file is very similar to nonhier.lua, but interposes
-- an additional transformation on the root node.  
-- The translation moves the scene, and the position of the camera
-- and lights have been modified accordingly.

mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25, 0.0, 0.0, 1.0)
mat2 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25)
mat4 = gr.material({0.7, 0.6, 1.0}, {0.5, 0.4, 0.8}, 25)
blue = gr.material({0.0, 0.25, 0.53}, {0.0, 0.25, 0.53}, 25, 0.4, 0.0, 1.0)
gold = gr.material({0.93, 0.8, 0.38}, {0.91, 0.78, 0.51}, 25)
red = gr.material({0.89, 0.21, 0.22}, {0.89, 0.21, 0.22}, 25)


scene = gr.node( 'scene' )
scene:translate(0, 0, -800)

noon = gr.nh_sphere('noon', {-400, 0, -500}, 100)
scene:add_child(noon)
noon:set_material(mat1)

sun = gr.nh_sphere('sun', {150, 150, -2000}, 400)
scene:add_child(sun)
sun:set_material(red)

earth = gr.nh_sphere('earth', {0, -1200, -500}, 1000)
scene:add_child(earth)
earth:set_material(blue)

-- b1 = gr.nh_box('b1', {-200, -125, 0}, 100)
-- scene:add_child(b1)
-- b1:set_material(mat4)

-- s4 = gr.nh_sphere('s4', {0, 25, -300}, 50)
-- scene:add_child(s4)
-- s4:set_material(mat3)

comet = gr.nh_sphere_mb('comet', {-250, 100, -550}, 40, {-50,50,0})
scene:add_child(comet)
comet:set_material(mat3)

-- s6 = gr.nh_triprism('s6', {-200, 0, 0}, 100, 200)
-- scene:add_child(s6)
-- s6:set_material(mat1)

pyramid1 = gr.nh_tripyramid('pyramid1', {100, -350, -50}, 200)
scene:add_child(pyramid1)
pyramid1:rotate('Z',-5)
pyramid1:set_material(gold)

pyramid2 = gr.nh_tripyramid('pyramid2', {10, -300, -125}, 170)
scene:add_child(pyramid2)
pyramid2:rotate('Z',-5)
pyramid2:set_material(gold)

pyramid3 = gr.nh_tripyramid('pyramid3', {-100, -300, -175}, 150)
scene:add_child(pyramid3)
pyramid3:rotate('Z',-5)
pyramid3:set_material(gold)

-- A small stellated dodecahedron.

steldodec = gr.mesh( 'dodec', 'smstdodeca.obj' )
steldodec:set_material(mat3)
scene:add_child(steldodec)
steldodec:scale(0.8,0.8,0.8)
steldodec:translate(400, -150, 0)

white_light = gr.light({-100.0, 150.0, -400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
sun_light = gr.light({150.0, 150.0, -1000.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
magenta_light = gr.light({400.0, 100.0, -650.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

gr.render(scene, 'final.png', 512, 512,
	  {0, 0, 0}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light, sun_light})
