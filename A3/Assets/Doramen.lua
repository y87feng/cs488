root = gr.node('root')

blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)

-- face
function drawFace()
        face_j = gr.joint('p_face', {-30, 0, 30}, {0, 0, 0} )
        left_eye_j = gr.joint('p_left_eye', {-30, 0, 30}, {0, 0, 0} )
        right_eye_j = gr.joint('p_right_eye', {-30, 0, 30}, {0, 0, 0} )

        left_eye = gr.mesh('sphere', 'left_eye')
        right_eye = gr.mesh('sphere', 'right_eye')
        left_pupil = gr.mesh('cube', 'left_pupil')
        right_pupil = gr.mesh('cube', 'right_pupil')

        left_eye:scale(0.35, 0.4, 0.1)
        right_eye:scale(0.35, 0.4, 0.1)
        left_eye:set_material(white)
        right_eye:set_material(white)

        left_pupil:scale(0.2, 0.05, 0.2)
        right_pupil:scale(0.2, 0.05, 0.2)
        left_pupil:rotate('z', 30.0)
        right_pupil:rotate('z', -30.0)
        right_pupil:translate(0.0, -0.15, 0.0)
        left_pupil:translate(0.0, -0.15, 0.0)
        left_pupil:set_material(black)
        right_pupil:set_material(black)

        left_eye_j:add_child(left_eye)
        left_eye_j:add_child(left_pupil)
        right_eye_j:add_child(right_eye)
        right_eye_j:add_child(right_pupil)

        left_eye_j:translate(-0.32, 0.0, 0.0)
        left_eye_j:rotate('x', -10)
        right_eye_j:translate(0.32, 0.0, 0.0)
        right_eye_j:rotate('x', -10)
        face_j:add_child(left_eye_j)
        face_j:add_child(right_eye_j)

        face_j:translate(2.0, 2.9, 1.75)

        return face_j
end

-- head
function drawHead()
        neck_j = gr.joint('p_neck', {-30, 0, 30}, {0, 0, 0} )
        neck = gr.mesh('sphere', 'neck')
        neck_j:add_child(neck)
        bell = gr.mesh('sphere', 'bell')
        neck_j:add_child(bell)

        neck:set_material(red)
        neck:scale(0.8, 0.2, 1)
        bell:set_material(yellow)
        bell:scale(0.15, 0.15, 0.15)
        bell:translate(0.0, 0.0, 1.0)

        neck_j:translate(2.0, 0.8, 0.0)

        head_j = gr.joint('p_head', {-10, 0, 10}, {0, 0, 0} )
        head = gr.mesh('sphere', 'head')
        head:set_material(blue)
        head:scale(2.0, 1.8, 1.8)
        head:translate(2.0, 2.5, 0.0)

        head_j:add_child(head)
        head_j:add_child(neck_j)
        head_j:add_child(drawFace())

        return head_j
end

root:add_child(drawHead())

-- torso
function drawTorso()
        torso = gr.mesh('sphere', 'torso')
        torso:scale(1.5, 1.5, 1.5)
        torso:set_material(blue)
        torso:translate(2.0, -0.5, 0.0)

        return torso
end

root:add_child(drawTorso())

-- arms and legs

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

function drawRightArm()
        right_shoulder_j = gr.joint('p_right_shoulder', {-30, 0, 30}, {0, 0, 0} )
        right_elbow_j = gr.joint('p_right_elbow', {-30, 0, 30}, {0, 0, 0} )
        right_wrist_j = gr.joint('p_right_wrist', {-30, 0, 30}, {0, 0, 0} )

-- right arm set up
        right_upper_arm = gr.mesh('sphere', 'right_upper_arm')
        right_hand = gr.mesh('sphere', 'right_hand')
        right_forearm = gr.mesh('sphere', 'right_forearm')

-- add mesh to joint
        right_wrist_j:add_child(right_hand)
        right_elbow_j:add_child(right_forearm)
        right_shoulder_j:add_child(right_upper_arm)
-- add lower joint to upper joint
        right_elbow_j:add_child(right_wrist_j)
        right_shoulder_j:add_child(right_elbow_j)

        right_upper_arm:scale(0.3, 0.6, 0.3)
        right_forearm:scale(0.3, 0.5, 0.3)
        right_hand:scale(0.3, 0.25, 0.25)

        right_shoulder_j:rotate('z', 45.0)
        right_shoulder_j:rotate('x', -30.0)

        right_elbow_j:translate(0.0, -0.6, 0.0)
        right_hand:translate(0.0, -0.5, 0.0)

        right_hand:set_material(white)
        right_forearm:set_material(blue)
        right_upper_arm:set_material(blue)

        right_shoulder_j:translate(3.3, 0.2, 0.2)

        return right_shoulder_j
end

root:add_child(drawRightArm())

function drawLeftLeg()
        left_hip_j = gr.joint('p_hip', {-30, 0, 30}, {0, 0, 0} )
        left_knee_j = gr.joint('p_left_leg', {-30, 0, 30}, {0, 0, 0} )
        left_ankle_j = gr.joint('p_left_ankle', {-30, 0, 30}, {0, 0, 0} )

-- left arm set up
        left_thigh = gr.mesh('sphere', 'left_thigh')
        left_foot = gr.mesh('sphere', 'left_foot')
        left_calf = gr.mesh('sphere', 'left_calf')

        left_ankle_j:add_child(left_foot)
        left_knee_j:add_child(left_calf)
        left_hip_j:add_child(left_thigh)

        left_knee_j:add_child(left_ankle_j)
        left_hip_j:add_child(left_knee_j)

-- adjust arm length
        left_thigh:scale(0.5, 0.4, 0.5)
        left_calf:scale(0.45, 0.35, 0.45)
        left_foot:scale(0.5, 0.3, 0.5)

-- arm position corresponding to shoulder
        left_knee_j:translate(0.0, -0.3, 0.0)
        left_foot:translate(0.0, -0.35, 0.0)

-- colour
        left_foot:set_material(white)
        left_calf:set_material(blue)
        left_thigh:set_material(blue)

-- final pos adjust
        left_hip_j:translate(1.35, -1.8, 0.0)

        return left_hip_j
end

function drawRightLeg()
        right_hip_j = gr.joint('p_hip', {-30, 0, 30}, {0, 0, 0} )
        right_knee_j = gr.joint('p_right_leg', {-30, 0, 30}, {0, 0, 0} )
        right_ankle_j = gr.joint('p_right_ankle', {-30, 0, 30}, {0, 0, 0} )

-- right arm set up
        right_thigh = gr.mesh('sphere', 'right_thigh')
        right_foot = gr.mesh('sphere', 'right_foot')
        right_calf = gr.mesh('sphere', 'right_calf')

        right_ankle_j:add_child(right_foot)
        right_knee_j:add_child(right_calf)
        right_hip_j:add_child(right_thigh)

        right_knee_j:add_child(right_ankle_j)
        right_hip_j:add_child(right_knee_j)

-- adjust arm length
        right_thigh:scale(0.5, 0.4, 0.5)
        right_calf:scale(0.45, 0.35, 0.45)
        right_foot:scale(0.5, 0.3, 0.5)

-- arm position corresponding to shoulder
        right_knee_j:translate(0.0, -0.3, 0.0)
        right_foot:translate(0.0, -0.35, 0.0)

-- colour
        right_foot:set_material(white)
        right_calf:set_material(blue)
        right_thigh:set_material(blue)

        right_hip_j:translate(2.65, -1.8, 0.0)

        return right_hip_j

end

root:add_child(drawLeftLeg())
root:add_child(drawRightLeg())


root:translate(-2.75, 0.0, -10.0)

return root
