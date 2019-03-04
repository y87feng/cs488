-- Simple Scene:
-- An extremely simple scene that will render right out of the box
-- with the provided skeleton code.  It doesn't rely on hierarchical
-- transformations.

-- Create the top level root node named 'root'.
rootNode = gr.node('root')

-- Create a GeometryNode with MeshId = 'cube', and name = 'torso'.
-- MeshId's must reference a .obj file that has been previously loaded into
-- the MeshConsolidator instance within A3::init().
cubeMesh = gr.mesh('cube', 'torso')
cubeMesh:scale(1.0, 1.0, 1.0)
--cubeMesh:rotate('y', 45.0)
cubeMesh:translate(0.0, 0.0, 0.0)
cubeMesh:set_material(gr.material({0.8, 0.2, 0.2}, {0.8, 0.8, 0.8}, 10.0))

-- Add the cubeMesh GeometryNode to the child list of rootnode.
rootNode:add_child(cubeMesh)

-- ################### Create sholder joints
--~ LeftShoulder = gr.mesh('sphere', 'leftShoulder')
LeftShoulder = gr.joint('leftShoulder',{-180,0,90},{-180,0,90})
--~ LeftShoulder:scale(0.1,0.1,0.1)
LeftShoulder:translate(0.5, 0.0, 0.0)
--~ LeftShoulder:set_material(gr.material({0.0, 0.4, 0.4}, {0.8, 0.8, 0.8}, 50.0))
cubeMesh:add_child(LeftShoulder)

--RightShoulder = gr.mesh('sphere', 'rightShoulder')
RightShoulder = gr.joint('rightShoulder',{-180,0,90},{-180,0,90})
--RightShoulder:scale(0.1,0.1,0.1)
RightShoulder:translate(-0.5, 0.0, 0.0)
--RightShoulder:set_material(gr.material({0.0, 0.4, 0.4}, {0.8, 0.8, 0.8}, 50.0))
cubeMesh:add_child(RightShoulder)

-- ############################### Create neck joint
Neck = gr.joint('neck_joint',{-15,0,15},{-90,0,90})
--Neck:scale(0.1,0.1,0.1)
Neck:translate(0.0,0.5,0.0)
--Neck:set_material(gr.material({0.0,0.4,0.4},{0.8,0.8,0.8},50))
cubeMesh:add_child(Neck)

-- ######################## Create  left arms
ULL = gr.mesh('cube','UpLeftLimb')
ULL:scale(0.1,1.0,0.1)
ULL:translate(0.1,-0.5,0.0)
ULL:set_material(gr.material({0.4,0.4,0.0},{0.8,0.8,0.8},50.0))
LeftShoulder:add_child(ULL)

LELB = gr.mesh('sphere', 'leftElbow')
--~ LEL = gr.joint('leftElbow',{-90,0,90},{-90,0,90})
LELB:scale(1,0.1,1)
LELB:translate(0.0,-0.6,0.0)
LELB:set_material(gr.material({0.0, 0.4, 0.4}, {0.8, 0.8, 0.8}, 50.0))
ULL:add_child(LELB)

--~ LEL = gr.mesh('sphere', 'leftElbow')
LEL = gr.joint('leftElbow_joint',{-90,0,0},{-90,0,0})
LEL:scale(10,1,10)
LEL:translate(0.0,-0.6,0.0)
--~ LEL:set_material(gr.material({0.0, 0.4, 0.4}, {0.8, 0.8, 0.8}, 50.0))
ULL:add_child(LEL)

LLL = gr.mesh('cube','LowerLeftLimb')
LLL:scale(0.1,1.0,0.1)
LLL:translate(0.0,-0.5,0.0)
LLL:set_material(gr.material({0.4,0.4,0.0},{0.8,0.8,0.8},50.0))
LEL:add_child(LLL)

-- ########################## Create  right arms
URL = gr.mesh('cube','UpRightLimb')
URL:scale(0.1,1.0,0.1)
URL:translate(-0.1,-0.5,0.0)
URL:set_material(gr.material({0.4,0.4,0.0},{0.8,0.8,0.8},50.0))
RightShoulder:add_child(URL)

RELB = gr.mesh('sphere', 'rightElbow')
--~ RELB = gr.joint('rightElbow',{-90,0,90},{-90,0,90})
RELB:scale(1.0,0.1,1.0)
RELB:translate(0.0,-0.6,0.0)
RELB:set_material(gr.material({0.0, 0.4, 0.4}, {0.8, 0.8, 0.8}, 50.0))
URL:add_child(RELB)

REL = gr.joint('rightElbow_joint',{-90,30,90},{-90,12,90})
REL:scale(10,1,10)
REL:translate(0.0,-0.6,0.0)
--REL:set_material(gr.material({0.0, 0.4, 0.4}, {0.8, 0.8, 0.8}, 50.0))
URL:add_child(REL)

RLL = gr.mesh('cube','LowerLeftLimb')
RLL:scale(0.1,1.0,0.1)
RLL:translate(0.0,-0.5,0.0)
RLL:set_material(gr.material({0.4,0.4,0.0},{0.8,0.8,0.8},50.0))
REL:add_child(RLL)



-- ############ Create a GeometryNode with MeshId = 'sphere', and name = 'head'.
sphereMesh = gr.mesh('suzanne', 'head')
sphereMesh:scale(0.3, 0.3, 0.3)
sphereMesh:translate(0.0, 0.3, 0.0)
sphereMesh:set_material(gr.material({0.4, 0.8, 0.4}, {0.8, 0.8, 0.8}, 50.0))


-- Add the sphereMesh GeometryNode to the child list of rootnode.
Neck:add_child(sphereMesh)

-- delete later

-- TRI = gr.mesh('tri',"TRI")
-- TRI:scale(0.5,0.5,0.5)
-- TRI:translate(-1.5,-1.5,0.0)
-- TRI:set_material(gr.material({0.4,0.4,0.0},{0.8,0.8,0.8},50.0))
-- rootNode:add_child(TRI)


-- INVTRI = gr.mesh('invtri',"INVTRI")
-- INVTRI:scale(0.5,0.5,0.5)
-- INVTRI:translate(1.5,-1.5,0.0)
-- INVTRI:set_material(gr.material({0.4,0.4,0.0},{0.8,0.8,0.8},50.0))
-- rootNode:add_child(INVTRI)

-- delete later



-- Return the root with all of it's childern.  The SceneNode A3::m_rootNode will be set
-- equal to the return value from this Lua script.

rootNode:translate(0.0,0.0,-5.0)


return rootNode
