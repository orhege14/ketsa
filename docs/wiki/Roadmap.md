# Roadmap

## Sürüm Geçmişi

### v0.1.0 (2026-05-01) — İlk Sürüm
- **Lexer**: Karakter akışı → token akışı
- **Parser**: Token akışı → AST
- **Interpreter**: Tree-walking yorumlama
- **Değişkenler**: `let`, `const`
- **İlkel Tipler**: int, float, string, bool
- **Aritmetik**: `+`, `-`, `*`, `/`, `%`, `**`
- **Karşılaştırma Operatörleri**
- **print Deyimi**
- **CMake Build Sistemi**
- **Windows MSVC Desteği**

### v0.2.0 (2026-06-15) — Genişletilmiş Özellikler
- **Fonksiyon Tanımlama**: Parametreler, dönüş tipleri
- **Sınıf Sistemi**: Sınıflar, metodlar, alanlar
- **Diziler**: Index erişimi
- **Nesne Literalleri**
- **Member Access Sistemi**
- **Kontrol Akışı**: if/else, while, for, switch
- **Yerleşik Fonksiyonlar**: print, println, input, type, len, toString, range
- **Modül/Import Sistemi**
- **String Interpolasyon Desteği**
- **Yapılandırılmış Hata Sistemi**

### v0.3.0 (2026-07-21) — Tip Denetimi + Bytecode VM
- **Type Checker**: Derleme zamanı tip denetimi
- **Bytecode Compiler**: AST → Bytecode (64+ talimat)
- **Virtual Machine**: Stack tabanlı yürütme
- **Pattern Matching**: Literal, değişken, wildcard, yapı bozma
- **Profesyonel CLI**: run, build, install, doctor, package, fmt, test, new
- **System Doctor**: Derleyici, build, ortam tanılaması
- **Proje İskeleti**: `ketsa new <project>`
- **Paket Yapılandırması**: `ketsa.toml`
- **Cross-Platform Build**: `build.bat`, `build.sh`
- **Test Paketi**: `examples/comprehensive.ketsa`

### v0.4.0 — Generics ve Interfaces (Planlanan)
- Generic fonksiyonlar ve tipler (`<T>` sözdizimi)
- Interface ve Trait sistemi
- Derleme zamanı tip parametre çözümleme
- Monomorfizasyon

### v0.5.0 — Async Runtime (Planlanan)
- Green threads / coroutines
- Async/await sözdizimi
- Channel tabanlı iletişim
- Actor model desteği
- Yerleşik async I/O

### v0.6.0 — Standart Kütüphane (Planlanan)
- Tam std kütüphanesi:
  - Dosya sistemi işlemleri
  - JSON parser/generator
  - HTTP client/server
  - Tarih/saat işlemleri
  - Regex motoru
  - Kriptografi primitifleri
  - Sıkıştırma (gzip, zlib)
  - Veritabanı bağlantısı

### v0.7.0 — Paket Yöneticisi (kpm) (Planlanan)
- Paket kayıt defteri
- Bağımlılık çözümleme
- Lock dosyaları
- Sürüm yönetimi
- Paket yayınlama

### v0.8.0 — Araçlar (Planlanan)
- Kod biçimlendirici (`ketsa fmt`)
- Linter (`ketsa lint`)
- Debugger (`ketsa debug`)
- Profiler (`ketsa profile`)
- Language Server Protocol (`ketsa-language-server`)
- VS Code eklentisi

### v0.9.0 — JIT Derleme (Planlanan)
- LLVM backend entegrasyonu
- Sıcak kodlar için JIT derleme
- Native kod üretimi
- AOT derleme (standalone binary)
- Cross-compilation desteği

### v1.0.0 — Kararlı Sürüm (Planlanan)
- Tam dil spesifikasyonu
- Kararlı standart kütüphane API'si
- Paket yöneticisi v1.0
- Tam IDE desteği
- Performans benchmark'ları
- Cross-platform binary release'ler

---

## Mevcut Durum

### ✅ Tamamlanan
| Özellik | Durum |
|---|---|
| Lexer / Parser | ✅ |
| AST | ✅ |
| Type Checker | ✅ |
| Bytecode Compiler | ✅ |
| Virtual Machine | ✅ |
| JIT Engine (deneysel) | ✅ |
| Pattern Matching | ✅ |
| CLI Tools | ✅ |
| System Doctor | ✅ |
| Project Scaffolding | ✅ |
| Cross-platform Build | ✅ |
| VS Code Extension | ✅ |

### 🔄 Devam Eden
| Özellik | Durum |
|---|---|
| Bytecode Optimizer | 🔄 |
| JIT Native Code Gen | 🔄 |
| Error Recovery | 🔄 |
| Documentation | 🔄 |

### 📋 Planlanan
| Özellik | Öncelik |
|---|---|
| Generics & Interfaces | Yüksek |
| Async Runtime | Yüksek |
| Standard Library | Yüksek |
| Package Manager | Orta |
| LSP / IDE | Orta |
| LLVM Backend | Düşük |

---

## Uzun Vadeli Hedefler

### IDE Entegrasyonu
- VS Code: sözdizimi vurgulama, otomatik tamamlama, hata ayıklama
- JetBrains eklentisi
- Vim/Neovim eklentisi

### Platform Desteği
- Windows (x64, ARM64)
- Linux (x64, ARM64)
- macOS (x64, ARM64/Apple Silicon)
- WebAssembly hedefi

### Ekosistem
- Paket kayıt defteri (ketsa.dev)
- Dokümantasyon oluşturucu
- Web framework
- Oyun geliştirme framework'ü
- AI/ML framework'ü

---

## Performans Hedefleri

| Kıyaslama | Mevcut | Hedef (v1.0) |
|---|---|---|
| Fibonacci(35) | ~2.5s (interpreter) | ~50ms (JIT) |
| Döngü (10M iterasyon) | ~800ms (VM) | ~20ms (JIT) |
| JSON parse (1MB) | - | ~5ms |
| HTTP istek/s | - | ~50k |

## Topluluk Hedefleri

- GitHub Stars: 100 (v0.3) → 1K (v0.6) → 10K (v1.0)
- Katkıda Bulunanlar: 5 → 20 → 100
- Paket Sayısı: 0 → 50 → 500
- Haftalık İndirme: 100 → 1K → 50K
