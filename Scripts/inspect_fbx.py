import unreal
u=unreal.FbxImportUI()
print('UI', [x for x in dir(u) if 'snap' in x.lower() or 'anim' in x.lower() or 'import' in x.lower()])
for p in ['anim_sequence_import_data','import_animations','animation_length','snap_to_closest_frame_boundary']:
 try: print(p,u.get_editor_property(p))
 except Exception as e: print(p,type(e).__name__,e)
try:
 d=u.get_editor_property('anim_sequence_import_data'); print('DATA',type(d),[x for x in dir(d) if 'snap' in x.lower() or 'frame' in x.lower() or 'animation' in x.lower()]);
 for p in ['animation_length','snap_to_closest_frame_boundary']: 
  try: print('D',p,d.get_editor_property(p))
  except Exception as e: print('D',p,e)
except Exception as e: print('DATAERR',e)
