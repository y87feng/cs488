root = gr.node('root')

blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)



-- arms and legs

-- torso
function drawTorso()
    torso = gr.mesh('sphere', 'torso')
    torso:scale(1.5, 1.5, 1.5)
    torso:set_material(blue)
    torso:translate(2.0, -0.5, 0.0)

    return torso
end
root:add_child(drawTorso())

function drawLeftArm()
        left_shoulder_j = gr.joint('p_left_shoulder', {-30, 0, 30}, {0, 0, 0} )
        left_elbow_j = gr.joint('p_left_elbow', {-30, 0, 30}, {0, 0, 0} )
        left_wrist_j = gr.joint('p_left_wrist', {-30, 0, 30}, {0, 0, 0} )
        
-- left arm set up
        left_upper_arm = gr.mesh('sphere', 'left_upper_arm')
        left_hand = gr.mesh('sphere', 'left_hand')
        left_forearm = gr.mesh('sphere', 'left_forearm')

-- add mesh to joint
        left_wrist_j:add_child(left_hand)
        left_elbow_j:add_child(left_forearm)
        left_shoulder_j:add_child(left_upper_arm)
-- add lower joint to upper joint
        left_elbow_j:add_child(left_wrist_j)
        left_shoulder_j:add_child(left_elbow_j)

-- adjust arm length
        left_upper_arm:scale(0.3, 0.6, 0.3)
        left_forearm:scale(0.3, 2.0, 0.3)
        left_hand:scale(0.3, 0.25, 0.25)

-- arm inital angle
        left_shoulder_j:rotate('z', -45.0)
        left_shoulder_j:rotate('x', -30.0)

-- arm position corresponding to shoulder
        left_elbow_j:translate(0.0, -0.6, 0.0)
        left_hand:translate(0.0, -0.5, 0.0)
-- colour
        left_hand:set_material(white)
        left_forearm:set_material(blue)
        left_upper_arm:set_material(blue)

-- final position correspoding to world
        left_shoulder_j:translate(0.7, 0.2, 0.2)

        return left_shoulder_j
end
root:add_child(drawLeftArm())
root:translate(-2.75, 0.0, -10.0)
return root;