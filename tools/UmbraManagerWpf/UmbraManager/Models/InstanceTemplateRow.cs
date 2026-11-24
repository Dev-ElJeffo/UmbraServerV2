namespace UmbraManager.Models;

/// <summary>Linha de instance_templates (instâncias lógicas da zone, etapa 4).</summary>
public sealed class InstanceTemplateRow
{
    public int InstanceTemplateId { get; set; }
    public string Code { get; set; } = "";
    public string Name { get; set; } = "";
    public string Description { get; set; } = "";
    /// <summary>0 = qualquer zone.</summary>
    public int ZoneId { get; set; }
    /// <summary>Soft path do level do UE: /Game/Maps/Instances/Lvl_X.Lvl_X.</summary>
    public string MapPath { get; set; } = "";
    /// <summary>Origem da cópia 0 (onde o cliente carrega o level). Dentro de ±1.000.000.</summary>
    public double OriginX { get; set; } = 600000;
    public double OriginY { get; set; } = -900000;
    public double OriginZ { get; set; }
    /// <summary>Distância entre cópias no eixo Y (maior que o level + 30000).</summary>
    public double SlotSpacing { get; set; } = 150000;
    /// <summary>Ponto de entrada LOCAL do level (coordenadas do Editor).</summary>
    public double EntryX { get; set; }
    public double EntryY { get; set; }
    public double EntryZ { get; set; } = 100;
    public double EntryYaw { get; set; }
    public int MinLevel { get; set; } = 1;
    public int MaxPlayers { get; set; } = 5;
    public bool PartyRequired { get; set; }
    /// <summary>0 = sem limite.</summary>
    public int TimeLimitMinutes { get; set; }
    /// <summary>Fecha a cópia vazia depois de N segundos.</summary>
    public int EmptyCloseSeconds { get; set; } = 120;
    public int MaxCopies { get; set; } = 12;
    public bool IsActive { get; set; } = true;
    /// <summary>NPCs do molde (npc_instances.instance_template_id).</summary>
    public int NpcCount { get; set; }

    // --- Dungeon (etapa 5) ---
    public bool IsDungeon { get; set; }
    /// <summary>final_boss (chefe final conclui) ou all_bosses (todos os chefes).</summary>
    public string CompletionMode { get; set; } = "final_boss";
    /// <summary>EXP e ouro de conclusão (x multiplicador da dificuldade).</summary>
    public int RewardExp { get; set; }
    public int RewardGold { get; set; }
    /// <summary>none, daily ou weekly: recompensa de conclusão 1x por período.</summary>
    public string LockoutReset { get; set; } = "daily";
    /// <summary>Fecha a cópia N s depois de concluir.</summary>
    public int CompletionCloseSeconds { get; set; } = 300;
    /// <summary>NPCs do molde marcados como chefe ou chefe final.</summary>
    public int BossCount { get; set; }
}
