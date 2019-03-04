-- Simple Scene:
-- An extremely simple scene that will render right out of the box
-- with the provided skeleton code.  It doesn't rely on hierarchical
-- transformations.

-- Create the top level root node named 'root'.
rootNode = gr.node('root')

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

-- Create a GeometryNode with MeshId = 'cube', and name = 'torso'.
-- MeshId's must reference a .obj file that has been previously loaded into
-- the MeshConsolidator instance within A3::init().
torso = gr.mesh('cube', 'torso')
torso:scale(1.0, 1.0, 1.0)
torso:rotate('y', 45.0)
torso:translate(0.0, -0.5, -5.0)
torso:set_material(gr.material({0.8, 0.2, 0.2}, {0.8, 0.8, 0.8}, 10.0))

-- Add the cubeMesh GeometryNode to the child list of rootnode.
rootNode:add_child(torso)

neck = gr.mesh('sphere', 'neck')
torso:add_child(neck)
neck:scale(0.15, 0.3, 0.15)
neck:translate(0.0, 0.6, 0.0)
neck:set_material(blue)

neckNode = gr.node('neckNode')
neck:add_child(neckNode)
neckNode:scale(1.0/0.15, 1.0/0.3, 1.0/0.15) -- now scale(1,1,1)

neckJoint = gr.joint('neckJoint', {-10, 0, 90}, {-110, 0, 110});
neckNode:add_child(neckJoint)

-- Create a GeometryNode with MeshId = 'sphere', and name = 'head'.
head = gr.mesh('suzanne', 'head')
head:scale(0.5, 0.5, 0.5)
head:rotate('y', -45.0)
head:translate(0.0, 0.5, 0.0)
head:set_material(gr.material({0.4, 0.8, 0.4}, {0.8, 0.8, 0.8}, 50.0))

-- Add the sphereMesh GeometryNode to the child list of rootnode.
neckJoint:add_child(head)

LeftEarNode = gr.node('LeftEarNode')
head:add_child(LeftEarNode)
LeftEarNode:scale(1.0/0.5, 1.0/0.5, 1.0/0.5) -- now scale(1,1,1)

LeftEarJoint = gr.joint('LeftEarJoint', {-10, 0, 90}, {-110, 0, 110});
LeftEarNode:add_child(LeftEarJoint)

LeftEar = gr.mesh('sphere', 'LeftEar')
LeftEar:scale(0.1, 0.1, 0.01)
-- LeftEar:rotate('y', -45.0)
LeftEar:translate(-0.25, 0.5, 0.0)
LeftEar:set_material(red)

LeftEarJoint:add_child(LeftEar)
-- Return the root with all of it's childern.  The SceneNode A3::m_rootNode will be set
-- equal to the return value from this Lua script.
return rootNode
