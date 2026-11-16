# VisionOps AI Studio · Distribution & Update Center

**Website:** https://furkovskii.github.io/visionops-ai-studio-updates/ (GitHub Pages must be enabled: Settings → Pages → Deploy from branch → main → /docs).  
**Windows installer:** https://github.com/furkovskii/visionops-ai-studio-updates/releases/latest/download/VisionOps_AI_Studio_Setup.zip  
**Windows updates:** Published as `VisionOps_AI_Studio_update.zip` in GitHub Releases.

## Version v0.8.0
- Workspace chooser displayed at application startup: Pet, Café, Farm.
- Pet dashboard only opens after Supabase account entitlement check; Café and Farm remain previews.
- Desktop and cloud “Remember me” use signed local sessions and Windows DPAPI-protected refresh tokens. Passwords are not stored.
- New off-white public website with product pages, Windows-branded download, and matching account screen.
- Café preview adopts vibrant coral, orange, cinnamon and cream colors.
- Existing Pet camera, OBS bridge, events and reports remain in place.

## How updates are published (developer only)
GitHub Actions workflow: `.github/workflows/publish-visionops.yml`. Upload versioned ZIP archives to:
- `release-input/v0.8.0.zip` → update file.
- `setup-input/v0.8.0.zip` → fresh setup file.

The workflow checks the internal version and releases the archives with the exact filenames expected by the app. The supplied Windows publisher uploads both files via GitHub CLI. App users just select “Check for updates” inside VisionOps.

**Do not upload** user databases, passwords, tokens, photos, camera recordings, Supabase secret/service_role keys, or `.env` files.

The repository and all Release assets are **public**. Python sources inside prototype packages are visible to anyone. Prior to commercial distribution, build a private source repository, signed installers, code-signing, and a tested rollback policy.

## Product status
- Pet: prototype with an active development entitlement.
- Café: visual preview; real analytics not yet available.
- Farm: planned.
- Payments/subscriptions: disabled; no checkout or active billing.
- Mobile application: postponed.

Website, account creation, app licensing and GitHub release delivery have different security boundaries. A Supabase entitlement controls access; a UI card alone does not grant a license.

## v0.9.0 · Yönetim Stüdyosu
- Masaüstü açılış ekranında sağ üstte **Yönetici Girişi** bulunur. Yerel yönetici parolası yalnızca yerel bilgisayar için kullanılır.
- Yerel yönetici stüdyosunda Pet/Café/Farm görünümleri önizlenir; Café ve Farm gerçek analiz modülü değildir.
- Başlık ve renklerin yayımlanması için mevcut **Supabase bulut hesabının yönetici rolü** kullanılır.
- Web sitesinde **Hesabım → Yönetici Girişi** ile aynı Supabase bulut hesabına giriş yaparak başlık, açıklama ve renkler değiştirilebilir.
- Windows yerel kullanıcı adı/parolası GitHub Pages'e veya Supabase'e aktarılmaz. Bu iki hesap katmanı şu aşamada birleştirilmiş değildir.
- Supabase RLS, yöneticiler dışında marka ayarlarına yazma izni vermez.
- Ücretlendirme ve canlı Café/Farm analizi etkin değildir.
- Geliştirme aşamasında yerel yönetici Pet prototipini bulut girişi olmadan açabilir; bu lisans kontrolünün ticari sürüm için uygun olduğu anlamına gelmez.

## v0.9.1 · Açılış, oturum güvenliği ve görsel editör
- Desktop starts on `/choose` at every launch. Pet / Café / Farm selection appears **before** local login.
- Local session signing key rotates each desktop process run; previous sessions cannot automatically authenticate. `Beni hatırla` only stores the local username.
- Cloud refresh tokens are not restored automatically. Remembered cloud email is optional; each new desktop session requires explicit cloud login.
- Admin desktop studio includes a local Pet panel preview and basic panel title, colors, custom information widgets.
- Website editor: https://furkovskii.github.io/visionops-ai-studio-updates/editor.html . Full-page preview, pencil edit toggle, text/heading/card/image/button/divider blocks, drag ordering within custom section, admin-only Supabase RLS saves.
- Account page lets signed-in users rotate a potentially compromised Supabase password. Browser password-manager warnings are not evidence of a VisionOps breach.
- **Scope limit:** the desktop editor does not yet move every internal dashboard widget, and the website editor reorders only custom blocks, not core layout regions. No production-hardening claims.
- On Windows, publish v0.9.1 from the provided GitHub publisher; app users then use Check for updates. The working earlier v0.9 remains recoverable.


## v0.9.2 — Hesap öz yönetimi ve öğeye tıklayarak düzenleme
- **Hesabım** (yerel Windows): Yönetici ve görüntüleyici kendi kullanıcı adını ve parolasını mevcut parola doğrulamasıyla güncelleyebilir; değişiklik eski oturumu sonlandırır.
- **Admin Studio** (masaüstü): Yerel yöneticinin kendi hesap ayarları ve Pet panel önizlemesindeki seçilebilir yazı/kart özellikleri. Tasarım değişiklikleri dosyada yerel olarak kaydedilir. Kamera ve olay hesaplama kodu değişmez.
- **Bulut & Senkronizasyon**: Bağlı Supabase kullanıcısı görünen adını ve parolasını, eski parolasıyla yeniden doğrulandıktan sonra değiştirebilir.
- **Hesabım (web)**: Supabase kullanıcısı kendi görünen adını ve parolasını değiştirebilir; işlem sonrası yeniden giriş gerekir.
- **Web Tasarımcı**: Önceki sürükle/bırak özel bileşenlere ek olarak, ana sayfadaki seçilebilir mevcut yazı ve kutuların görünümünü doğrudan sağ özellik panelinden değiştirebilir. Değişiklikler public-read/admin-write RLS ile korunur.
- Editörün erişimi admin hesabıyla doğrulanır. Kullanıcılar kendi rolünü admin yapamaz.
- **Sınır:** Tüm dahili UI öğeleri için tam serbest konumlandırma veya işletim mantığını değiştiren bir no-code sistem değildir.
- Supabase güvenlik danışmanı sızdırılmış parola kontrolünün kapalı olduğu uyarısını vermektedir; ticari yayından önce ele alınmalıdır.
- Dağıtım: versioned v0.9.2 update/setup files are published via existing GitHub Actions after a publisher ZIP is uploaded by developer. No payment subscription is activated.

## v0.9.2 · Self-service account settings + desktop visual edits
- Both local admin and viewer accounts can change only their own username/password after presenting their current password. Existing login sessions are invalidated on change; roles never change through self-service.
- Windows Pet dashboard: admin-only preview-based property inspector for selected text/cards (appearance changes only), with strict allowlisting and no change to camera/analytics logic.
- Cloud: signed-in users can change their own `display_name` and password with current-password reauthentication. Only the `display_name` column is writable to non-admin users; authorization `role` cannot be edited.
- Website: signed-in user profile editor and existing live visual editor are available. Published website edits are admin-protected by Supabase RLS.
- Editing support is **not unrestricted**: only approved visual targets and properties can be edited; all business logic, access controls and interactive behavior remain controlled by developers.
- Current Windows launch requirement: desktop choose screen precedes local login and new process requires login.
