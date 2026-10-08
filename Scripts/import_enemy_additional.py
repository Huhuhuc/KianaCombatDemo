import unreal
import json
from pathlib import Path

root = Path('E:/mihoyou/KianaCombatDemo')
dest = '/Game/CombatDemo/Characters/StrikeJaeger/Animations'
sk = unreal.load_asset('/Game/CombatDemo/Characters/StrikeJaeger/Meshes/Monster_StrikeJaeger_Model_RootBone_Skeleton')
assert sk
names = ['Attack04', 'Attack05', 'Attack06', 'Dodge', 'Dodge_L', 'Dodge_R', 'Hit_H_Back', 'Hit_L_Back_Up', 'Walk_B', 'Walk_L', 'Walk_R']
unreal.SystemLibrary.execute_console_command(None, 'Interchange.FeatureFlags.Import.FBX 0')
report = []
for suffix in names:
    name = 'Monster_StrikeJaeger_Ani_' + suffix
    path = dest + '/' + name
    asset = unreal.load_asset(path)
    if not asset:
        opts = unreal.FbxImportUI()
        opts.set_editor_property('automated_import_should_detect_type', False)
        opts.set_editor_property('mesh_type_to_import', unreal.FBXImportType.FBXIT_ANIMATION)
        opts.set_editor_property('import_mesh', False)
        opts.set_editor_property('import_animations', True)
        opts.set_editor_property('skeleton', sk)
        opts.get_editor_property('anim_sequence_import_data').set_editor_property('snap_to_closest_frame_boundary', True)
        task = unreal.AssetImportTask()
        task.filename = str(root / 'SourceAssets/StrikeJaeger/Animation_RootBone' / (name + '.fbx'))
        task.destination_path = dest
        task.destination_name = name
        task.automated = True
        task.save = True
        task.replace_existing = False
        task.options = opts
        task.factory = unreal.FbxFactory()
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        asset = unreal.load_asset(path)
    assert isinstance(asset, unreal.AnimSequence), path
    assert asset.get_editor_property('skeleton') == sk, path
    duration = asset.get_editor_property('sequence_length')
    assert duration > 0, path
    asset.set_editor_property('enable_root_motion', True)
    asset.set_editor_property('force_root_lock', True)
    asset.set_editor_property('root_motion_root_lock', unreal.RootMotionRootLock.REF_POSE)
    assert unreal.EditorAssetLibrary.save_loaded_asset(asset, False), path
    report.append(dict(asset=path, seconds=duration, skeleton=sk.get_path_name()))
    print('ENEMY_IMPORT_VERIFIED', name, duration)
(root / 'Saved/EnemyAdditionalImport.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print('ENEMY_IMPORT_COMPLETE', len(report))
