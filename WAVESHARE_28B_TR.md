> Güncel sürüm **1.16.1-ws28b-pro**: [Pro raporu](reports/BUILD-REPORT.md), [yükleme](reports/FLASH-INSTRUCTIONS.md). Önceki sürüm raporları tarihsel kayıttır. Pro donanım testi bekliyor.

> Güncel teslimat: **1.16.1-ws28b-trq**. Menü, UTF-8 kapsamı, yükleme ve fiziksel test: [TURKISH-Q.md](boards/waveshare_28b/TURKISH-Q.md). Önceki sürüm raporları tarihsel bağlamdır.

> Güncel teslimat: **1.16.1-ws28b-native-hid**. Kurulum, sınırlar ve test planı: [NATIVE-USB-HID.md](boards/waveshare_28b/NATIVE-USB-HID.md). Aşağıdaki önceki sürüm notları tarihsel bağlamdır; güncel derleme sonucu `boards/waveshare_28b/build-result.json` içindedir.

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


# Kurtarma/batarya imajı — yükleme ve kısa test

Yeni sürüm 1.16.1-ws28b-recovery-bat derlendi; fiziksel kararlılığı/batarya doğruluğu henüz test edilmedi.
Okunabilir görüntüye hemen dönmek için eski, kullanıcı tarafından doğrulanan
backup-working-28b/Bruce-waveshare_28b.bin dosyası korunuyor.
Yeni imaj kontrollü karşılaştırma için adaydır; önceki sürümden daha kötü olmadığı
sadece kaynak karşılaştırması/derlemeyle garanti edilemez.
1.16.1-waveshare28b-displayfix sürümü geri çekildi.

## USB yükleme

COM7 örnektir. Gerçek portu seçin; seri monitörü kapatın.
BOOT'a basılı tutup RESET'e basın, BOOT'u bırakın.
Birleşik imaj0x0 NVS aralığını da kapsar; kayıtlı ayarlar/gain sıfırlanabilir.
Mevcut cihaz flash/NVS yedeği isterseniz, yüklemeden önce:
~~~powershell
py -m pip install esptool
py -m esptool --chip esp32s3 --port COM7 --baud 460800 read-flash 0x0 0x1000000 board-backup-16MB.bin
~~~
Yeni aday:
~~~powershell
py -m esptool --chip esp32s3 --port COM7 --baud 460800 write-flash --flash-size 16MB 0x0 Bruce-1.16.1-ws28b-recovery-bat-merged.bin
~~~
Referansa dönüş aynı komutla eski Bruce-waveshare_28b.bin kullanılarak yapılır.
SD dosyaları ayrıca yedeklenir. Yükleme sonrası RESET.
~~~powershell
py -m platformio device monitor --port COM7 --baud 115200
~~~

## Ekranı dört ayrı koşulda teşhis

Config→LCD Diagnostics:
Fixed image / Touch polling OFF / WiFi OFF / WiFi scan ON.
Ekran okunmuyorsa, ana menüdeyken USB CLI girişleri:
~~~text
ws28b-fixed
ws28b-touch-off
ws28b-wifi-off
ws28b-wifi-on
ws28b-battery
~~~
Her ekran testi ana UI task'ına yönlendirilir; bir kere sabit resim çizilir.
60 saniyede biter; BOOT düğmesine basmak erken çıkarır.
BOOT'u reset sırasında tutmak test başlatmaz, ROM yükleyicisini seçer.
Wi-Fi isolasyonu AP/APSTA/WebUI aktifken reddedilir; önce bu servisleri durdurun.
WiFi ON testi STA taramasını her10 saniyede çalıştırır; parola/kalıcı config değiştirmez.
Çıkışta test taraması durdurulur, touch polling ve önceki Wi-Fi modu geri alınır;
eski STA bağlantısı varsa yeniden bağlantı asenkron başlar. BLE durumu değiştirilmez.
Touch OFF yalnızca GT911 polling'i durdurur; touch'ın beslemesi veya ortak I²C kapanmaz.

Aynı parlaklık/USB koşuluyla referans ve yeni adayı karşılaştırın:
1. Statik ekranda60s: siyah flash, renk kayması, tüm kare titremesi veya çizgi yırtılması.
2. Touch OFF: statik testle fark var mı; dönüşte touch/BOOT/menü düzgün mü.
3. WiFi OFF ve WiFi ON: tarama/bağlantı yükü görüntüyü bozuyor mu.
4. Normal menü/Wi-Fi ağ listesi:25% büyük yazılar taşmıyor mu; uzun SSID ve
   ilk/son seçenek kaydırması, PREV/SEL/NEXT alanları çalışıyor mu.
5. En az15dk normal kullanım; Wi-Fi/BLE/SD/CLI temel işlemleri, PSRAM free/largest
   eğilimi ve hata/yeniden başlama kaydı. Kalibrasyon veya ayar NVS yazımı sırasında
   görüntüyü ayrıca izleyin.

Test sırasında başka ekran çizen CLI komutlarını kullanmayın; sabit görüntü ölçümü bozulur.
Görüntü referanstan kötüleşirse referans BIN'e dönün ve hangi modda bozulduğunu kaydedin.
Sadece bir USB kameranın rolling shutter çizgileri yerine gözle görüntüyü de karşılaştırın.

## Batarya/güç test matrisi

| Koşul | Yapılacak gözlem |
|---|---|
| USB takılı, batarya sökülü | Kart USB ile açılmalı; ADC voltajı hücre varlığı sayılmamalı, source/charge unknown kalır |
| Batarya takılı, USB çıkarılmış | SW1 açıkken açılış ve ekran; voltaj BAT konnektörüyle karşılaştırılır |
| USB+batarya birlikte | Voltaj ve fiziksel şarj LED'i; yazılım LED'den şarj durumu algıladığını iddia etmez |
| SW1 kapalı, USB takılı | USB beslemesinin cihazı açık tutup tutmadığı kaydedilir |
| SW1 kapalı, USB çıkarılmış | Fiziksel batarya yolunun kesilmesi kontrol edilir; yazılım güç kesme testi değildir |

Config→Battery→Voltage/estimate veya ws28b-battery değerini multimetreyle
BAT-GND üzerinde karşılaştırın. 3:1 oran ve eFuse kalibrasyonu kaynakta doğrulandı;
gerçek doğruluk ölçümle belirlenir.
İsterseniz Calibrate with meter'a gerçek BAT voltajını girin; girilen değerin
hesaplanmış bir ADC değeri olmaması gerekir. Başarılı gain NVS'ye kaydedilir.
Farklı voltajlarda tekrar kontrol edin; tek nokta gain düzeltmesi tüm ADC eğrisini doğrulamaz.
Batarya-yalnız kullanımda USB seri bağlantısı kesilir; ölçümü ekrandan kaydedin.

Düşük voltaj davranışı yazılım eşikleri3.45V/3.60V ve üç5s örnektir.
LCD LOW, durum çubuğu yeniden çizildiğinde; seri uyarı eşik geçişinde görünür.
Donanım testi için uygun kontrollü batarya kaynağıyla voltajı ölçerek bu davranışı sınayın.
Şarj yüzdesi yaklaşık; gerçek kapasite/BMS testi yapılmış değildir.
Güvenli yazılım güç kesme desteklenmez. Deep Sleep, fiziksel güç kesme olarak değerlendirilmez.

## Kaynaktan derleme/test

~~~powershell
py -m pip install platformio==6.2.0
py -m platformio run -e waveshare_28b
py tests/waveshare_28b/recovery_contract.py g++
~~~
C++11 host compiler gerekir. PlatformIO bağımlılıkları mevcut yapılandırmadan yükler.
build.py bootloader0x0 + partition0x8000 + app0x10000 birleşimini
Bruce-waveshare_28b.bin adıyla üretir.
Windows uzun yol gerekirse kısa çalışma yolu kullanılmalıdır; bu derleme B: üzerinde yapıldı.
Arduino3.3.9 / pioarduino55.3.39 / IDF libs5.5.4+sha.735507283d / Xtensa GCC14.2 kullanıldı.
Yalnız firmware dosyasını yeniden adlandırmak yerine orijinal Bruce kaynakları derlendi.
