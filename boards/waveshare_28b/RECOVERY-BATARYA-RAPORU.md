# Bruce Waveshare 2.8B — regresyon geri alma ve batarya desteği

Sürüm: 1.16.1-ws28b-recovery-bat. Gerçek PlatformIO derlemesi SUCCESS.
Bu yeni imaj **fiziksel donanımda test edilmedi**. Öncekinden daha iyi veya aynı
görüntü kararlılığı verdiği henüz kanıtlanmadı. Yeni batarya ölçümü de fiziksel doğrulama bekliyor.
Temel orijinal Bruce 1.16.1 commit: ba519c936c87b89c9c667c15b932d7faca5360d5.

## Regresyon ve geri alma

Kullanıcı 1.16.1-waveshare28b-displayfix imajında okunamayacak düzeyde titreme bildirdi.
Bu sürüm geri çekildi. Önceki çalışan Bruce-waveshare_28b.bin yedeği değiştirilmedi.
Önceki host testleri gerçek DMA/PSRAM yükünü doğrulamaya yetmedi.

| Yol | Çalışan referans | Başarısız displayfix | Yeni kurtarma |
|---|---|---|---|
| PSRAM framebuffer | 1×614.400 B | 2×614.400 B | 1×614.400 B |
| Dahili bounce tamponları | 2×9.600 B | yok | 2×9.600 B |
| DMA kaynağı | dahili bounce | doğrudan PSRAM | dahili bounce |
| Özel swap/VSYNC/EOF | yok | 3 EOF + VSYNC bekleme | kaldırıldı |
| Sunum task'ı/mirror | yok | var | kaldırıldı |
| SDK log seviyesi | 1 | 3 | 1 |

Dört RGB bus/display sürücü dosyası çalışan referansla byte düzeyinde aynı.
HAL framebuffer constructor ve cache writeback davranışı geri alındı.
50ms sunum task'ı, iki tampon dirty mirroring, callback sayaçları ve kare teslim bariyeri yok.
Menüde seçili satırı kısmen yenileyen deneysel optimizasyon da kaldırıldı.
Gereksiz yeni panel restart veya tahmini clock/timing eklenmedi.

**En güçlü olası neden:** bounce tamponlarını kaldırıp 30MHz'de doğrudan PSRAM DMA'ya
geçmek ve buna cache flush/mirror/karşılaştırma trafiğini eklemek bant genişliği
ve underrun davranışını kötüleştirmiş olabilir. Ayrıca olay sayısı ile gerçek
taranan framebuffer kimliği fiziksel olarak doğrulanmamıştı.
Kesin nedeni tek başına gözlenen titremeden saptayamıyorum; osiloskop/underrun kaydı yok.
ST7701 aynı RGB pixel akışını alır; çift tampon IDF'de desteklenen bir mekanizmadır,
bu kart/SDK/yük birleşiminde yaptığım uygulamanın kararlı olduğu kanıtlanmadı.
Bu nedenle çift tampon yolunu tutmadım.
[ESP-IDF RGB bellek/senkronizasyon açıklaması](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/api-reference/peripherals/lcd/rgb_lcd.html)

Bu geri alma, referansın mevcut tek-framebuffer yırtılma ve flash/NVS sırasında
bounce-cache sınırlamalarını da korur. “Titreme tamamen giderildi” iddiası değildir.
Sonraki optimizasyon ancak karşılaştırmalı fiziksel ölçümle yapılmalıdır.

## Panel, dokunmatik ve font

480×640, RGB565, PLL240M/PCLK 30MHz; yükselen kenar.
HSYNC pulse/back/front 10/70/60; VSYNC 10/20/20.
16 RGB veri pini, dört kontrol pini ve 258 ST7701 init işlemi resmî demo ile eşleşiyor.
Çalışan porttaki 4800px bounce boyutu korundu; resmî demo makrosu 6400px'dir.
Kararlı referanstan farklı yeni bir bounce boyutu seçilmedi.

GT911 reset, adres, interrupt sırası ve koordinat dönüşümü korunuyor.
InputHandler'a yalnızca batarya servisi ve isteğe bağlı touch-polling baskılama kontrolü eklendi.
Baskılama normal kullanımda false; BOOT girişi her durumda okunuyor.
Wi-Fi/BLE araç kodları değişmedi. CLI ve Config menüsüne yalnızca board'a özel girişler eklendi.

Özgün glcdfont ile FM hücresi 12×16 →15×20 (%25); küçük/büyük fontlarda
piksel yuvarlama var. Satır yüksekliği 24px, footer alanı 32px; portrede en çok22,
yatayda15 seçenek. Bildirim ve dosya listesi metrikleri buna uyar.
Font raster büyütmesi RAM/bellek trafiğini değiştirebildiği için bu bölümün
görüntü üzerindeki etkisi de fiziksel karşılaştırmada kontrol edilmelidir.

## Batarya ve güç şeması

[Resmî Waveshare şeması](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8B/ESP32-S3-Touch-LCD-2.8B_schematic_diagram.pdf) görsel ve bağlantı tablosu ile incelendi.
R2=200kΩ, R18=100kΩ, tolerans ±%1; GPIO4 BAT_ADC/ADC hattını okur.
Nominal dönüşüm Vbat=3×Vadc. SW1, Q2 üzerinden BAT→BAT_OUT yolunu kontrol eder.
MCU güç-kesme hattı yok. USB'nin VCC besleme yolu ayrı kalır.
ETA6098 STAT, LED1'i sürer; MCU'ya bağlı şarj-durum veya USB-detect hattı yok.
Şema SHA256: 9c7d84529d421a86b907ff29063155ddb54a01bc1805d1f81769097b9af95947.

GPIO4, ADC1 üzerinde 12bit ve11dB kullanır. Arduino3.3.9 analogReadMilliVolts
kodunun IDF curve-fitting kalibrasyonu çağırdığı yerel framework kaynağında kontrol edildi.
16 örnek, uç ikişer örneği çıkarılmış ortalama; 5 saniye aralık.
Başarısız/sıfır veya geçersiz voltaj gizlenir; son örnek15 saniyeden eskiyse geçerli sayılmaz.
[Espressif ADC API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html)

Resmî demo 0.992857 ampirik offset kullanıyor. Bunu kullanıcı kartına
ölçmeden aktarmadım; varsayılan ek gain=1.0.
Config→Battery→Calibrate with meter ile BAT konnektöründe ölçülen voltaj girilebilir.
Gain0.9–1.1 aralığı ve güncel ADC örneği kontrol edilir; başarılı NVS yazımı sonrası uygulanır.
Bu NVS işlemi, referans bounce yolunda flash/cache etkisini test etmek için de izlenmelidir.

Pil simgesi, voltaj ve yaklaşık yüzde (~) durum çubuğunda gösterilir.
Yüzde eğrisi genel tek hücre LiPo için yazılım göstergesi; ölçülmüş kullanıcı hücresinin SOC eğrisi değildir:
3300/3500/3600/3700/3800/3900/4100/4200mV →0/5/15/35/60/75/95/100%.
Şarj sırasında veya yük altında gerçek kalan kapasiteyi doğru temsil ettiği iddia edilmez.

≤3.45V üç ardışık örnekte LOW uyarısı; ≥3.60V'da temizleme.
Seri uyarı eşik geçişinde, kırmızı LOW/simge bir sonraki durum çubuğu çiziminde görünür.
Araç ekranlarının çalışmasını kesen otomatik popup veya batarya kapatma eklenmedi.
Bu eşikler yazılım uyarı politikasıdır; hücre koruma devresi yerine geçmez.

**Desteklenmeyen:** otomatik USB/batarya kaynağı, şarj durumu ve hücre varlığı algılama;
gerçek yazılımla güç kesme. USB'ye bağlanmak “şarj oluyor” kanıtı sayılmaz.
Hücre bağlı değilken şarj devresi BAT hattında voltaj üretebilir;
geçerli ADC değeri bataryanın takılı olduğunun kanıtı değildir.
Power menüsü bu kartta fiziksel switch bilgisini gösterir;
CLI poweroff desteklenmeyen güç kesme için false döndürür ve gizlice deep sleep başlatmaz.
Mevcut Deep Sleep/Sleep seçenekleri kalır; tüm kartın enerjisini kestikleri iddia edilmez.

## Doğrulama ve bellek

PlatformIO compile/link; imaj checksum/hash; merged offsetler; partition sınırı;
sürüm/commit metni; kaynak/ELF zaman sırası; 386 korunan kaynak hash'i doğrulandı.
Gerçek battery_model.h üzerinde monoton0–100% sınırı, düşük-voltaj debounce,
geçersiz örnek ve histerezis host testleri geçti.
Dört rotasyonda1.228.800 koordinat ve iki yönde1–500 seçenekli geometri kontrolü geçti.
Teşhis cleanup/dispatch kontrolleri statik sözleşmedir; gerçek radyo/touch testi değildir.

PSRAM8.388.608 B OPI80MHz; framebuffer614.400 B; bounce dahili19.200 B.
Yeni ELF .dram0.data+.dram0.bss=120,052 B.
.ext_ram.dummy=3,669,984 B sanal linker boşluğudur; fiziksel PSRAM tahsisi değildir.
Ek presentation task/canvas/LVGL çizim tamponu yok. ADC ve mutex heap maliyeti runtime'da ölçülmedi.
Free/largest değerleri teşhis modunda her5 saniyede loglanır; burada ölçülmüş değer uydurulmadı.

**Fiziksel kullanıcı doğrulaması olan:** eski sürümün açılış/touch/menü/Wi-Fi işlemleri ve okunabilir ekranı.
**Başarısız fiziksel sürüm:** displayfix; yeni sürüm bu iddiayı devralmaz.
**Yeni sürümde fiziksel test bekleyen:** ekran/font/diagnostics, pil voltajı/yüzde/uyarı,
USB/batarya/switch davranışı, Wi-Fi/BLE/SD/CLI regresyonu ve uzun süreli bellek.
Önceki IMU/RTC/buzzer, RGB invert/sleep ve USB HID sınırlamaları devam eder.

## İmaj ve değişen dosyalar

Birleşik Bruce-1.16.1-ws28b-recovery-bat-merged.bin: 3,842,816 B, adres0x0.
Uygulama 3,777,280 B; factory partition0x10000/0x470000'a sığar.
SHA256: d9052b84c4c8da5ed8fe24d99027d3c8fdc5ee9bbc0678167483588d610ae524
Çalışan referans SHA256: ad4ea0b8253e2fc9e996bbe698531f859322b02615d56561357edebf23f9689d

Çalışan referansa göre değişen/eklenen kaynaklar:
- boards/waveshare_28b/battery.cpp
- boards/waveshare_28b/battery_model.h
- boards/waveshare_28b/diagnostics.cpp
- boards/waveshare_28b/interface.cpp
- boards/waveshare_28b/pins_arduino.h
- boards/waveshare_28b/waveshare_28b.ini
- boards/waveshare_28b/waveshare_support.h
- lib/Arduino_GFX/src/Arduino_GFX.cpp
- lib/Arduino_GFX/src/Arduino_GFX.h
- lib/HAL/display/ardgfx.cpp
- src/core/display.cpp
- src/core/menu_items/ConfigMenu.cpp
- src/core/serial_commands/cli.cpp
- src/core/serial_commands/power_commands.cpp
- src/main.cpp

Başarısız sürüme göre dört RGB bus/display dosyası ayrıca geri alındı.
recovery-vs-working-source.patch uygulanabilir metin farkını içerir.
USB yükleme, kontrollü teşhis ve güç test matrisi: RECOVERY-YUKLEME-TEST.md.
