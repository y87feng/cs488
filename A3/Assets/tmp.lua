rightInnerEar = gr.mesh('sphere', 'rightInnerEar')
rightInnerEar:scale(0.8, 0.8, 1)
rightInnerEar:translate(0, 0, 0.2)
rightInnerEar:set_material(darkbrown)

rightEar:add_child(rightInnerEar)