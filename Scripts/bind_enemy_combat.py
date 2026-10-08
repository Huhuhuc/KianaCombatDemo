import unreal
import json
from pathlib import Path

base = '/Game/CombatDemo/Characters/StrikeJaeger/Animations/Monster_StrikeJaeger_Ani_'
mapping = dict(as_idle='Idle', as_walk='Walk_F', as_run_start='Run_Start', as_run='Run_Loop', as_run_end='Run_End',
               as_walk_back='Walk_B', as_walk_left='Walk_L', as_walk_right='Walk_R',
               as_dodge='Dodge', as_dodge_left='Dodge_L', as_dodge_right='Dodge_R',
               as_hit_h='Hit_H_Front', as_hit_l='Hit_L_Front_Up', as_hit_back_light='Hit_L_Back_Up', as_hit_back_heavy='Hit_H_Back',
               as_stun_start='Debuff_Stun_Start', as_stun_loop='Debuff_Stun_Loop', as_stun_end='Debuff_Stun_End', as_death='Death_Stay')
mapping.update({'as_attack%02d' % n: 'Attack%02d' % n for n in range(1, 7)})
bp = unreal.load_asset('/Game/CombatDemo/Blueprints/Enemy/BP_StrikeJaeger_Cpp')
cdo = unreal.get_default_object(bp.generated_class())
sk = unreal.load_asset('/Game/CombatDemo/Characters/StrikeJaeger/Meshes/Monster_StrikeJaeger_Model_RootBone_Skeleton')
report = []
for prop, suffix in mapping.items():
    a = unreal.load_asset(base + suffix)
    assert isinstance(a, unreal.AnimSequence) and a.get_editor_property('skeleton') == sk, suffix
    assert a.get_editor_property('sequence_length') > 0
    a.set_editor_property('enable_root_motion', True)
    a.set_editor_property('force_root_lock', True)
    a.set_editor_property('root_motion_root_lock', unreal.RootMotionRootLock.REF_POSE)
    assert unreal.EditorAssetLibrary.save_loaded_asset(a, False)
    cdo.set_editor_property(prop, a)
    report.append(dict(property=prop, asset=a.get_path_name(), seconds=a.get_editor_property('sequence_length')))
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp, False)
cdo = unreal.get_default_object(bp.generated_class())
for prop, suffix in mapping.items():
    assert cdo.get_editor_property(prop).get_path_name().split('.')[0] == base + suffix
Path('E:/mihoyou/KianaCombatDemo/Saved/EnemyCombatBindings.json').write_text(json.dumps(report, indent=2))
print('ENEMY_BINDINGS_VERIFIED', len(mapping))
