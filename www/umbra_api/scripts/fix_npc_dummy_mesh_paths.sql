-- Limpa paths de mesh invalidos no dummy_treino.
-- O cliente UE usa fallback: mesh/AnimBP do pawn local quando estes campos sao NULL/vazios.
UPDATE npc_templates
SET skeletal_mesh_path = NULL,
    anim_blueprint_path = NULL
WHERE npc_name = 'dummy_treino';
