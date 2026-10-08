import unreal
base='/Game/CombatDemo/Characters/Kiana'
lib=unreal.MaterialEditingLibrary
names={'Body':'Avatar_Kiana_C2_Texture_Body_Hair_Color','Face':'Avatar_Kiana_Texture_Face_Color','Eye':'Eye','Mouth':'Mouth'}
mats={}
for part,texture in names.items():
    path=base+'/Materials/M_Kiana_'+part
    mat=unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_Kiana_'+part,base+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    lib.delete_all_material_expressions(mat)
    sample=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-400,0)
    sample.texture=unreal.load_asset(base+'/Textures/'+texture)
    lib.connect_material_property(sample,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
    rough=lib.create_material_expression(mat,unreal.MaterialExpressionConstant,-400,180);rough.r=0.8
    lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    spec=lib.create_material_expression(mat,unreal.MaterialExpressionConstant,-400,260);spec.r=0.15
    lib.connect_material_property(spec,'',unreal.MaterialProperty.MP_SPECULAR)
    mat.set_editor_property('two_sided',True)
    mat.set_editor_property('used_with_skeletal_mesh',True)
    lib.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    mats[part]=mat
mesh=unreal.load_asset(base+'/Meshes/Avatar_Kiana_C2_Model_RootBone')
slots=mesh.get_editor_property('materials')
# Maya export retains generic material names. Face slot is explicit;
# body/hair share an atlas. Eye and mouth assignments follow export materials.
parts=['Body','Body','Eye','Eye','Mouth','Face','Body']
for i,part in enumerate(parts):
    slot=slots[i]
    slot.set_editor_property('material_interface',mats[part])
    slots[i]=slot
mesh.set_editor_property('materials',slots)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
print('KIANA_COLOR_BOUND',[(str(s.material_slot_name),str(s.material_interface)) for s in mesh.get_editor_property('materials')])
