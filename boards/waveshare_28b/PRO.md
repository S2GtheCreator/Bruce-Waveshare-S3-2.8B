# Bruce WS28B Pro — derleme raporu

**Sürüm: 1.16.1-ws28b-pro. Yazılımsal derleme ve host testleri tamamlandı; Pro fiziksel kartta çalıştırılmadı.**

Başlangıç sürümü kullanıcı tarafından doğrulanan `1.16.1-ws28b-trq` portudur. Orijinal Bruce 1.16.1 tabanı: `ba519c936c87b89c9c667c15b932d7faca5360d5`. Portun derlenen kaynak commit'i: `16edbe5a71b1f82f16285651b0590e037d612a2a`. Geliştirme branch'i `ws28b-pro`; geri dönüş etiketi `stable-ws28b-trq`. Orijinal Bruce menüleri/araçları ve lisans bildirimleri korundu.

| Aşama | Kaynak commit | Uygulama bayt | Birleşik bayt | Gerçek PlatformIO |
|---|---|---:|---:|---|
| stage1 | `12f8f74465a2` | 3,790,784 | 3,856,320 | SUCCESS |
| stage2 | `7de3df55c92b` | 3,793,856 | 3,859,392 | SUCCESS |
| stage3 | `0dccb73ca197` | 3,808,096 | 3,873,632 | SUCCESS |
| stage4 | `8a78795b3ec1` | 3,843,520 | 3,909,056 | SUCCESS |
| stage5 | `16edbe5a71b1` | 3,847,952 | 3,913,488 | SUCCESS |

Başlangıç sürümü de ayrıca PlatformIO ile başarıyla derlendi (`baseline-build.log`). Aşama 1–4 ara dosyalarının iç sürüm etiketi trq olarak kalır; bunları `stages/stageN/result.json` ve SHA ile ayırt edin. Final etiketi pro'dur. Aşama 4'ün ilk derlemesi son önizleme değişikliğini içermediği için bütünlük kontrolünde reddedildi; son kaynak yeniden derlendi ve ara BIN değiştirildi. Eski günlük `stage4-initial-build.log` yalnızca denetim kaydıdır.

Araç zinciri: PlatformIO Core 6.2.0, pioarduino 55.3.39, Arduino ESP32 3.3.9, ESP-IDF 5.5.4 kütüphaneleri, Xtensa GCC 14.2.0+20260121. Kesin bağımlılık sürümleri derleme günlüklerinde kayıtlıdır. Windows alias başlatıcısı hata verirse mevcut `native_toolchain.py` aynı kurulu birleşik derleyiciyi doğru ESP32-S3 dynconfig/multilib ile seçer; SDK arşivleri değiştirilmez.

Final uygulama: **3,847,952 bayt** / 4,653,056 bayt uygulama bölümü. Birleşik: **3,913,488 bayt**. SHA-256: `b52d512748974cfff768d0ccbe51997cac196628239f0f45cd7e040f4b65f8e9`.

Bootloader 0x0, partition table 0x8000, uygulama 0x10000: birleşik dosyanın bu aralıkları ayrı bileşenlerle bayt düzeyinde aynı. Bootloader ve uygulama esptool image-info kontrollerinden geçti. Son kaynak/nesne/ELF zamanları kontrol edildi. Partition değişmedi; LittleFS imajı eklenmedi. **Tek parça 0x0 yazımı NVS aralığını da kapsar; ayarları koruyan önerilen yol uygulamayı 0x10000'e yazmaktır.**

Statik dahili DRAM data+bss: 120,276 bayt; IRAM text+vectors: 108,451 bayt. Bunlar çalışma anındaki heap/stack/SDK tahsislerinin tamamı değildir. 8 MB OPI PSRAM yapılandırması korundu. RGB framebuffer 614.400 bayt PSRAM; iki bounce tamponu toplam 19.200 bayt dahili SRAM. İkinci framebuffer ve LVGL yok. `.ext_ram.dummy` bölüm boyutunu fiziksel PSRAM tüketimi olarak yorumlamayın.

Host kontrolleri: 447 gerçek klavye kaynak raporu; 22 gerçek önizleme kuru testi; 8 gerçek dosya kaydetme yordamı hata senaryosu; 11 sınırlı okuma; 20 ön tahsis sınırı/satır bütünlüğü; 11 SSID/CSV metin testi; metin alanı/UTF-8 ve pil modeli kontrolleri. 67 kaynak dosyası için başlangıç SHA karşılaştırması geçti. Uyarılar içeren eski bağımlılıklar korunmuştur; derleme hatası/ABI uyumsuzluğu yoktur. Fiziksel çalışma, titreme, ADC doğruluğu, Wi-Fi/BLE ve flash yüklemesi bu sonuçlardan çıkarılamaz.

Diğer raporlar: [ekran](DISPLAY-REPORT.md), [batarya](BATTERY-REPORT.md), [Wi-Fi](WIFI-ANALYZER-REPORT.md), [BadUSB](BADUSB-REPORT.md), [testler](REGRESSION-TESTS.md), [yükleme](FLASH-INSTRUCTIONS.md), [değişiklikler](CHANGELOG.md).

HAL constructor/başlatma ve Native USB başlatma bölgeleri de normalize satır sonlarıyla başlangıç sürümüne eşittir; kayıt: preserved-regions.json. Pro kapalı/yalnız Touch Pro kapalı SDK syntax kontrolleri geçti; bu varyantlarda ayrıca tam PlatformIO link/firmware üretilmedi.
