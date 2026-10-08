import unreal, json, traceback
from pathlib import Path
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
world=None
events=set()
checks=[]
clips=set()
def finish(error=None):
    unreal.unregister_slate_post_tick_callback(handle)
    Path('E:/mihoyou/KianaCombatDemo/Saved/MovementRecovery.json').write_text(json.dumps(dict(checks=checks,error=error),indent=2))
    level.editor_request_end_play()
def tick(dt):
    global world,p,start,origin
    try:
        if world is None:
            world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
            if not world:return
            p=unreal.GameplayStatics.get_player_character(world,0)
            p.disable_input(unreal.GameplayStatics.get_player_controller(world,0))
            for e in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.StrikeJaeger):
                e.set_actor_tick_enabled(False)
                e.set_actor_location(unreal.Vector(10000,10000,100),False,False)
            start=unreal.GameplayStatics.get_time_seconds(world)
        t=unreal.GameplayStatics.get_time_seconds(world)-start
        a=p.get_active_sequence()
        if a:clips.add(a.get_name())
        def once(k,at,fn):
            if t>=at and k not in events:events.add(k);fn()
        once('move',0.2,lambda:p.debug_move(0,1))
        once('attack',0.6,lambda:p.debug_attack())
        once('release',0.8,lambda:p.debug_attack_released())
        once('resume',4,lambda:checks.append(dict(name='held direction resumes after attack',passed=p.get_velocity().length()>50)))
        once('run',4.1,lambda:p.debug_run_held(True))
        once('stop',6,lambda:p.debug_move(0,0))
        once('stopped',7.5,lambda:checks.append(dict(name='run releases to stationary',passed=p.get_velocity().length()<5)))
        once('stopclip',7.6,lambda:checks.append(dict(name='player run stop clip used',passed=any('Stop' in n for n in clips))))
        if t>=8:finish()
    except Exception:finish(traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
level.editor_request_begin_play()
