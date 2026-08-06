using System.Windows;
using System.Windows.Input;
using UmbraManager.Services;

namespace UmbraManager.Views;

public partial class LoginDialog : Window
{
    private bool _loginInProgress;

    public string Username { get; private set; } = "";
    public string Token { get; private set; } = "";
    public string Role { get; private set; } = "super";

    public LoginDialog()
    {
        InitializeComponent();
        UsernameBox.Focus();
    }

    private async void Login_Click(object sender, RoutedEventArgs e) => await TryLoginAsync();

    private async void Input_KeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.Enter)
            await TryLoginAsync();
    }

    private async Task TryLoginAsync()
    {
        if (_loginInProgress) return;

        var user = UsernameBox.Text.Trim();
        var password = PasswordBox.Password;
        if (string.IsNullOrWhiteSpace(user))
        {
            ErrorText.Text = "Informe o username.";
            return;
        }
        if (string.IsNullOrEmpty(password))
        {
            ErrorText.Text = "Informe a senha.";
            return;
        }

        _loginInProgress = true;
        ErrorText.Text = "Verificando...";
        LoginButton.IsEnabled = false;
        CancelButton.IsEnabled = false;

        try
        {
            var php = new PhpAdminClient();
            php.Configure(AppConfig.Instance.PhpApiBase, user);
            var (ok, err, data) = await php.VerifyAdminAsync(password);
            if (!ok)
            {
                ErrorText.Text = err;
                data?.Dispose();
                return;
            }

            Username = user;
            if (data != null)
            {
                var root = data.RootElement;
                if (root.TryGetProperty("token", out var tokenEl))
                    Token = tokenEl.GetString() ?? "";
                if (root.TryGetProperty("role", out var roleEl))
                    Role = roleEl.GetString() ?? "super";
                else if (root.TryGetProperty("admin", out var admin) && admin.TryGetProperty("role", out var ar))
                    Role = ar.GetString() ?? "super";
                data.Dispose();
            }

            DialogResult = true;
        }
        catch (Exception ex)
        {
            ErrorText.Text = ex.Message;
        }
        finally
        {
            _loginInProgress = false;
            LoginButton.IsEnabled = true;
            CancelButton.IsEnabled = true;
        }
    }

    private void Cancel_Click(object sender, RoutedEventArgs e) => DialogResult = false;
}
