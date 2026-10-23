# Regresyon testleri

Host/derleme sonuçları donanım testi değildir. Kullanıcı başlangıç `ws28b-trq` sürümünde açılış, ekran, dokunmatik, Wi-Fi tarama/bağlantı, Native HID ve Türkçe Q ile uygulama/URL açmayı doğruladı. Bu doğrulama Pro sürümüne aktarılmaz.

| Kontrol | Yazılım kanıtı | Pro fiziksel durum |
|---|---|---|
| Bruce açılışı | Gerçek derleme, image checksum/hash | Test edilmedi |
| ST7701 görüntü | RGB/DMA/init/pin SHA aynı | Kararlılık/titreme test edilmedi |
| GT911 | Sürücü SHA aynı, koordinat ekranı hazır | Köşeler/döndürmeler test edilmedi |
| Menü/geçiş/uzun liste | Metin widget bounds/UTF-8 testleri | Test edilmedi |
| Wi-Fi tarama | Mevcut araçlar aynı; yeni pasif mod derlendi | Test edilmedi |
| Wi-Fi bağlantısı | Bağlantı/araç kaynakları aynı | Test edilmedi |
| BLE başlatma | BLE taşıması korunup linklendi | Test edilmedi |
| Türkçe Q / English US | 447 gerçek sürücü raporu/mock USB | Windows testi yapılmadı |
| USB HID / CDC | Native koşullar/link, kitaplık aynı | Enumerasyon/CDC birlikte test edilmedi |
| Shell | Sınırlı geçmiş, font/viewport ve SDK derleme | 200+ satır testi yapılmadı |
| Dosya sistemi | 8 safeSave fault-injection senaryosu | Gerçek LittleFS/SD/dolu FS testi yapılmadı |
| Batarya | Gerilim modeli/debounce testi | ADC-multimetre testi yapılmadı |
| USB çıkarma | Otomatik kaynak/latch sensörü yok | Pille çalışma doğrulanmadı |
| Yeniden başlatma | Mevcut conf ve yeni metadata/NVS kayıt yolları | Kalıcılık testi yapılmadı |

Gerçek host kodu testleri `source/tests/waveshare_28b/README-PRO.md` altında; günlükler bu klasördedir. Hiçbiri Windows'a tuş yazmaz veya script çalıştırmaz.

Kısa cihaz kabul planı:

1. Ayarları koruyan uygulama yüklemesiyle başlayın. Açılış/menüler/dokunmatik köşeleri ve dört döndürmeyi kontrol edin; Touch/Turkish text testinde harfleri okuyun.
2. Wi-Fi kapalı/açık/tarama altında 5'er dakika ekranı gözleyin. Eski trq ile karşılaştırın; daha kötü görüntüde geri dönün.
3. Kendi test SSH/Telnet sunucunuzda uzun ve Türkçe 200+ satır çıktı, başlık/giriş ayrımı, dikey geçmiş ve uzun giriş önizlemesini deneyin.
4. Battery Manager'da önce pil varlığını fiziksel doğrulayın. Multimetre/BAT_OUT/3V3 ve USB çıkarma adımlarını BATTERY-REPORT'a göre uygulayın.
5. Analyzer'da bilinen AP/kanalı görün, karşılaştırma seçin, RSSI geçmişi ve iptali deneyin. Önceki bağlantı/mod korunmalı. CSV/JSON'u bilgisayarda açın; maskeli raporda gerçek kimlikler olmamalı.
6. Yedekli test FS üzerinde oluştur/kopyala/rename/kategori/favori/geri al/save-as/sil-onay; dolu FS'de hata ve eski dosyanın korunması. Reboot sonrası metadata/klavye ayarı korunmalı.
7. Kendi Windows bilgisayarınızda boş Notepad'e beş zararsız testi uygulayın. Windows ve Bruce düzeni eşleşmeli. TR/US dönüşünü, Shift/AltGr/Caps'i ve desteklenmeyen Unicode hatasını kontrol edin.
8. 30 s DELAY ve uzun STRING sırasında STOP/BOOT; sonrasında Ctrl/Alt/Shift/GUI basılı kalmamalı. USB çıkarma/yeniden takma ve aynı sırada CDC bağlantısını kontrol edin.
9. 20 modül aç/kapat döngüsünden önce/sonra Memory snapshot: free/min heap, largest block, PSRAM ve stack marjını kaydedin. SDK ısınma/cache etkisini tek seferlik tahsisten ayırın; sürekli azalma varsa raporlayın.

Yeni özelliklerde başarılı fiziksel test sonucu uydurulmamıştır.
