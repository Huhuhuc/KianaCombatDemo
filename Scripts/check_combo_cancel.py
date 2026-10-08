import unreal,json,time,traceback
from pathlib import Path
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not level.is_in_play_in_editor()
s={'p':None,'done':set(),'clips':{},'checks':[]};wall=time.monotonic()
def tick(dt):
 try:
  w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
  if not w:return
  if s['p'] is None:
   hud=unreal.WidgetLibrary.get_all_widgets_of_class(w,unreal.CombatHUDWidget,False)
   if not hud:return
   hud[0].call_method('StartBattle');s['p']=unreal.GameplayStatics.get_player_character(w,0);s['start']=unreal.GameplayStatics.get_time_seconds(w)
   for e in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StrikeJaeger):
    e.set_actor_tick_enabled(False);e.set_actor_location(unreal.Vector(20000,20000,100),False,False)
  p=s['p'];t=unreal.GameplayStatics.get_time_seconds(w)-s['start']
  for label,base,count in [('single',1,1),('dodge',8,5),('skill',15,5),('hurt',23,5)]:
   if t>=base and label not in s['done']:
    s['done'].add(label)
    for n in range(count):p.debug_attack();p.debug_attack_released()
   if label!='single' and t>=base+.15 and label+'cancel' not in s['done']:
    s['done'].add(label+'cancel')
    if label=='dodge':p.debug_dodge()
    elif label=='skill':p.debug_skill()
    else:unreal.GameplayStatics.apply_damage(p,1,None,None,unreal.DamageType)
   if base+.3<t<base+6:
    a=p.get_active_sequence();name=a.get_name() if a else 'None'
    s['clips'].setdefault(label,set()).add(name)
  if t>30 or time.monotonic()-wall>90:
   for label,names in s['clips'].items():
    attacks=[n for n in names if 'Attack' in n]
    s['checks'].append({'case':label,'passed':attacks==['Kiana_Attack1'] if label=='single' else not attacks,'seen':sorted(names)})
   Path('E:/mihoyou/KianaCombatDemo/Saved/ComboCancelCheck.json').write_text(json.dumps({'checks':s['checks']},indent=2))
   unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
 except Exception:
  Path('E:/mihoyou/KianaCombatDemo/Saved/ComboCancelCheck.json').write_text(traceback.format_exc());unreal.unregister_slate_post_tick_callback(handle);level.editor_request_end_play()
handle=unreal.register_slate_post_tick_callback(tick);level.editor_request_begin_play()
