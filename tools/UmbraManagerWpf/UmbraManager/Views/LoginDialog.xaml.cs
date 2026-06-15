using System.Windows;
using System.Windows.Input;
using UmbraManager.Services;

namespace UmbraManager.Views;

public partial class LoginDialog : Window
{
  private bool _loginInProgress;

  public string Username { get; private set; } = "";

  public LoginDialog()
  {
    InitializeComponent();
    UsernameBox.Focus();
  }

  private async void Login_Click(object sender, RoutedEventArgs e)
  {
    await TryLoginAsync();
  }

  private async void UsernameBox_KeyDown(object sender, KeyEventArgs e)
  {
    if (e.Key == Key.Enter)
      await TryLoginAsync();
  }

  private async Task TryLoginAsync()
  {
    if (_loginInProgress) return;

    var user = UsernameBox.Text.Trim();
    if (string.IsNullOrWhiteSpace(user))
    {
      ErrorText.Text = "Informe o username.";
      return;
    }

    _loginInProgress = true;
    ErrorText.Text = "Verificando...";
    LoginButton.IsEnabled = false;
    CancelButton.IsEnabled = false;

    try
    {
      var php = new PhpAdminClient();
      php.Configure(AppConfig.Instance.PhpApiBase, user, AppConfig.Instance.AdminSecret);
      var (ok, err, data) = await php.VerifyAdminAsync();
      data?.Dispose();

      if (!ok)
      {
        ErrorText.Text = err;
        return;
      }

      Username = user;
      DialogResult = true;
      // DialogResult já fecha o modal; não chamar Close() de novo
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

  private void Cancel_Click(object sender, RoutedEventArgs e)
  {
    DialogResult = false;
  }
}
