ARŞİV: Başarısız displayfix sürümüne aittir. Bu sürüm geri çekildi; güncel belge RECOVERY-BATARYA-RAPORU.md.

# Bruce Waveshare 2.8B ekran düzeltmesi

Teslim sürümü: 1.16.1-waveshare28b-displayfix. Gerçek PlatformIO derlemesi SUCCESS.
Bu yeni imaj fiziksel kartta test edilmedi; titremenin giderildiği henüz doğrulanmadı.
Temel orijinal Bruce 1.16.1: ba519c936c87b89c9c667c15b932d7faca5360d5.

## Kaynak incelemesi ve olası kök neden

Önceki portun tek 614.400 bayt framebuffer'ına bütün çizim işlemleri doğrudan yazıyordu.
CPU çizim mutex'i RGB DMA'yı kilitlemiyordu. Primitive başına cache writeback,
çizim sırasında framebuffer'ın taranmasına engel değildi: yırtılmaya yol açabilecek
bir yarış kaynakta doğrulandı. Bunun gözlenen yoğun titremenin tamamını açıkladığı
fiziksel ölçümle kanıtlanmadı.

Önceki 480×10 piksel bounce-buffer yolu, PSRAM'dan CPU önbelleği üzerinden ISR kopyaları
yapıyordu. Wi-Fi/çizim yükünde yetişememe ve flash/NVS işlemleri sırasındaki önbellek
durumu ikinci olası etkendir. SDK'da PSRAM XIP etkin değil.
Bu değerlendirme [ESP-IDF RGB dokümantasyonu](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/lcd/rgb_lcd.html)
ve [v5.5.4 sürücü kaynağı](https://github.com/espressif/esp-idf/blob/v5.5.4/components/esp_lcd/rgb/esp_lcd_panel_rgb.c)
ile karşılaştırıldı. Osiloskop/logic analyzer/underrun ölçümü yapılmadı.

## Yapılan değişiklikler

- IDF'nin ayırdığı iki RGB565 PSRAM framebuffer ve doğrudan EDMA kullanılıyor. Üçüncü çizim canvas'ı veya LVGL eklenmedi.
- CPU yalnızca arka tampona çiziyor. Tüm Print, bitmap, sprite ve primitive yolları aynı recursive mutex ile korunuyor.
- Sunum esp_lcd_panel_draw_bitmap üzerinden tam sürücü framebuffer adresi ile yapılıyor; driver cache senkronizasyonunu yapıyor.
- on_frame_buf_complete ve on_vsync olayları internal DRAM sayaçlarına kaydediliyor. Erken gelen tek EOF tamponun güvenle yeniden kullanılabileceği anlamına gelmiyor.
- IDF descriptor prefetch nedeniyle eski kareyi bir kez daha gönderebildiğinden, gönderimden sonra üç DMA kare tamamlanması ve en az bir VSYNC bekleniyor. Bekleme sırasında iki tampon da değişmiyor.
- Başarı sonrası yalnızca değişen byte aralığı emekli tampona kopyalanıyor. Aynı piksel içeriği yeniden çizilmişse yeni DMA sunumu yapılmıyor.
- Olaylar 250ms içinde gelmezse hata seri porta yazılıyor; eski tamponun DMA'da kullanılıyor olma ihtimaline karşı render kilidi bırakılmadan çizim durduruluyor. Bu durumda cihaz yeniden başlatılmalı.
- Menü ve bildirim boyama işlemleri tek batch halinde korunuyor; tuş bekleyen döngüler batch dışında.
- Menüde aynı görünür sayfada yalnızca eski/yeni seçim satırı yenileniyor. Sayfa, etiket veya etkinlik durumu değişirse sayfa yeniden çiziliyor.
- Bitmap aktarımı piksel başına çağrı yerine mevcut Arduino_GFX bitmap yolunu kullanıyor.
- Sunum task'ı 50ms aralıkla çizimleri birleştiriyor. Panel yaklaşık 70,1Hz tarar; üç karelik bekleme nedeniyle sürekli UI sunumu yaklaşık 10–11Hz olabilir. Bu hız animasyon akıcılığı açısından cihazda değerlendirilmelidir.
- Mevcut SDK'nın VSYNC restart desteği korunuyor. Periyodik, zorunlu panel restart eklenmedi.

Çift tampon yırtılma yarışını hedefler; doğrudan EDMA yolunda PSRAM/flash bant genişliği
paylaşımı devam eder. Yoğun yük altında underrun riskinin tamamen kalktığı iddia edilmiyor.
Kconfig için etkisiz derleyici tanımları eklenmedi, yeni rastgele pixel clock seçilmedi.

## Resmî panel değerleri

480×640 RGB565; PLL240M; PCLK 30MHz; yükselen kenar.
HSYNC pulse/back/front: 10/70/60. VSYNC: 10/20/20.
HSYNC/VSYNC idle-low bayrakları 0, DE/PCLK idle 0.
16 veri ve dört kontrol pini, 258 başlangıç işlemi ve timing değerleri
[Waveshare resmî demo](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8B/ESP32-S3-Touch-LCD-2.8B-Demo.zip)
ile eşleşiyor. GT911 reset/interrupt/I²C kodu, TCA9554 işlemleri ve backlight PWM değişmedi.
Resmî demo bounce boyutu 10×640 piksel; eski port 10×480 kullanıyordu. Yeni yol bounce kullanmıyor.

| Seçenek | Değerlendirme |
|---|---|
| Tek framebuffer | En az PSRAM; taranan tampona eşzamanlı çizim yarışı sürer |
| Çift framebuffer | +614.400 bayt PSRAM; doğru emeklilik senkronizasyonu gerekli |
| Tek/çift framebuffer + bounce | Dahili tamponlar ani yük dalgalarını tamponlar; ISR kopyası, cache durması ve ek CPU yükü bu SDK'da sorun olabilir |
| Seçilen çift framebuffer + direkt EDMA | CPU bounce kopyası kaldırıldı; bant genişliği sınırı için fiziksel stres testi gerekli |

## Yazı ve yerleşim

Özgün glcdfont korunarak raster ölçeklemesi eklendi; ikinci UI sistemi yazılmadı.
FM menü hücresi 12×16 → 15×20: iki eksende %25 artış.
FP 6×8 → 8×10, FG 18×24 → 23×30; piksel yuvarlama nedeniyle genişlik artışı tam %25 değildir.
LW=8 konservatif yerleşim sınırı; gerçek textWidth/fontHeight ve cursor/bounds aynı raster metriklerini kullanır.

Menü satırları 24px, alt touch alanı 32px. Dikeyde en çok 22, yatayda 15 liste satırı.
Uzun seçenekler mevcut yatay metin kaydırmasını kullanır; uzun ortalı başlıklar taşmamak için
üç nokta ile kısalır. Bildirim satır aralığı/boyama kutusu gerçek font yüksekliğine göre hesaplanır.
Dosya listesi satır/karakter hesapları büyüyen fonta uyar; ScrollableTextArea dinamik metrikleri kullanır.
Durum çubuğunda kısa Bruce sürümü gösterilir; tam build kimliği About/CLI için korunur.
Dokunmatik yön haritası ve BOOT geri kontrolü aynı; footer alanı tftHeight ile otomatik genişler.

## Doğrulama ve özellik durumu

PlatformIO compile/link, imaj checksum/hash, birleşik offsetler ve partition sınırı doğrulandı.
386 korunan kaynak dosyası çalışan portla byte düzeyinde aynı.
src/main.cpp ve src/core/display.cpp dışındaki bütün src dosyaları korunuyor.
Host testi gerçek sürücü/sunum fonksiyonlarını mock DMA olayları ile çalıştırdı:
dirty birikimi, aynı kareyi atlama, kısmi kopya, sayaç taşması, VSYNC/EOF yokluğu ve timeout'ta tampon sahipliği.
Dört rotasyonda 1.228.800 koordinat ve iki yönde 1–500 seçenekli menü geometri hesabı geçti.
Bunlar fiziksel DMA, bant genişliği veya görüntü kalitesi testi değildir.

**Çalışan eski sürüm, kullanıcı tarafından doğrulanan:** açılış, touch, menüler, Wi-Fi tarama/liste/bağlantı ve Evil Twin başlatma.
**Yeni sürümde derleme/test ile doğrulanan:** kaynak bütünlüğü, sürücü ve UI kodunun derlenmesi, yukarıdaki host/statik kontroller.
**Yeni fiziksel test bekleyen:** titreme/yırtılma, Wi-Fi yükü, touch gecikmesi, uzun listeler, bildirim/klavye, bellek kararlılığı, BLE/SD/CLI ve bütün çalışma özellikleri.
**Önceki eksikler korunuyor:** batarya yüzdesi kalibrasyonu; özel IMU/RTC/buzzer entegrasyonu; RGB invert/donanım sleep no-op; USB HID/BadUSB yapılandırılmamış. CC1101/NRF24/LoRa/RFID/IR/GPS gibi kartta olmayan donanımlar eklenmedi.

## İmaj ve yedek

Birleşik Bruce-1.16.1-waveshare28b-displayfix-merged.bin: 3,854,912 bayt; yükleme 0x0.
Uygulama 3,789,376 bayt; 0x10000; factory sınırı 0x470000 = 4.653.056 bayt.
SHA256: ca3e1348c554bfd7987531ff4be00013361b09136af63f3bc8092e9d85d4d741

Çalışan teslim BIN'i ve önceki kaynak ZIP'i backup-working-28b içinde korundu.
Eski BIN SHA256: ad4ea0b8253e2fc9e996bbe698531f859322b02615d56561357edebf23f9689d.
Bu yedek cihazın güncel flash/NVS/SD içeriğinin okunmuş kopyası değildir.
Yeni proje ve eski proje ayrı klasörlerde tutuldu.

Ayrıntılı bellek: PSRAM-FRAMEBUFFER-RAPORU.md.
USB yükleme ve kısa kabul testi: YUKLEME-VE-EKRAN-TESTI.md.
