"""PIE regression using real engine ticks; no persistent level/test stat edits."""
import unreal
import json
import traceback
from pathlib import Path

out = Path('E:/mihoyou/KianaCombatDemo/Saved/EnemyCombatRuntime.json')
out.write_text('{"status":"starting"}')
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level('/Game/CombatDemo/Levels/L_CombatArena')
bp = unreal.load_asset('/Game/CombatDemo/Blueprints/Player/BP_KianaCharacter_Cpp')
cdo = unreal.get_default_object(bp.generated_class())
old_health = cdo.get_editor_property('max_health')
cdo.set_editor_property('max_health', 10000.0)
world = None
checks = []
seen = set()
states = set()
done = set()
samples = []
max_root_offset = 0.0
initial_position = None
last_name = None

def check(name, passed, detail=''):
    checks.append(dict(name=name, passed=bool(passed), detail=str(detail)))

def finish(error=None):
    unreal.unregister_slate_post_tick_callback(handle)
    cdo.set_editor_property('max_health', old_health)
    out.write_text(json.dumps(dict(status='passed' if not error and all(c['passed'] for c in checks) else 'failed',
        checks=checks, sequences=sorted(seen), states=sorted(states), max_root_offset=max_root_offset, samples=samples, error=error), indent=2))
    level.editor_request_end_play()
    print('ENEMY_RUNTIME_FINISHED', str(out))

def once(key, t, when, fn):
    if t >= when and key not in done:
        done.add(key)
        fn()

def hit_from_back(amount):
    player.set_actor_location(enemy.get_actor_location() - enemy.get_actor_forward_vector()*160, False, False)
    enemy.apply_combat_damage(amount, player)
    # Capture immediately after applying damage: a later Slate callback can
    # arrive after a short reaction has already finished on a slow frame.
    active = enemy.get_active_sequence()
    samples.append(dict(t=round(unreal.GameplayStatics.get_time_seconds(world)-start,2),
        animation=active.get_name() if active else 'None', state=str(enemy.get_editor_property('enemy_state'))))

def tick(dt):
    global world, player, enemy, start, initial_position, last_name, max_root_offset, dodge_origin
    try:
        if world is None:
            world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
            if world is None:
                return
            player = unreal.GameplayStatics.get_player_character(world, 0)
            enemy = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.StrikeJaeger)[0]
            player.disable_input(unreal.GameplayStatics.get_player_controller(world, 0))
            enemy.set_editor_property('attack_damage', 0.1)
            start = unreal.GameplayStatics.get_time_seconds(world)
            initial_position = enemy.get_actor_location()
            unreal.SystemLibrary.execute_console_command(world, 't.MaxFPS 60')
            unreal.GameplayStatics.set_global_time_dilation(world, 1.0)
        t = unreal.GameplayStatics.get_time_seconds(world) - start
        a = enemy.get_active_sequence()
        name = a.get_name() if a else 'None'
        state = str(enemy.get_editor_property('enemy_state'))
        seen.add(name)
        states.add(state)
        if name != last_name:
            samples.append(dict(t=round(t,2), animation=name, state=state))
            last_name = name
        mesh = enemy.get_component_by_class(unreal.SkeletalMeshComponent)
        offset = (mesh.get_socket_location('Root') - mesh.get_world_location()).length()
        max_root_offset = max(max_root_offset, offset)
        once('chase',t,3,lambda:check('enemy moves during chase',(enemy.get_actor_location()-initial_position).length()>50))
        once('six',t,66,lambda:check('six attack clips used',all('Monster_StrikeJaeger_Ani_Attack%02d'%n in seen for n in range(1,7))))
        once('skill',t,67,lambda:player.debug_skill())
        once('light',t,69,lambda:enemy.apply_combat_damage(5,player))
        once('stun_recovery',t,78,lambda:check('stun ends after player skill','STUNNED' not in state and all('Monster_StrikeJaeger_Ani_Debuff_Stun_'+p in seen for p in ['Start','Loop','End']),state))
        for direction, at in [(0,79),(1,83),(2,87)]:
            def dodge(d=direction):
                global dodge_origin
                dodge_origin = enemy.get_actor_location()
                enemy.debug_dodge(d)
            once('dodge'+str(direction),t,at,dodge)
            once('dodgecheck'+str(direction),t,at+0.5,lambda d=direction:check('dodge displacement '+str(d),(enemy.get_actor_location()-dodge_origin).length()>80))
        once('backlight',t,91,lambda:hit_from_back(5))
        once('backlightcheck',t,91.6,lambda:check('back light reaction',any(s['animation']=='Monster_StrikeJaeger_Ani_Hit_L_Back_Up' and 91<=s['t']<=92 for s in samples),name))
        once('backheavy',t,93,lambda:hit_from_back(60))
        once('backheavycheck',t,93.6,lambda:check('back heavy reaction',any(s['animation']=='Monster_StrikeJaeger_Ani_Hit_H_Back' and 93<=s['t']<=94 for s in samples),name))
        once('secondstun',t,103,lambda:check('repeated heavy recovers','STUNNED' not in state,state))
        once('kill',t,104,lambda:enemy.apply_combat_damage(10000,player))
        once('dead',t,106,lambda:check('death terminal',enemy.is_dead() and 'Death' in name,state))
        if t>=107:
            check('locomotion resumes after actions',any(s['t']>78 and 'Walk_' in s['animation'] for s in samples))
            check('root stays near capsule',max_root_offset<250,max_root_offset)
            finish()
    except Exception:
        finish(traceback.format_exc())

handle=unreal.register_slate_post_tick_callback(tick)
level.editor_request_begin_play()
