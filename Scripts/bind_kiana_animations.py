"""Run after compiling the editor target. Bind all 21 compatible clips."""
import unreal
import json
from pathlib import Path

base = '/Game/CombatDemo/Characters/Kiana/Animations/'
mapping = {
    'as_idle':'Kiana_Idle', 'as_idle_alt':'Additional/Kiana_Idle_1',
    'as_walk':'Kiana_Walk', 'as_run':'Kiana_Run',
    'as_run_start':'Additional/Kiana_RunStart',
    'as_run_stop_left':'Additional/Kiana_RunStopLeft',
    'as_run_stop_right':'Additional/Kiana_RunStopRight',
    'as_attack1':'Kiana_Attack1', 'as_attack2':'Kiana_Attack2',
    'as_attack3':'Kiana_Attack3', 'as_attack4':'Kiana_Attack4',
    'as_attack5':'Kiana_Attack5',
    'as_attack3_branch':'Kiana_Attack3_fenzhi',
    'as_attack4_branch':'Kiana_Attack4_fenzhi',
    'as_attack_qte':'Additional/Kiana_Attack_QTE',
    'as_evade_front':'Kiana_EvadeFront', 'as_evade_back':'Kiana_EvadeBack',
    'as_skill':'Kiana_Skill', 'as_hit_h':'Kiana_Hit_H', 'as_hit_l':'Kiana_Hit_L',
    'as_die':'Additional/Kiana_Die',
}
bp = unreal.load_asset('/Game/CombatDemo/Blueprints/Player/BP_KianaCharacter_Cpp')
cdo = unreal.get_default_object(bp.generated_class())
for prop, name in mapping.items():
    asset = unreal.load_asset(base+name)
    assert isinstance(asset,unreal.AnimSequence), name
    asset.set_editor_property('enable_root_motion', True)
    asset.set_editor_property('force_root_lock', True)
    asset.set_editor_property('root_motion_root_lock',unreal.RootMotionRootLock.REF_POSE)
    assert unreal.EditorAssetLibrary.save_loaded_asset(asset,False)
    cdo.set_editor_property(prop,asset)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
cdo = unreal.get_default_object(bp.generated_class())
for prop,name in mapping.items():
    assert cdo.get_editor_property(prop).get_path_name().split('.')[0] == base+name, prop
Path('E:/mihoyou/KianaCombatDemo/Saved/KianaAnimationBindings.json').write_text(json.dumps(mapping,indent=2),encoding='utf-8')
print('BOUND_AND_VERIFIED',len(mapping))
