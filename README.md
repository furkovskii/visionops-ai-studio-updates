# VisionOps AI Studio · Ücretsiz Güncelleme Kanalı

**GitHub kullanıcı adı:** `furkovskii`  
**Depo:** `visionops-ai-studio-updates`

Bu depo yalnızca masaüstü uygulamasının yeni sürümlerini **GitHub Releases** üzerinden dağıtmak için kullanılır. Kaynak kodu, veritabanı dosyalarını, kamera kayıtlarını, kullanıcı şifrelerini veya Supabase secret/service_role anahtarlarını paylaşmayın. Buraya eklenen Release dosyalarının herkese açık olduğunu unutmayın.

## v0.5.1 güncellemesini yayımlama (bir kez)

1. ChatGPT'ten indirilen **VisionOps_AI_Studio_v051_Guncelleme.zip** dosyasını bilgisayara kaydedin.
2. Dosyanın adını **`VisionOps_AI_Studio_update.zip`** yapın. İçeriği açmayın veya değiştirmeyin.
3. [Yeni Release oluştur](https://github.com/furkovskii/visionops-ai-studio-updates/releases/new) bağlantısına girin.
4. **Choose a tag → Create new tag** kısmından **`v0.5.1`** etiketini oluşturun; hedef dal `main` olsun.
5. Release title: **`VisionOps AI Studio v0.5.1`**.
6. ZIP dosyasını **Attach binaries by dropping them here or selecting them** alanına yükleyin.
7. **Publish release** deyin.
8. Windows VisionOps AI Studio > **Ayarlar & Güncellemeler** içinden kullanıcı adı `furkovskii`, depo `visionops-ai-studio-updates` olduğuna bakın; ardından **Güncellemeleri denetle → İndir ve hazırla** deyin.
9. Uygulamayı tamamen kapatıp masaüstü kısayoluyla yeniden açın.

Sürüm `0.5.1` kuruluysa `v0.5.1` yeni sürüm olarak görünmez. Daha sonra `v0.5.2` gibi daha yüksek bir etiket kullanın.

**SHA-256 doğrulama özeti (v0.5.1 dosyası):** `f08163428cc570896a54bdc7d8dfa22ec234c21da018b57e6f411f76f37fd2fa`

## Güvenlik ve sınırlamalar

- Uygulama GitHub Releases API'sinden en son sürümü kontrol eder. Güncelleme, kullanıcı onayı ile indirilir ve uygulama yeniden açılınca uygulanır.
- İndirme sonrası SHA-256 karşılaştırması bozuk dosyayı yakalar; **tek başına yayıncının kimliğini ispatlamaz**. Ürün yayına çıkmadan önce imzalı güncelleme, pinlenmiş kaynak ve geri alma senaryoları güçlendirilmelidir.
- Public Release ZIP'i indirilebilir ve kaynak kodlarını içerebilir; ticari sürümden önce özel kaynak deposu ile ayrı imzalı dağıtım kurulmalıdır.
- Supabase bulut oturumu ile GitHub güncelleme sistemi ayrı işlerdir.
