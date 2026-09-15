using UmbraManager.Models.Editors;
using UmbraManager.Services;
using Xunit;

namespace UmbraManager.Tests;

public class StructuredJsonSerializerTests
{
    [Fact]
    public void ItemStats_CanonicalizesPhysicalAttack()
    {
        var fields = StructuredJsonSerializer.DeserializeItemStats("""{"physical_attack":12,"strength":3}""");
        var json = StructuredJsonSerializer.SerializeItemStats(fields);
        Assert.Contains("\"attack\":12", json.Replace(" ", ""));
        Assert.DoesNotContain("physical_attack", json);
        Assert.Contains("\"strength\":3", json.Replace(" ", ""));
    }

    [Fact]
    public void Effects_RoundTrip_AndKeepUnsupported()
    {
        var json = """[{"type":"DAMAGE","value_flat":10,"chance_percent":80},{"type":"FUTURE_EFFECT","value_flat":1}]""";
        var rows = StructuredJsonSerializer.DeserializeEffects(json);
        Assert.Equal(2, rows.Count);
        Assert.Equal("DAMAGE", rows[0].Type);
        Assert.True(rows[1].IsUnsupported);
        Assert.Equal("FUTURE_EFFECT", rows[1].UnsupportedType);
        var outJson = StructuredJsonSerializer.SerializeEffects(rows);
        Assert.Contains("FUTURE_EFFECT", outJson);
        Assert.Contains("DAMAGE", outJson);
    }

    [Fact]
    public void Effects_WrapSingleObject()
    {
        var rows = StructuredJsonSerializer.DeserializeEffects("""{"type":"heal","value_flat":5}""");
        Assert.Single(rows);
        Assert.Equal("HEAL", rows[0].Type);
    }

    [Fact]
    public void Effects_KeepConditions()
    {
        var json = """[{"type":"HEAL","value_flat":5,"chance_percent":100,"conditions_json":{"health_below_percent":40,"trigger":"on_dodge"}}]""";
        var rows = StructuredJsonSerializer.DeserializeEffects(json);
        Assert.Single(rows);
        Assert.Equal(40, rows[0].HealthBelowPercent);
        Assert.Equal("on_dodge", rows[0].Trigger);
        var outJson = StructuredJsonSerializer.SerializeEffects(rows);
        Assert.Contains("health_below_percent", outJson);
        Assert.Contains("on_dodge", outJson);
    }

    [Fact]
    public void Effects_RoundTrip_PreservesUnknownPropertiesAndOverrides()
    {
        var json = """[{"type":"REFLECT","value_percent":25,"target_override":"AREA_ALLY","include_caster":false,"future_field":{"mode":"x"},"conditions_json":{"trigger":"on_attack_received","future_condition":7}}]""";
        var rows = StructuredJsonSerializer.DeserializeEffects(json);
        Assert.Single(rows);
        Assert.Equal("REFLECT", rows[0].Type);
        Assert.Equal("AREA_ALLY", rows[0].TargetOverride);
        Assert.False(rows[0].IncludeCaster);

        var outJson = StructuredJsonSerializer.SerializeEffects(rows);
        Assert.Contains("\"future_field\":{\"mode\":\"x\"}", outJson);
        Assert.Contains("\"future_condition\":7", outJson);
        Assert.Contains("\"target_override\":\"AREA_ALLY\"", outJson);
        Assert.Contains("\"include_caster\":false", outJson);
    }

    [Fact]
    public void Effects_CatalogContainsAllAuthoritativeTypes()
    {
        Assert.Equal(25, SchemaCatalog.ImplementedEffectTypes.Length);
        Assert.Contains("EXECUTE", SchemaCatalog.ImplementedEffectTypes);
        Assert.Contains("RESOURCE_RESTORE", SchemaCatalog.ImplementedEffectTypes);
    }

    [Fact]
    public void ItemStats_KeepsUnknownLegacyKeys()
    {
        var fields = StructuredJsonSerializer.DeserializeItemStats("""{"attack":4,"custom_stat":9}""");
        Assert.Contains(fields, f => f.Key == "custom_stat" && f.Value == 9);
        var json = StructuredJsonSerializer.SerializeItemStats(fields);
        Assert.Contains("custom_stat", json);
    }
}

public class ServerConfigModelTests
{
    [Fact]
    public void RoundTrip_PreservesPortsAndBindHost()
    {
        var src = """{"environment":"development","database":{"host":"localhost","port":3306,"name":"umbra_eternum","user":"root","password":"x"},"auth":{"port":8080},"admin":{"bind_host":"127.0.0.1"}}""";
        var model = ServerConfigModel.FromJson(src);
        Assert.Equal(3306, model.DbPort);
        Assert.Equal("127.0.0.1", model.AdminBindHost);
        var merged = model.MergeInto(src);
        Assert.Contains("127.0.0.1", merged);
        Assert.Contains("8080", merged);
    }
}

public class SkillEffectValidationTests
{
    [Fact]
    public void ChanceOutOfRange_SetsError()
    {
        var row = new SkillEffectEditorRow { ChancePercent = 150 };
        Assert.True(row.HasErrors);
        Assert.NotEmpty(row.GetErrors(nameof(SkillEffectEditorRow.ChancePercent)));
    }
}

public class GmCatalogTests
{
    [Fact]
    public void ReloadLoot_Exists()
    {
        Assert.Contains(GmCommandCatalogService.BuildDefaultCatalog(), c => c.Name == "reload_loot");
    }

    [Fact]
    public void GiveItem_HasTypedArguments()
    {
        var give = GmCommandCatalogService.BuildDefaultCatalog().First(c => c.Name == "give_item");
        Assert.Contains(give.Arguments, a => a.Name == "player_id" && a.Required);
    }
}

public class NavRoleMatrixTests
{
    [Theory]
    [InlineData("super", true, true, true)]
    [InlineData("ops", false, true, false)]
    [InlineData("content", false, false, true)]
    [InlineData("", false, false, false)]
    public void RoleVisibility(string role, bool super, bool ops, bool content)
    {
        bool Allows(string required) => required switch
        {
            "any" => true,
            "super" => role == "super",
            "ops" => role is "super" or "ops",
            "content" => role is "super" or "content",
            _ => false
        };
        Assert.Equal(super, Allows("super"));
        Assert.Equal(ops, Allows("ops"));
        Assert.Equal(content, Allows("content"));
        Assert.True(Allows("any"));
    }
}
