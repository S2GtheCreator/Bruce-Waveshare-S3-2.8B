# Waveshare 2.8B Native USB HID düzeltmesi

Bu sürüm, mevcut `1.16.1-ws28b-recovery-bat` kaynak projesinin ayrı kopyasıdır.
Bruce temeli: 1.16.1, commit `ba519c936c87b89c9c667c15b932d7faca5360d5`.
Yeni sürüm: `1.16.1-ws28b-native-hid`.

## Kök neden ve değişiklikler

Eski kart tanımı `ARDUINO_USB_MODE=1` (Hardware CDC/JTAG) kullanıyordu;
`USB_as_HID` tanımlı değildi. Bu nedenle Bruce, kartta bulunmayan harici
CH9329'u UART üzerinden bekliyordu. Native OTG/TinyUSB modu seçildi:

```
ARDUINO_USB_MODE=0
ARDUINO_USB_CDC_ON_BOOT=1
USB_as_HID=1
```

Arduino çekirdeği CDC açıkken `setup()` öncesinde `USB.begin()` çağırır.
TinyUSB CDC'nin global `USBSerial` nesnesi, Bruce'un aynı isimli wrapper
sınıfıyla çakışıyordu. Wrapper tipi `BruceUSBSerial` olarak adlandırıldı;
mevcut `USBserial` örneği ve seri CLI arayüzü korunur.
Klavye descriptor'ı global, kalıcı `USBHIDKeyboard` ile bundan önce kaydedilir.
Bu Bruce kütüphanesi descriptor nesnesine işaretçi sakladığı için, menü
çıkışında nesne silinmez; tuşlar bırakılır ve oturum işaretçisi temizlenir.
Script, klavye, dosya tarayıcı ve mevcut CLI çıkışları aynı temizlemeyi kullanır.
Native başlangıçta layout ve key-delay uygulanır, `USB.begin()` sonucu kontrol
edilir. Host bekleyişi 15 saniye ile sınırlıdır ve Back/BOOT ile iptal edilebilir.
Beklerken ekran her tur yeniden çizilmez.

Bağlantı durumu `tud_mounted() && !tud_suspended()` üzerinden okunur;
seri monitörün CDC DTR sinyali gerekmez. Bu, hostun USB cihazını yapılandırdığı
anlamına gelir; Windows sürücü kurulumu veya fiziksel tuş teslimi doğrulaması değildir.
Kablo çıkarılmışken yerel tuş durumu temizlenebilir, USB raporu gönderilmez.
Waveshare USB Keyboard açılışındaki eski otomatik ALT basışı kaldırıldı;
Not Defteri'nin menüsünü etkinleştirip yazı odağını değiştirmemesi amaçlanır.

Kart varyantı; native HID eksikliği, yanlış USB modu, kapatılmış TinyUSB HID/CDC
ve Lite build için açık `#error` üretir. CH9329 fallback bu hedefte seçilemez.
Gerçek SDK ön işlemci çıktısında CH9329 bekleme döngüsü ve ona ait UART
nesnesi bulunmaz. Ortak `Bad_Usb_Lib` içindeki CH9329 sınıf sembolleri ELF'te
kalabilir; Waveshare BadUSB/Keyboard yolu bu sınıfı kullanmaz. Sınıfın global
constructor'ı yalnızca klavye haritasını ayarlar; UART başlatmaz.

## Korunan donanım

ST7701/RGB sürücüleri, timing/init dizisi, font boyutları, GT911/I2C,
batarya ölçüm kodu, Wi-Fi/BLE uygulamaları, flash/partition ve PSRAM ayarları
önceki kaynakla karşılaştırılır. RGB halen tek PSRAM framebuffer (614400 bayt)
ve iki dahili bounce tamponu (toplam 19200 bayt) kullanır. USB tamponları
TinyUSB tarafından yönetilir; gerçek çalışma zamanı heap ölçümü yapılmadı.

USB OTG aynı dahili PHY'yi kullandığından bu uygulama çalışırken Hardware
USB Serial/JTAG yerine TinyUSB CDC seri portu vardır. COM numarası değişebilir.
USB ROM download/BOOT yöntemi yükleme için kullanılabilir; uygulama içindeki
USB JTAG debug bu konfigürasyonun parçası değildir.

## USB kapsamı ve sınırlar

Bu sürüm USB Keyboard ve mevcut Bruce DuckyScript yolunu düzeltir.
`USB_as_HID` bayrağı normalde ek Clicker/U2F menülerini de açar. Bunların
mevcut lazy descriptor kaydı açılıştaki CDC+klavye bileşimiyle uyumlu olmadığı
için Waveshare'de bu yeni menüler açılmaz. Bu özellikler önceki hedefte de
sunulmuyordu. Fare, U2F veya MSC desteği sağlandığı iddia edilmez.

Mevcut MSC kodu USB nesnesinin destructor'ını doğrudan çağırdığı ve depolama
paylaşımı gerektirdiği için bu birleşik CDC+HID sürümünde çalıştırılmaz.
Files > Mass Storage açık bir kullanılamaz uyarısı verir; Mass Storage otomatik
başlatma seçeneği bu hedefte kaydedilmez. SD/LittleFS dosya kullanımı korunur.

## Derleme

PlatformIO Core 6.2.0 ile proje kökünde:

```
pio run -e waveshare_28b
```

Orijinal PlatformIO platformu, Arduino 3.3.9 / IDF libs 5.5.4 ve kütüphaneler
korunur. Kurulu Python 3.14 çalıştırılamadığı için bu teslimat derleme ortamı
Python 3.12 ile onarıldı; firmware platform sürümü değiştirilmedi.
Windows'taki çip takma adlı derleyici başlatıcısı başarısız olursa hedefe özel
build hook aynı kurulu unified GCC'yi açık ESP32-S3 dynconfig ile seçer.
Kurulu derleyici dosyaları değiştirilmez; işlem ve seçilen çip derleme loguna yazılır.
Yöntem: [Espressif xtensa-dynconfig](https://github.com/espressif/xtensa-dynconfig#using).
`build.py`, bootloader 0x0 + partition 0x8000 + uygulama 0x10000 birleştirir.
Tam derleme sonucu, boyutlar ve SHA-256 ayrı `native-hid-build-result.json`
dosyasında teslim edilir. Bu dosya tek başına derlemenin geçtiği iddiası değildir.
Bilgisayar tarafı kontrol: `python tests/waveshare_28b/native_hid_contract.py`.
Host C++ derleyicisi gerekirse `HOST_CXX` ortam değişkeniyle seçilir.
Kaynak ağacında önceki ekran/batarya raporları ve eski testler tarihsel olarak
korunur; bu USB sürümü için güncel test `native_hid_contract.py` dosyasıdır.

## Yükleme ve fiziksel test

1. `backup-before-native-hid` önceki teslim edilen BIN ve kaynak ZIP'ini içerir;
   SHA-256 doğrulanmıştır. Bu bir cihaz flash/NVS okuma yedeği değildir.
2. USB veri kablosuyla bağla. Gerekirse BOOT'a basılı tutup RESET'e bas/bırak,
   ardından BOOT'u bırak. Windows'ta ROM download COM portunu seç.
3. Espressif Flash Download Tool'da ESP32-S3 seç; birleşik
   `Bruce-1.16.1-ws28b-native-hid-merged.bin` dosyasını **0x0** adresine yükle.
   SPI flash 16 MB / 80 MHz; araçtaki mevcut görüntü ayarlarını koru.
   Tam flash silme gerekmez. Ancak birleşik imaj NVS aralığını FF dolgu olarak
   içerir: 0x0'dan yükleme kayıtlı ayarları sıfırlayabilir. Gerekli ayarlarını
   önceden kaydet. LittleFS bölümü bu imajın dışında kalır; tam flash silme
   seçersen dosyalar da silinir. Cihazdan NVS/dosya yedeği bu görevde alınmadı.
   Alternatif: proje kökünden `pio run -e waveshare_28b -t upload --upload-port COMx`.
4. Normal reset yap. Uygulama USB bileşimi değiştiğinden COM portunu yeniden seç.
   Windows Aygıt Yöneticisi'nde USB bileşik cihaz, HID klavye ve CDC COM arayüzlerini kontrol et.
5. Not Defteri'ni açıp metin alanına odaklan. `examples/waveshare_28b/USB_KEYBOARD_TEST.txt`
   dosyasını SD veya mevcut LittleFS dosya yükleme yöntemiyle cihaza kopyala.
   Dosya BIN içine otomatik yazılmaz. Others > BadUSB & HID > BadUSB'den seç;
   cihazın mevcut başlatma onayını ver. Yaklaşık iki saniye sonra
   `Bruce USB HID Test` ve satır sonu beklenir. CH9329 mesajı çıkmamalı.
6. USB Keyboard menüsünde normal metin gönder. Bruce klavye düzenini Windows
   klavye düzeniyle eşleştir; önce İngilizce US düzeniyle verilen ASCII testi yap.
7. BadUSB ve USB Keyboard'a art arda en az 10 kez gir/çık. Yapışan tuş,
   çökme, CLI kaybı olmamalı. Host yokken 15 saniyelik bekleme ve BOOT iptalini dene.
8. Kabloyu çıkar/yeniden tak; yeniden menüye girerek yazı testini tekrarla.
   CDC seri monitör kapalıyken de test et. Çalışan script sırasında kablo
   çıkarılması otomatik script durdurma özelliği olarak sunulmuyor.
9. Dokunmatik/geri, büyütülmüş menü yazıları, pil göstergesi, SD/LittleFS ve
   Wi-Fi tarama/bağlantısı ve BLE taramasını karşılaştır. Ekran titremesinde iyileşme iddiası yoktur;
   ekran kodu önceki sürümden korunmuştur. Bellek ölçümleri için açılıştaki PSRAM
   satırını kaydet. Windows HID durumu, Not Defteri çıktısı ve bu donanım testleri
   fiziksel kartta henüz tarafımızdan doğrulanmadı.

Mevcut Bruce ve vendored kütüphane lisansları kaynak projede korunmuştur.
