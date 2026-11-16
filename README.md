# VisionOps AI Studio — Güncelleme Merkezi

Bu herkese açık depo **yalnızca VisionOps AI Studio masaüstü uygulamasının sürüm güncellemelerini** dağıtmak içindir. Uygulama kaynak kodu, kullanıcı parolaları, kullanıcı veritabanları ve kamera kayıtları burada bulunmamalıdır.

## Nasıl güncelleme yayımlanır?

1. Değişiklikleri yerel ortamda test et ve `version.json` içindeki sürüm numarasını yükselt (örnek: `0.5.0`).
2. Yeni sürüme ait, doğrulanmış `VisionOps_AI_Studio_update.zip` dosyasını hazırla.
3. Bu depoda **Releases → Draft a new release** bölümünü aç.
4. **Choose a tag → Create new tag** ile `v0.5.0` gibi bir sürüm etiketi seç.
5. Paket dosyasını adı **tam olarak** `VisionOps_AI_Studio_update.zip` olacak şekilde Release'e ekle.
6. **Publish release** ile yayımla.
7. VisionOps AI Studio'da yönetici olarak **Ayarlar & Güncellemeler → Güncellemeleri kontrol et** menüsünü kullan.

Bağlantı ayarları:
- GitHub kullanıcısı: `furkovskii`
- Depo: `visionops-ai-studio-updates`

**Önemli:** Uygulamanın mevcut sürümü `0.4.1` ise `v0.4.1` yayını “yeni güncelleme” olarak görülmez. Yalnızca daha yüksek sürüm numarası güncelleme olarak sunulur.

## Güvenlik

- Buraya `.env`, `users.db`, `events.db`, `*.sqlite`, erişim anahtarları, parolalar, tokenlar veya özel kamera görüntüleri yüklemeyin.
- Güncelleme ZIP'leri herkese açık olacaktır. Kaynak kodun gizli kalması gerekiyorsa yayın öncesinde ayrıca derleme/paketleme sürecini oluşturun.
- SHA-256 indirme bozulmasını tespit eder; tek başına imzalı yayın doğrulaması yerine geçmez. Ticari sürümden önce imzalı güncelleme/doğrulanmış yayın süreci ekleyin.

## Bulut hesapları

Supabase gibi bir kimlik doğrulama servisi bu depodan ayrıdır. Kayıtlar ve kullanıcı hesapları, açıkça güvenli bulut senkronizasyonu geliştirilene kadar yerel bilgisayarda kalır.
