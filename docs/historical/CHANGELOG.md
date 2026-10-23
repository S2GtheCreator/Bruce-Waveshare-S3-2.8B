> Historical report for the original pre-publication build. It does not certify the sanitized release binary. See ../../BUILD-REPORT.md.

# 1.16.1-ws28b-pro değişiklikleri

* Aşama 0: çalışan trq BIN/kaynak arşivi SHA yedeği, Git stable etiketi/branch, başlangıç PlatformIO derlemesi.
* Aşama 1: metin alanı sınırları/font geri yükleme/UTF-8 satırları; 12 Türkçe glif; Shell başlık/çıktı ayrımı; koordinat testi. RGB, GT911 ve HID sürücüleri korundu.
* Aşama 2: Battery Manager dashboard, geçerlilik/belirsizlik ve bilinmeyen güç durumu; fiziksel pil onayına bağlı yüzde. ADC/gain/debounce tabanı korundu.
* Aşama 3: pasif 2,4 GHz Analyzer, AP/kanal/RSSI/karşılaştırma, sınırlı geçmiş, maskeli CSV/JSON ve temiz çıkış.
* Aşama 4: Script Manager/editör/favori/recent/kategori; doğrulanan kayıt ve çalışma kopyası; cihaz onayı, önizleme ve iptal; CLI/JS HID oturum çakışma kontrolü; beş zararsız test.
* Aşama 5: ortak Türkçe giriş alanı; 256 satır Shell geçmişi/fixed prompt/10 Hz gövde limiti; kaynak snapshot; kompakt pil yüzdesi; BadUSB font/STOP/ilerleme alanı; final Pro sürümü ve regresyon doğrulaması.

Derleme seçenekleri: board ini'deki WS28B_TOUCH_PRO=1 yerine 0 seçilebilir. Diğer Pro eklentilerini kapatmak için aynı hedefin build_flags bölümüne -DWS28B_BATTERY_PRO=0, -DWS28B_WIFI_ANALYZER_PRO=0, -DWS28B_BADUSB_PRO=0 ekleyin; bunlar #ifndef varsayılanlarıdır. Temel Bruce donanım/HID işlevlerini kapatmazlar. USB_as_HID=1 ve USB OTG/CDC ayarlarını kaldırmayın.

Bilinen sınırlar: pil/USB/şarj otomatik ayrımı ve yazılımsal güç kesme donanımda yok; yüzdeler tahmin. Yeni swipe/kinetik sürücü, tam Unicode ekran fontu, masaüstü düzeyinde editör veya Pro'da ALT sayısal metin çalıştırma eklenmedi. Donanım kararlılığı/ADC doğruluğu/performans ölçülmedi. Eski ağır çift-framebuffer displayfix geri getirilmedi.
