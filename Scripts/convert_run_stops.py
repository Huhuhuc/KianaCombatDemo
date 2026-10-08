"""Run inside Unreal Editor: convert original Kiana stop clips to the RootBone rig.
Original assets and the playable skeleton are never modified.
"""
import unreal

source_base = '/Game/CombatDemo/ImportSupport/KianaOriginal/'
target_base = '/Game/CombatDemo/Characters/Kiana/Animations/Additional/'
target_mesh = unreal.load_asset('/Game/CombatDemo/Characters/Kiana/Meshes/Avatar_Kiana_C2_Model_RootBone')
source_mesh = unreal.load_asset(source_base + 'Avatar_Kiana_C2_Model')
target_skeleton = target_mesh.skeleton
target_pose = target_skeleton.get_reference_pose()
source_pose = source_mesh.skeleton.get_reference_pose()
root_ref = target_pose.get_bone_pose('root', unreal.AnimPoseSpaces.LOCAL)
target_names = set(map(str, target_pose.get_bone_names()))

def mapped(name):
    return str(name).replace('(mirrored)', '_mirrored_')

# Ensure this is the same rig with an added root, not an arbitrary retarget.
for name in source_pose.get_bone_names():
    dest = mapped(name)
    assert dest in target_names, (name, dest)
    parent = source_mesh.get_bone_parent(name)
    expected_parent = 'root' if str(parent) == 'None' else mapped(parent)
    assert str(target_mesh.get_bone_parent(dest)) == expected_parent, name
    a = source_pose.get_bone_pose(name, unreal.AnimPoseSpaces.WORLD)
    b = target_pose.get_bone_pose(dest, unreal.AnimPoseSpaces.WORLD)
    assert (a.translation - b.translation).length() < 0.1, name

for side in ('Left', 'Right'):
    source = unreal.load_asset(source_base + 'Avatar_Kiana_C2_Ani_RunStop' + side + '_fix')
    name = 'Kiana_RunStop' + side
    assert source is not None
    asset = unreal.load_asset(target_base + name)
    if asset is None:
        factory = unreal.AnimSequenceFactory()
        factory.set_editor_properties(dict(target_skeleton=target_skeleton, preview_skeletal_mesh=target_mesh))
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, target_base.rstrip('/'), unreal.AnimSequence, factory)
    model = source.data_model_interface
    poses = [source.get_anim_pose_at_frame(i, unreal.AnimPoseEvaluationOptions()) for i in range(model.get_number_of_frames() + 1)]
    controller = asset.controller
    controller.open_bracket('Convert stop clip to Kiana RootBone', False)
    try:
        controller.remove_all_bone_tracks(False)
        controller.set_frame_rate(model.get_frame_rate(), False)
        controller.set_number_of_frames(unreal.FrameNumber(model.get_number_of_frames()), False)
        assert controller.add_bone_curve('root', False)
        assert controller.set_bone_track_keys('root', [root_ref.translation], [root_ref.rotation], [root_ref.scale3d], False)
        # Importer sanitizes track names, but the original skeleton still uses
        # parentheses. Sample actual skeleton names, including reference fallback.
        for bone in source_pose.get_bone_names():
            transforms = [pose.get_bone_pose(bone, unreal.AnimPoseSpaces.LOCAL) for pose in poses]
            positions = [t.translation for t in transforms]
            rotations = [t.rotation for t in transforms]
            scales = [t.scale3d for t in transforms]
            dest = mapped(bone)
            if str(bone) == 'Bip001':
                # Keep pose and vertical motion; CharacterMovement supplies planar braking.
                count = max(len(positions), len(rotations), len(scales))
                anchor = target_pose.get_bone_pose('Bip001', unreal.AnimPoseSpaces.WORLD).translation
                converted = []
                for i in range(count):
                    pos = positions[min(i, len(positions)-1)]
                    t = unreal.Transform(location=unreal.Vector(anchor.x, anchor.y, pos.z), rotation=rotations[min(i, len(rotations)-1)].rotator(), scale=scales[min(i, len(scales)-1)])
                    converted.append(t.make_relative(root_ref))
                positions = [t.translation for t in converted]
                rotations = [t.rotation for t in converted]
                scales = [t.scale3d for t in converted]
            assert controller.add_bone_curve(dest, False), dest
            assert controller.set_bone_track_keys(dest, positions, rotations, scales, False), dest
    finally:
        controller.close_bracket(False)
    asset.set_editor_property('enable_root_motion', True)
    asset.set_editor_property('force_root_lock', True)
    asset.set_editor_property('root_motion_root_lock', unreal.RootMotionRootLock.REF_POSE)
    assert unreal.EditorAssetLibrary.save_loaded_asset(asset, False)
    print('CONVERTED', asset.get_path_name(), asset.sequence_length, len(asset.data_model_interface.get_bone_track_names()))

