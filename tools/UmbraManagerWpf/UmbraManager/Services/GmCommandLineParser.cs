using System.Text.Json.Nodes;

namespace UmbraManager.Services;

public static class GmCommandLineParser
{
    public static (string Command, JsonObject Args) Parse(string input)
    {
        var tokens = Tokenize(input);
        if (tokens.Count == 0)
            return ("", new JsonObject());

        var args = new JsonObject();
        for (var i = 1; i < tokens.Count; i++)
        {
            var token = tokens[i];
            var eqIndex = token.IndexOf('=');
            if (eqIndex <= 0)
                continue;

            var key = token[..eqIndex].Trim();
            var rawValue = token[(eqIndex + 1)..].Trim();
            if (string.IsNullOrWhiteSpace(key))
                continue;

            args[key] = ParseValue(rawValue);
        }

        return (tokens[0], args);
    }

    private static JsonNode? ParseValue(string raw)
    {
        if (string.IsNullOrWhiteSpace(raw))
            return "";

        if (bool.TryParse(raw, out var booleanValue))
            return booleanValue;

        if (int.TryParse(raw, out var intValue))
            return intValue;

        if (double.TryParse(raw, out var doubleValue))
            return doubleValue;

        if ((raw.StartsWith("{") && raw.EndsWith("}")) || (raw.StartsWith("[") && raw.EndsWith("]")))
        {
            try
            {
                return JsonNode.Parse(raw);
            }
            catch
            {
            }
        }

        return raw;
    }

    private static List<string> Tokenize(string input)
    {
        var tokens = new List<string>();
        if (string.IsNullOrWhiteSpace(input))
            return tokens;

        var current = new System.Text.StringBuilder();
        var quote = '\0';

        foreach (var ch in input)
        {
            if (quote == '\0' && char.IsWhiteSpace(ch))
            {
                FlushCurrent(tokens, current);
                continue;
            }

            if (ch is '"' or '\'')
            {
                if (quote == '\0')
                {
                    quote = ch;
                    continue;
                }

                if (quote == ch)
                {
                    quote = '\0';
                    continue;
                }
            }

            current.Append(ch);
        }

        FlushCurrent(tokens, current);
        return tokens;
    }

    private static void FlushCurrent(List<string> tokens, System.Text.StringBuilder current)
    {
        if (current.Length == 0)
            return;

        tokens.Add(current.ToString());
        current.Clear();
    }
}
