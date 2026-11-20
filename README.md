# VisionOps AI Studio · Distribution & Update Center

## Current repository layout

- Current Windows release: [GitHub Releases](https://github.com/furkovskii/visionops-ai-studio-updates/releases/latest). Older installers and updates remain attached to their original releases.
- `release-input/` and `setup-input/`: retained v0.13.0 reproducible base and v0.12.9 fallback only. Historical duplicates are removed only after SHA-256 equality with release assets is verified by CI. Git history remains intact.
- `qa/ai-engine/`: active AI tracker build, tests and release integration. `android-update.json`: active Android update metadata. `docs/`: existing public redirects and compatibility pages.
- Web/iOS/Android sources live separately in [visionops-ai-studio](https://github.com/furkovskii/visionops-ai-studio).


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
- `release-input/v0.13.0.zip` → pinned AI build input.
- `setup-input/v0.13.0.zip` → pinned installer input.

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

## v0.9.3 · Admin panel, multiple pages and no-cost password screening

- Public homepage no longer has a visual-editor pencil.
- Website administration: `/admin.html`, authenticated via Supabase Auth and a server-enforced admin RLS role; sections for Panels, Panel Information, Account Settings and a link to Visual Editor.
- Visual editor: `/editor.html`, includes local undo history for unpublished changes, editable existing elements and custom blocks. New pages may be created with slugs and populated with blocks; public pages render through `/page.html?slug=...`.
- Desktop Pet workspace: `← Çalışma alanlarına dön` returns to Pet/Café/Farm selection without closing the app. Sign out returns to selection, not app exit. Admin studio now has active/planned panel cards, navigation and local account self-service.
- Password screening: free HIBP Pwned Passwords k-anonymity (5 SHA-1 hex characters sent only) on local password creation/change and website UI signup/change. It is **not a substitute for server-side Supabase Auth leaked-password protection**; the latter is a paid feature, and the free UI check can be bypassed by direct API use.
- No new paid services. Supabase existing row-level security remains enabled for page editing, and no payment entitlement is automatically granted.
- Security limits: local SQLite data is not claimed to be encrypted at rest, unsigned prototype update packages are not suitable for commercial launch, and browser-admin sessions rely on same-origin JavaScript security.
- Website files are committed to GitHub. Distribution ZIP still needs developer release publishing via GitHub CLI before the installed app's Check for Updates can download v0.9.3.

## v0.9.4 — Ücretsiz güvenlik ve görsel site tasarımı
- Web editöründeki `Geçersiz öğe kimliği` hatası: hatalı kaçırılmış regex düzeltildi.
- Yeni sayfa oluşturma: tek isim alanı, otomatik Türkçe uyumlu URL kısa adı.
- İki editör: mevcut görsel düzenleyici (`/editor.html`) ve ücretsiz açık kaynak GrapesJS stüdyosu (`/studio.html`), yalnızca Supabase admin hesabıyla erişilebilir.
- GrapesJS: sürükle-bırak bloklar, font/stil denetimi, şablonlar, mobil/tablet önizleme, Openverse anahtarsız açık lisanslı görsel araması; lisans ve atıf koşulları kullanıcı tarafından kontrol edilmeli.
- GrapesJS sayfaları Supabase RLS ile korunur; yayınlanmış içerikler `/page.html?slug=...` üzerinden script çalıştırmayan sandbox iframe ile görüntülenir.
- GitHub Actions `actions/attest@v4` ile ücretsiz Sigstore/SLSA kaynak doğrulama kaydı üretir.
- v0.9.4 istemcisi yeni güncellemeleri doğrulanmış yayın imzası olmadan yüklemez; *ücretsiz GitHub CLI* gerektirir. Kötü amaçlı yazılım barındırabilecek manuel ZIP yüklemeleri de aynı şekilde doğrulanır.
- Masaüstü: isteğe bağlı parola korumalı taşınabilir AES-256-GCM yedek. Yedekler yerel oluşturulur, buluta gönderilmez; 12+ karakterlik yedek parolası unutulursa geri yüklenemez.
- Yeni kurulum, `YEDEK_GERI_YUKLE.bat` kurtarma yardımcısını içerir. Mevcut kurulumda aynı kurtarma işlemi, uygulama kapalıyken `secure_backup.py --restore <file>` üzerinden çalıştırılabilir.
- **Sınır:** canlı SQLite veritabanı tamamen şifrelenmiş değildir. Windows Device Encryption/BitLocker gibi cihaz-disk şifrelemesini destekleyen bilgisayarda etkinleştirmek gerekir. Bu özellik bazı Windows sürümlerinde bulunmayabilir.
- Supabase'in server-side leaked password protection özelliği **Pro planında** bulunduğu için etkinleştirilmedi. Ücretsiz istemci tarafı parola kontrolü tek başına zorunlu sunucu koruması sağlamaz.
- Güvenlik özellikleri Windows, internete bağlı GitHub ve gerçek kamera üzerinde uçtan uca henüz doğrulanmadı; prototip üzerinde çalışıyoruz.
- Ücretli ürün, yükseltme veya abonelik açılmadı. GitHub Pages ve herkese açık Actions attestations ücretsiz kullanım kotası ve politikalarına tabidir.

## v0.9.5 · Consolidated editor, desktop usability, accounts
- One web editor: `/studio.html` (old `/editor.html` now redirects). Only available from the admin dashboard; user navigation no longer has two editors.
- Admin session is reused through same-tab `sessionStorage` only after server-side Supabase admin role re-verification. Expired sessions require login again.
- Editor loads current website HTML/CSS on first use; no empty initial canvas. GrapesJS stylesheet path corrected to `dist/css/grapes.min.css`. Published page data is preserved as GrapesJS project JSON.
- Web main page renders published advanced design instead of duplicating the original homepage below it.
- GitHub Actions gained `artifact-metadata: write` permission needed for artifact attestations v4. Historical failing jobs must still be inspected from actual GitHub Actions logs.
- Windows startup shows CSS logo animation (reduced-motion respected), then the Pet/Café/Farm chooser with welcome copy.
- Pet sidebar scrolls and ends in a single **Çıkış yap** button. Touchpad horizontal swipe (when Windows WebView exposes it) and Alt+Left attempt to return to prior panel view; behavior is hardware-dependent.
- Desktop admin visual designer removed; replaced with product cards and account list, including creation and last sign-in timestamp (recorded from v0.9.5 onward).
- Supabase admin role-checked RPC `visionops_admin_account_overview` shows account package/status/grant date/last signed-in, but only if the local desktop admin has also connected a Supabase admin cloud session. Never return password hashes.
- Packages remain free prototype; no payment, billing, or new paid service.
- Limitations: no end-to-end Windows/WebView test, no verified successful GitHub Action run, and current account duration only means enrollment/since granted, not total time used.

## v0.9.6 — Backward-compatible update and one website editor
- Fixed "Paket beklenmeyen dosya içeriyor" from old updaters by moving secure backup functions into already-approved `secure_store.py`. Update ZIP contains only files accepted by installed versions v0.9.0 through v0.9.5. Offline encrypted-backup restoration remains accessible via `secure_store.py --restore`.
- One visual website editor at `docs/studio.html`, accessible from website admin. It no longer depends on a GrapesJS CDN or a fullscreen hack; the first canvas is populated from current live HTML/CSS or published `visionops_visual_pages`.
- Features: live page selection, inline text editing, image URL, typography, backgrounds, cards, section templates, Openverse image picker, undo/redo, desktop/tablet/phone preview, and admin-only Supabase publishing. Work in progress: granular responsive layout controls and reliable in-browser UX testing.
- GitHub Actions `repair-v096.yml` prepares compatibility packages from prior Release; trusted signer `publish-visionops.yml` consumes them after repair workflow success and emits persistently stored free provenance attestations.
- The legacy installed app requires the trusted signer workflow, so a ZIP created directly without that attestation cannot be installed through the strict local updater. Use the app's Check for Updates after both workflows succeed.
- Neither workflow success nor Windows and browser end-to-end functionality can be guaranteed from static source tests. Do not expose local admin/data services to the Internet.

## Lovable website integration — 17 November 2026

- New public website: **https://visionops-ai-studio.lovable.app/** (Lovable free-plan hosting).
- The previous GitHub Pages website at https://furkovskii.github.io/visionops-ai-studio-updates/ now redirects visitors to the Lovable website. Old /admin.html, /account.html, and /studio.html URLs redirect to the relevant Lovable routes.
- Previous web pages are backed up under `docs/legacy-2026-11/`. To roll back, restore these into `docs/`.
- Lovable project: https://lovable.dev/projects/90ddaf44-3d8e-41b4-bd50-b809a9324b0e
- Lovable frontend uses the **existing** Supabase Auth and RLS project; no new database was provisioned.
- Current Lovable code has **not** been synchronized back into this repository. GitHub remains the Windows release and update distribution repo; Lovable hosts the web frontend independently.
- The site's free-domain URL changes from GitHub Pages to lovable.app on redirect. This is a redirect-based integration, not a static code deployment into GitHub Pages.
- Before a commercial launch, perform browser QA, Supabase permissions tests and confirm download links; do not publish paid plans or expose customer data.
