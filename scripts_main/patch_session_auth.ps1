# JWTManager.hpp
$p = 'd:\UmbraServerV2\src\auth\JWTManager.hpp'
$t = Get-Content $p -Raw
if ($t -notmatch 'sessionVersion') {
  $t = $t.Replace('  uint64_t playerId = 0;', "  uint64_t playerId = 0;`n  uint32_t sessionVersion = 0;")
  $t = $t.Replace('uint32_t expirationMinutes = 60);', 'uint32_t expirationMinutes = 60,`n                            uint32_t sessionVersion = 0);')
  Set-Content $p $t -NoNewline
}
# JWTManager.cpp
$p = 'd:\UmbraServerV2\src\auth\JWTManager.cpp'
$t = Get-Content $p -Raw
if ($t -notmatch 'session_version') {
  $t = $t.Replace(
    'std::string JWTManager::generateToken(uint64_t accountId, `r`n                                      uint64_t playerId,`r`n                                      const std::string& username,`r`n                                      uint32_t expirationMinutes) {',
    'std::string JWTManager::generateToken(uint64_t accountId, `r`n                                      uint64_t playerId,`r`n                                      const std::string& username,`r`n                                      uint32_t expirationMinutes,`r`n                                      uint32_t sessionVersion) {'
  )
  $t = $t.Replace('  payload.username = username;', "  payload.username = username;`n  payload.sessionVersion = sessionVersion;")
  $t = $t.Replace('  json["username"] = payload.username;', "  json[`"username`"] = payload.username;`n  json[`"session_version`"] = payload.sessionVersion;")
  $t = $t.Replace('    payload.username = json["username"];', "    payload.username = json[`"username`"];`n    if (json.contains(`"session_version`")) {`n      payload.sessionVersion = json[`"session_version`"];`n    }")
  $t = $t.Replace('                       expirationMinutes);', '                       expirationMinutes,`n                       payload->sessionVersion);')
  Set-Content $p $t -NoNewline
}
# CMakeLists
$p = 'd:\UmbraServerV2\src\zone\CMakeLists.txt'
$t = Get-Content $p -Raw
if ($t -notmatch 'umbra_auth') {
  $t = $t.Replace('  umbra_services', "  umbra_services`n  umbra_auth")
  Set-Content $p $t -NoNewline
}
Write-Host 'jwt+cmake patched'
