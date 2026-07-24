# Ketsa Programming Language

> Python'un sadeliği, C++'ın performansı, Rust'tan ilham alan güvenlik.

Ketsa; modern, genel amaçlı bir programlama dilidir. Zarif sözdizimini yüksek performans, güçlü tip denetimi ve modern araçlarla birleştirir.

---

## Hızlı Bakış

```ketsa
// Fibonacci
func fibonacci(n: int): int {
    if (n <= 1) return n
    return fibonacci(n - 1) + fibonacci(n - 2)
}

print fibonacci(10)  // 55

// Pattern Matching
match (deger) {
    case 1:
        print "bir"
    case 2:
        print "iki"
    else:
        print "diger"
}
```

---

## Temel Özellikler

| Özellik | Açıklama |
|---|---|
| **Modern Sözdizimi** | Python ve C benzeri dillerden ilham alır |
| **Statik Tip Denetimi** | Derleme zamanında tip hatalarını yakalar |
| **Bytecode VM** | Stack tabanlı sanal makine ile yüksek performans |
| **JIT Derleme** | x64 native kod üretimi (deneysel) |
| **Pattern Matching** | Literal, değişken, wildcard ve yapı bozma desenleri |
| **Modüler Yapı** | Modüller, import/export sistemi |
| **Cross-Platform** | Windows, Linux, macOS |

---

## Hızlı Başlangıç

```bash
# Derleme
./build.sh          # Linux/macOS
build.bat           # Windows

# Çalıştırma
ketsa run examples/hello.ketsa

# Proje oluşturma
ketsa new myapp
cd myapp
ketsa run src/main.ketsa
```

---

## Dil Karşılaştırması

| Ketsa | Python | C++ | Rust |
|---|---|---|---|
| Statik tipli | Dinamik tipli | Statik tipli | Statik tipli |
| Bytecode VM | Yorumlanan | Native derlenen | Native derlenen |
| Çöp toplamalı | Çöp toplamalı | Manual yönetim | Ownership |
| Pattern matching | Match (3.10+) | - | Match |
| Sınıflar | Sınıflar | Sınıflar | Struct + impl |

---

## Sürüm Geçmişi

| Sürüm | Tarih | Öne Çıkanlar |
|---|---|---|
| 0.5.0 | 2026-07 | JIT motoru, native kod, optimizer |
| 0.3.0 | 2026-07 | Tip denetleyici, bytecode VM, pattern matching |
| 0.2.0 | 2026-06 | Fonksiyonlar, sınıflar, diziler |
| 0.1.0 | 2026-05 | Lexer, parser, interpreter, değişkenler |

---

## Hata Yakalama ve Güvenlik

Ketsa, hataları üç seviyede bildirir:

- **Warning (Uyarı)**: Potansiyel sorunlar, çalışmayı engellemez
- **Error (Hata)**: Derlemeyi durdurur
- **Fatal (Ölümcül)**: Sistem hatası

Hata kodları modülerdir:
- `1xxx`: Lexer hataları
- `2xxx`: Parser hataları
- `3xxx`: Tip hataları
- `4xxx`: Runtime hataları
- `5xxx`: Modül hataları

---

## Ekosistem

- **VS Code Eklentisi**: Sözdizimi vurgulama, çalıştırma butonu
- **CLI Araçları**: run, build, test, doctor, fmt, package, new
- **Proje Şablonu**: `ketsa new` ile hızlı başlangıç
- **Tanı Sistemi**: `ketsa doctor` ile sistem kontrolü

---

```bash
ketsa doctor

  ╔══════════════════════════════════════╗
  ║       Ketsa System Doctor            ║
  ╚══════════════════════════════════════╝

  [PASS] Ketsa Compiler
  [PASS] Build System
  [PASS] C++ Compiler
  [PASS] JIT Engine
```
