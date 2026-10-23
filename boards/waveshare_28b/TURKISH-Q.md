# Bruce Waveshare 2.8B — Türkçe Q

Sürüm: **1.16.1-ws28b-trq**. Temel: çalışan Native USB HID portu
`1.16.1-ws28b-native-hid`; resmî Bruce 1.16.1 commit'i
`ba519c936c87b89c9c667c15b932d7faca5360d5`.
Kullanıcı, önceki portun Windows ENG US seçiliyken BadUSB metinlerini doğru
yazdığını bildirdi. Bu yeni Türkçe Q sürümü fiziksel kartta tarafımızdan denenmedi.

## Kullanım ve kalıcı ayar

Menü: **Others > BadUSB & HID > BadUSB > Settings > Keyboard Layout**.
**Turkish Q** (13) ve **English US** (0) seçenekleri bulunur; önceki diğer
düzen seçenekleri korunur. **Run Script** mevcut Bruce dosya seçicisini açar.
Windows ve Bruce'ta seçilen düzenler eşleşmelidir.

Waveshare varsayılanı Turkish Q'dur. Önceki config dosyasının ilk yüklenişinde
`badUSBLayoutRevision` yoksa düzen bir kez 13 yapılır; yeni config'e revizyon 1
yazılır. Bundan sonra kaydedilen English US tercihi açılışta tekrar Türkçe'ye
çevrilmez. Seçim mevcut Bruce `setBadUSBBLEKeyboardLayout` / `saveFile()` yoluyla
LittleFS içindeki `/bruce.conf` dosyasına, mevcut düzenekle SD kopyasına kaydedilir.
Bu ayar, Bruce'un mevcut USB/BLE ortak klavye ayarıdır. Kalıcılığın gerçek
güç kesip açma testi henüz yapılmadı.

## Eşleme ve desteklenen metin

Eski Türkçe ASCII tablosundaki `i`, `q`, `y`, `:`, `-`, `@` ve diğer hatalı
eşlemeler düzeltildi. Referans, Windows `kbdtuq.dll` Türkçe Q (KLID 0000041F)
verisidir. `MapVirtualKeyExW` ve `ToUnicodeEx(flags=4)` ile Caps kapalı/açık
durumları okunmuştur; bilgisayara tuş gönderilmemiş ve masaüstünün aktif düzeni
değiştirilmemiştir. DLL SHA-256 ve elde edilen referans JSON kaynakta bulunur.
Microsoft şeması: [Turkish Q](https://learn.microsoft.com/en-us/globalization/keyboards/kbdtuq).

Ortak `HidText.h` metni çalışma sırasında UTF-8'den çözer. Dosyalar değiştirilmez.
USB ve BLE `write(buffer,size)` aynı codec'i kullanır; `STRING`, `STRINGLN`,
normal USB Keyboard metin yazımı ve mevcut JavaScript `badusbPrint/Println`
çağrıları bu yoldan geçer. `STRINGLN` metin başarılıysa mevcut CRLF/Enter yolunu
kullanır. Bir hata olursa yeni satır veya sonraki komut çalıştırılmaz.

| Karakter | Türkçe Q HID usage | Değiştirici |
|---|---:|---|
| `:` | 0x38 | Shift |
| `/` | 0x24 | Shift |
| `?` | 0x2D | Shift |
| `=` | 0x27 | Shift |
| `-` / `_` | 0x2E | Yok / Shift |
| `@` | 0x14 | Sağ Alt (AltGr) |
| `#` / `$` | 0x20 / 0x21 | AltGr |
| `[` / `]` | 0x25 / 0x26 | AltGr |
| `{` / `}` | 0x24 / 0x27 | AltGr |
| `i` / `I` | 0x34 / 0x0C | Yok / Shift |
| `ı` / `İ` | 0x0C / 0x34 | Yok / Shift |

Destek: yazdırılabilir 95 ASCII karakter, `çğıöşüÇĞİÖŞÜ` ve doğrudan TR Q
tuşlarıyla çıkan 10 ek Unicode (`£¨´½Æßæé€₺`). Tab/Backspace/LF desteklenir;
CR, Bruce'un önceki CRLF davranışına uygun atlanır. Türkçe Q ölü tuşları
`^`, grave/backtick, `~`, `¨`, `´` sonrasında boşluk darbesiyle tamamlanır;
örneğin `^a` istemeden `â` olmaz. Diğer Unicode ve birleşen/decomposed
karakter dizileri desteklenmiş kabul edilmez; açık hata oluşturur.

USB/BLE host LED raporundan Caps Lock okunur ve metin eşlemesine uygulanır.
Shift/AltGr her karakterin raporundadır; ardından önceki tuş/modifier durumu
geri gönderilir. Modifiersiz metin sonunda Shift/AltGr basılı kalmaz.
`GUI`, `CTRL`, `ALT`, `SHIFT`, `ENTER`, fonksiyon tuşları ve raw key API'si
mevcut komut yolunu korur. TR Q modunda CTRL/ALT/GUI + harf kısayollarında
fiziksel US harf konumları korunur; örneğin CTRL+i yanlışlıkla farklı OEM
tuşuna dönüşmez. Klavye **Queue character** tek ASCII tuşuyla sınırlıdır;
Unicode için **Type text** kullanılır, UTF-8'in ilk baytı fonksiyon tuşuna çevrilmez.

CTRL/ALT/GUI modifier tuşu harften önce bırakılsa da, basılan fiziksel `i` usage değeri harf bırakılana kadar saklanır. Bu sıra için ek gerçek-sürücü host testi geçti.

## Hata davranışı ve sınırlar

Her Print tamponu tamamen doğrulanır; hatalı UTF-8 veya desteklenmeyen kod
noktası varsa o tampondan hiçbir prefix gönderilmez. Önceki başarılı satırların
işlemleri geri alınamaz. DuckyScript çalıştırıcısı ekranda `Unsupported U+....`
veya UTF-8 hatasını gösterip durur, tuşları bırakır. CLI başarısızlık döndürür;
JavaScript metin çağrısı TypeError üretir. Backend ayrıca CDC günlüğüne hata
yazar ve `textError()` sağlar. Açık key-code API'si ile UTF-8 metin API'si ayrı
semantik taşır; UTF-8 daima metin/Print tamponuyla gönderilmelidir.

Bu kaynak ağacında WifiExe entegrasyonu bulunmuyor. Dolayısıyla ayrı bir
WifiExe çalıştırıcısı testi yapılmadı. Mevcut `key_input` veya HID Print/buffer
yolunu çağıran entegrasyonlar aynı metin codec'inden yararlanır. Orijinal
`ALTCHAR/ALTSTRING` sayısal Alt-code komutlarının özel semantiği korunur;
normal UTF-8 metin için `STRING/STRINGLN` kullanılır.

Native USB (`USB_as_HID=1`, `USB_MODE=0`, CDC açık), kalıcı USB descriptor
nesnesi, ST7701/RGB timing ve framebuffer, GT911/I2C, büyütülmüş fontlar,
batarya, Wi-Fi uygulamaları, flash/partition ve OPI PSRAM ayarları korunmuştur.
BLE taşıma/bağlantı altyapısı aynı kalır; klavye metin gönderimi ortak codec'e
bağlanmıştır. Önceki Native HID sürümündeki MSC/Clicker/U2F sınırları geçerlidir.

## Bilgisayar tarafı kontroller

`tests/waveshare_28b/turkish_q_contract.py`, gerçek `USBHIDKeyboard.cpp` ve
layout kaynaklarını, yalnızca taşıma/Arduino platform çağrıları taklit edilerek
host C++ derleyicisinde çalıştırır. **447 rapor vakası** dondurulmuş Windows
layout verisiyle karşılaştırılır: TR Q/US, Caps açık/kapalı, ASCII, Türkçe,
diğer desteklenen Unicode, URL/PowerShell metni, ölü tuş+boşluk, CRLF/Tab/BS,
hatalı UTF-8 ve desteklenmeyen karakterler. Ayrıca GUI+r, CTRL/ALT/GUI+i,
ALT+F4, SHIFT+ENTER, ham ENTER, modifier korunması ve dolu 6-tuş raporu denenir.
Bu testler USB paketi göndermez; Windows'a gerçekten yazı yazma testi değildir.

`turkish_q_preservation.py`, tanımlı metin/ayar değişiklikleri dışındaki kaynakları
ve eski script örneklerini karşılaştırır; menü/kalıcılık bağlantısını statik
kontrol eder. Önceki sürümlere ait test ve raporlar tarihsel olarak kaynakta
kalabilir; bu sürümün güncel testleri bu iki dosyadır.

```
python tests/waveshare_28b/turkish_q_contract.py
python tests/waveshare_28b/turkish_q_preservation.py
pio run -e waveshare_28b
```

Host test C++ derleyicisi `HOST_CXX` ile seçilebilir. Derleme sonucu, gerçek
boyutlar, SHA-256 ve doğrulama kayıtları `turkish-q-build-result.json` içinde
teslim edilir. Bu belge tek başına derlemenin geçtiği iddiası değildir.

## Yükleme ve fiziksel Windows testi

Önceki BIN ve kaynak ZIP'i `outputs/backup-before-turkish-q` altında SHA-256
ile korunmuştur. Bu, cihazın flash/NVS okuma yedeği değildir.

1. Gerekirse BOOT basılıyken RESET'e bas/bırak, ardından BOOT'u bırak.
   Espressif Flash Download Tool'da ESP32-S3 seç ve birleşik
   **Bruce-1.16.1-ws28b-trq-merged.bin** dosyasını **0x0** adresine yükle.
   Flash 16 MB / 80 MHz. Alternatif: `pio run -e waveshare_28b -t upload --upload-port COMx`.
2. 0x0 birleşik imaj NVS alanını FF dolgu olarak kapsar; NVS ayarları ve pil
   ADC kazanç kalibrasyonu sıfırlanabilir. Tam flash silme yapma: LittleFS,
   config ve script dosyaları bu BIN'in dışında kalır. Tam silme seçersen
   bu dosyalar da silinir. Gerekli cihaz verilerini önceden ayrıca yedekle.
3. Normal reset sonrası CDC COM portunu yeniden seç. Windows düzenini
   **Türkçe Q**, Bruce düzenini **Turkish Q** yap. Not Defteri'ne odaklan.
4. `examples/waveshare_28b/TRQ_NOTEPAD_TEST.txt` dosyasını SD/LittleFS'e kopyala
   ve BadUSB > Run Script ile çalıştır. Beklenen metin aynı adlı `.expected.txt`
   dosyasındadır. Yeni test dosyaları BIN'e otomatik yüklenmez.
5. Testi Caps Lock kapalı ve açıkken tekrarla. Özellikle `i I ı İ`, Shift/AltGr
   sembolleri ve `^`/grave/`~` sonrasındaki harfleri karşılaştır. Caps raporu
   aktarımı, karakter teslimi ve tuşların bırakılması fiziksel doğrulama bekliyor.
6. `TRQ_RUN_URL_DRY_TEST.txt`, GUI+r sonrası yalnızca example.com URL'sini
   yazar; Enter göndermez. Metni kontrol edip Esc ile kapat. PowerShell için
   `TRQ_POWERSHELL_TEXT_TEST.txt` açık bir PowerShell penceresine yalnızca
   zararsız `Write-Output` komut metnini yazar; Enter manuel verilir.
7. `TRQ_UNSUPPORTED_TEST.txt` ilk satırda emoji içerir. O satırdan hiçbir
   metin yazılmamalı, ekranda U+1F642 hatası görülmeli ve sonraki satır çalışmamalı.
8. Bruce'u **English US**, Windows'u **ENG US** yap; `US_RETURN_TEST.txt`
   ile ASCII metnini doğrula. Gücü kes/aç ve English US seçiminin korunduğunu
   kontrol et; sonra Turkish Q'ya dönüp tekrar açılış kontrolü yap.
9. USB Keyboard > Type text, mevcut CLI/JS metin çağrıları ve BLE klavye için
   ayrı smoke test yap. USB CDC, menüler, dokunmatik/geri, ekran davranışı,
   batarya göstergesi ve Wi-Fi tarama/bağlantısını önceki sürümle karşılaştır.

**Fiziksel Windows Türkçe Q, GUI Çalıştır/PowerShell, BLE ve yeniden açılış
testleri tarafımızdan yapılmadı. Derleme/rapor eşleşmesi bunların geçtiği
anlamına gelmez.** Kaynak lisansları korunmuştur.
