"""Run in editor; validate evaluated stop poses against the original rig."""
import unreal
import math
import json
from pathlib import Path

base = '/Game/CombatDemo/Characters/Kiana/Animations/'
sk = unreal.load_asset('/Game/CombatDemo/Characters/Kiana/Meshes/Avatar_Kiana_C2_Model_RootBone_Skeleton')
opts = unreal.AnimPoseEvaluationOptions()
rows = []
for path in unreal.EditorAssetLibrary.list_assets(base, recursive=True):
    asset = unreal.load_asset(path)
    if not isinstance(asset, unreal.AnimSequence):
        continue
    assert asset.get_editor_property('skeleton') == sk, path
    assert asset.get_play_length() > 0 and len(asset.data_model_interface.get_bone_track_names()) > 0, path
    for frame in range(asset.data_model_interface.get_number_of_frames() + 1):
        pose = asset.get_anim_pose_at_frame(frame, opts)
        for bone in pose.get_bone_names():
            t = pose.get_bone_pose(bone, unreal.AnimPoseSpaces.WORLD)
            assert all(math.isfinite(v) for v in (t.translation.x,t.translation.y,t.translation.z,t.rotation.x,t.rotation.y,t.rotation.z,t.rotation.w,t.scale3d.x,t.scale3d.y,t.scale3d.z)), (path,frame,str(bone))
    rows.append(dict(name=asset.get_name(),seconds=asset.get_play_length(),frames=asset.data_model_interface.get_number_of_frames()))

comparisons = []
anchor = sk.get_reference_pose().get_bone_pose('Bip001',unreal.AnimPoseSpaces.WORLD).translation
for side in ('Left','Right'):
    source = unreal.load_asset('/Game/CombatDemo/ImportSupport/KianaOriginal/Avatar_Kiana_C2_Ani_RunStop'+side+'_fix')
    target = unreal.load_asset(base+'Additional/Kiana_RunStop'+side)
    max_error = 0.0
    for frame in range(source.data_model_interface.get_number_of_frames()+1):
        a = source.get_anim_pose_at_frame(frame,opts)
        b = target.get_anim_pose_at_frame(frame,opts)
        hips = a.get_bone_pose('Bip001',unreal.AnimPoseSpaces.WORLD).translation
        offset = unreal.Vector(hips.x-anchor.x,hips.y-anchor.y,0)
        for bone in a.get_bone_names():
            dest = str(bone).replace('(mirrored)','_mirrored_')
            p = a.get_bone_pose(bone,unreal.AnimPoseSpaces.WORLD).translation-offset
            q = b.get_bone_pose(dest,unreal.AnimPoseSpaces.WORLD).translation
            max_error = max(max_error,(p-q).length())
    assert max_error < 0.5, (side,max_error)
    comparisons.append(dict(side=side,max_joint_error_cm=max_error))
assert len(rows)==21,len(rows)
report=dict(assets=rows,stop_pose_comparison=comparisons,passed=True)
Path('E:/mihoyou/KianaCombatDemo/Saved/KianaAssetValidation.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report))
