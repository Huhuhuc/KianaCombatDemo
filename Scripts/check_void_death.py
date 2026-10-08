import unreal, time, json, traceback
from pathlib import Path
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
started=time.monotonic()
phase=0
def tick(dt):
    global phase
    try:
        if time.monotonic()-started>30: raise RuntimeError('Timeout')
        w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        if not w:return
        widgets=unreal.WidgetLibrary.get_all_widgets_of_class(w,unreal.CombatHUDWidget,False)
        if not widgets:return
        p=unreal.GameplayStatics.get_player_character(w,0)
        if phase==0:
            if not unreal.GameplayStatics.is_game_paused(w):return
            widgets[0].call_method('StartBattle')
            p.set_actor_location(p.get_actor_location()-unreal.Vector(0,0,1500),False,False)
            phase=1
            return
        if p.is_dead():
            result=dict(dead=True,health=p.get_health_percent(),pawn_retained=True)
            assert result['health']==0
            Path('E:/mihoyou/KianaCombatDemo/Saved/VoidDeathCheck.json').write_text(json.dumps(result))
            unreal.unregister_slate_post_tick_callback(handle)
            level.editor_request_end_play()
    except Exception:
        Path('E:/mihoyou/KianaCombatDemo/Saved/VoidDeathCheck.json').write_text(traceback.format_exc())
        unreal.unregister_slate_post_tick_callback(handle)
        level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick)
level.editor_request_begin_play()
