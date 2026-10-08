import unreal,json,traceback
from pathlib import Path
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not level.is_in_play_in_editor()
state={'world':None,'done':set(),'seen':set(),'peak':0}
def tick(dt):
 try:
  w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
  if not w:return
  if state['world'] is None:
   state['world']=w
   widgets=unreal.WidgetLibrary.get_all_widgets_of_class(w,unreal.CombatHUDWidget,False)
   widgets[0].call_method('StartBattle')
   state['p']=unreal.GameplayStatics.get_player_character(w,0)
   for e in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StrikeJaeger):
    e.set_actor_tick_enabled(False)
    e.set_actor_location(unreal.Vector(20000,20000,100),False,False)
   state['start']=unreal.GameplayStatics.get_time_seconds(w)
  t=unreal.GameplayStatics.get_time_seconds(w)-state['start'];p=state['p']
  for key,when,fn in [('skill',1,p.debug_skill),('dodge',4,p.debug_dodge),('hit',7,lambda:unreal.GameplayStatics.apply_damage(p,1,None,None,unreal.DamageType))]:
   if t>=when and key not in state['done']:state['done'].add(key);fn()
  actors=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.CombatFeedback)
  state['peak']=max(state['peak'],len(actors))
  for a in actors:
   c=a.get_component_by_class(unreal.NiagaraComponent);asset=c.get_asset()
   if asset:state['seen'].add(asset.get_name())
  if t>14:
   result={'seen':sorted(state['seen']),'peak':state['peak'],'remaining':len(actors),'passed':len(state['seen'])==3 and len(actors)==0}
   Path('E:/mihoyou/KianaCombatDemo/Saved/VFXCheck.json').write_text(json.dumps(result,indent=2))
   unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
 except Exception:
  Path('E:/mihoyou/KianaCombatDemo/Saved/VFXCheck.json').write_text(traceback.format_exc())
  unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick)
level.editor_request_begin_play()



