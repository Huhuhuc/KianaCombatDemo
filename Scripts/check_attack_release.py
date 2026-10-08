import unreal, json, traceback
from pathlib import Path
out=Path('E:/mihoyou/KianaCombatDemo/Saved/AttackReleaseCheck.json')
out.write_text('{"status":"running"}')
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not level.is_in_play_in_editor(), 'Stop current PIE before test'
world=None
events=set()
checks=[]
clips={0:[],1:[],2:[]}
def finish(error=None):
    unreal.unregister_slate_post_tick_callback(handle)
    out.write_text(json.dumps(dict(status='failed' if error or not all(x['passed'] for x in checks) else 'passed',checks=checks,clips=clips,error=error),indent=2))
    level.editor_request_end_play()
def tick(dt):
    global world,p,start,origin,dodgeorigin
    try:
        if world is None:
            world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
            if not world: return
            p=unreal.GameplayStatics.get_player_character(world,0)
            p.disable_input(unreal.GameplayStatics.get_player_controller(world,0))
            for e in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.StrikeJaeger):
                e.set_actor_tick_enabled(False)
                e.set_actor_location(unreal.Vector(10000,10000,100),False,False)
            origin=p.get_actor_location()
            start=unreal.GameplayStatics.get_time_seconds(world)
            unreal.SystemLibrary.execute_console_command(world,'t.MaxFPS 60')
        t=unreal.GameplayStatics.get_time_seconds(world)-start
        p.debug_move(0,1 if t<1.5 else 0)
        def once(key,when,fn):
            if t>=when and key not in events: events.add(key); fn()
        once('move',1.6,lambda:checks.append(dict(name='movement',passed=(p.get_actor_location()-origin).length()>100)))
        for trial,base,interval in [(0,3,0.01),(1,12,0.35),(2,21,0.55)]:
            for n in range(5):
                def click(): p.debug_attack(); p.debug_attack_released()
                once((trial,n),base+n*interval,click)
            if base<=t<base+8:
                a=p.get_active_sequence(); name=a.get_name() if a else 'None'
                if 'Attack' in name and (not clips[trial] or clips[trial][-1]!=name): clips[trial].append(name)
        if t>=30:
            for trial,names in clips.items():
                checks.append(dict(name='bounded ordered attack trial '+str(trial),passed=bool(names) and names==['Kiana_Attack'+str(n) for n in range(1,len(names)+1)] and (trial!=0 or len(names)<=2)))
            checks.append(dict(name='returns to idle without new input',passed='Attack' not in p.get_active_sequence().get_name()))
            finish()
    except Exception: finish(traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
level.editor_request_begin_play()
