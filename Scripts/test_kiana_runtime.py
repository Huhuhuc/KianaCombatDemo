"""Editor PIE regression, advances using game time across real engine frames."""
import unreal
import json
import traceback
from pathlib import Path

report_path=Path('E:/mihoyou/KianaCombatDemo/Saved/KianaRuntimeValidation.json')
report_path.write_text('{"status":"running"}',encoding='utf-8')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert world
player=unreal.GameplayStatics.get_player_character(world,0)
assert hasattr(player,'debug_move')
player.disable_input(unreal.GameplayStatics.get_player_controller(world,0))
mesh=player.get_components_by_class(unreal.SkeletalMeshComponent)[0]
for enemy in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.StrikeJaeger):
    enemy.set_actor_tick_enabled(False)
    enemy.set_actor_location(unreal.Vector(20000,20000,100),False,False)
origin=player.get_actor_location()
start_time=unreal.GameplayStatics.get_time_seconds(world)
events=[]
checks=[]
seen=set()
last_sequence=None
max_root_offset=0.0
last_click=-1.0

def seq():
    a=player.get_active_sequence()
    return a.get_name() if a else 'None'

def check(label,names):
    actual=seq()
    checks.append(dict(label=label,expected=names,actual=actual,passed=actual in names))

def click():
    player.debug_attack()
    player.debug_attack_released()

def event(t,fn):
    events.append([t,fn,False])

event(0.1,lambda:check('idle',['Kiana_Idle']))
event(0.65,lambda:check('start',['Kiana_RunStart']))
event(1.85,lambda:check('run',['Kiana_Run']))
event(2.5,lambda:check('stop',['Kiana_RunStopLeft','Kiana_RunStopRight']))
event(4.7,lambda:check('stop returns idle',['Kiana_Idle']))
event(5.3,lambda:check('walk',['Kiana_Walk']))
event(7.05,lambda:check('stop interrupt setup',['Kiana_RunStopLeft','Kiana_RunStopRight']))
event(7.4,lambda:check('movement interrupts stop',['Kiana_RunStart','Kiana_Run']))
event(9.0,click)
event(9.08,lambda:check('attack interrupts stop',['Kiana_Attack1']))
event(9.1,click)
event(10.05,lambda:check('early click buffered',['Kiana_Attack2']))
event(22.5,lambda:check('combo five exits',['Kiana_Idle','Kiana_Idle_1']))
event(23.0,click)
event(23.15,click)
event(24.1,player.debug_attack)
event(25.15,lambda:check('hold selects branch 3',['Kiana_Attack3_fenzhi']))
event(25.2,player.debug_attack)
event(26.5,lambda:check('hold selects branch 4',['Kiana_Attack4_fenzhi']))
event(26.55,player.debug_attack_released)
event(30.1,player.debug_dodge)
event(30.3,lambda:check('front evade',['Kiana_EvadeFront']))
event(32.1,lambda:check('evade returns moving',['Kiana_Run']))
event(32.2,click)
event(32.3,lambda:check('evade followup',['Kiana_Attack_QTE']))
event(38.0,lambda:player.debug_move(0,-1))
event(38.05,player.debug_dodge)
event(38.2,lambda:check('back evade',['Kiana_EvadeBack']))
event(40.1,player.debug_skill)
event(40.2,lambda:check('skill',['Kiana_Skill']))
event(42.0,lambda:unreal.GameplayStatics.apply_damage(player,5,None,None,unreal.DamageType))
event(42.2,lambda:check('light hit',['Kiana_Hit_L']))
event(44.0,lambda:unreal.GameplayStatics.apply_damage(player,25,None,None,unreal.DamageType))
event(44.2,lambda:check('heavy hit',['Kiana_Hit_H']))
event(52.1,lambda:check('alternate idle',['Kiana_Idle_1']))
event(53.0,lambda:unreal.GameplayStatics.apply_damage(player,1000,None,None,unreal.DamageType))
event(53.2,lambda:check('death',['Kiana_Die']))

def finish(error=None):
    unreal.unregister_slate_post_tick_callback(callback)
    if unreal.SystemLibrary.is_valid(player):
        player.enable_input(unreal.GameplayStatics.get_player_controller(world,0))
    data=dict(status='failed' if error or not all(c['passed'] for c in checks) else 'passed',checks=checks,sequences=sorted(seen),max_root_offset_cm=max_root_offset,error=error)
    report_path.write_text(json.dumps(data,indent=2),encoding='utf-8')
    print('KIANA_RUNTIME_RESULT',json.dumps(data))

def tick(dt):
    global last_sequence,max_root_offset,last_click
    try:
        t=unreal.GameplayStatics.get_time_seconds(world)-start_time
        forward=1 if (.5<=t<2.25 or 5.8<=t<6.9 or 7.2<=t<8.8 or 30<=t<32.15) else (.4 if 5<=t<5.65 else (-1 if 38<=t<39.55 else 0))
        player.debug_run_held(not (5<=t<5.65))
        player.debug_move(0,forward)
        if 10.1<t<17.5 and t-last_click>.09:
            click()
            last_click=t
        for e in events:
            if not e[2] and t>=e[0]:
                e[2]=True
                e[1]()
        name=seq()
        seen.add(name)
        root=mesh.get_socket_location('root')
        offset=(root-mesh.get_world_location()).length()
        max_root_offset=max(max_root_offset,offset)
        if name!=last_sequence:
            print('KIANA_SEQUENCE',round(t,3),name,'root_offset',round(offset,3))
            last_sequence=name
        if t>=55:
            finish()
    except Exception:
        finish(traceback.format_exc())

callback=unreal.register_slate_post_tick_callback(tick)
print('KIANA_RUNTIME_STARTED')
