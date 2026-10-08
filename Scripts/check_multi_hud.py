import unreal,json,time,traceback
from pathlib import Path
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not level.is_in_play_in_editor()
start=time.monotonic(); state={'phase':0}
def tick(dt):
 try:
  w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
  if not w:return
  hud=unreal.WidgetLibrary.get_all_widgets_of_class(w,unreal.CombatHUDWidget,False)
  if not hud:return
  if state['phase']==0:
   hud[0].call_method('SelectHard');hud[0].call_method('StartBattle');state['phase']=1;state['start']=time.monotonic()
  if time.monotonic()-state['start']>2:
   enemies=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StrikeJaeger)
   # Native widget tree is reflected through all_widgets helper, if available.
   bars=[o for o in unreal.ObjectIterator(unreal.ProgressBar) if o.get_world()==w] if hasattr(unreal,'ObjectIterator') else []
   report={'enemy_count':len(enemies),'progress_bars':len(bars),'enemies_passed':len(enemies)==4,'bar_count_verified':len(bars)>=7}
   Path('E:/mihoyou/KianaCombatDemo/Saved/MultiEnemyHUDCheck.json').write_text(json.dumps(report))
   unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
 except Exception:
  Path('E:/mihoyou/KianaCombatDemo/Saved/MultiEnemyHUDCheck.json').write_text(traceback.format_exc());unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
