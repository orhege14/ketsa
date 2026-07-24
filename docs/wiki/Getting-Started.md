# Getting Started

## Kurulum

### Gereksinimler

| Gereksinim | Windows | Linux | macOS |
|---|---|---|---|
| C++20 Derleyici | MSVC 2022+ | GCC 11+ / Clang 14+ | Apple Clang 14+ |
| CMake 3.20+ | `winget install Kitware.CMake` | `apt install cmake` | `brew install cmake` |
| Ninja (opsiyonel) | `winget install Ninja` | `apt install ninja-build` | `brew install ninja` |
| Git | `winget install Git` | `apt install git` | `brew install git` |

### Kaynaktan Derleme

```bash
git clone https://github.com/orhege14/ketsa
cd ketsa

# Windows
build.bat                  # Debug
build.bat --release        # Release

# Linux / macOS
chmod +x build.sh
./build.sh                 # Debug
./build.sh --release       # Release
```

### Doğrulama

```bash
ketsa --version
ketsa doctor
```

Çıktı:
```
Ketsa Programming Language v0.5
Compiler: Type Checker + Bytecode VM
Runtime: Tree-walking Interpreter + Bytecode VM
```

---

## İlk Program

```ketsa
print "Merhaba Ketsa!"
```

Çalıştırma:
```bash
ketsa run program.ketsa
```

---

## Proje Oluşturma

```bash
ketsa new my_project
cd my_project
ketsa run src/main.ketsa
```

Bu komut şu yapıyı oluşturur:

```
my_project/
├── ketsa.toml          # Proje yapılandırması
├── src/
│   └── main.ketsa      # Ana kaynak dosyası
├── lib/                # Kütüphaneler
└── tests/
    └── test_main.ketsa # Test dosyası
```

### ketsa.toml

```toml
[package]
name = "my_project"
version = "0.1.0"
description = "A Ketsa project"
entry = "src/main.ketsa"

[dependencies]
```

---

## Çalıştırma Seçenekleri

```bash
ketsa run <dosya> [seçenekler]

Seçenekler:
  --check          Sadece sözdizimi ve tip denetimi
  --ast            AST yapısını yazdır
  --tokens         Token akışını yazdır
  --bytecode       Bytecode talimatlarını yazdır
  --vm             Bytecode VM kullan (deneysel)
  --jit            JIT derlemeyi etkinleştir
  --no-typecheck   Tip denetimini atla
```

Örnekler:
```bash
ketsa run --check program.ketsa
ketsa run --jit --vm program.ketsa
ketsa run --ast program.ketsa
ketsa run --tokens program.ketsa
ketsa run --vm --bytecode program.ketsa
```

---

## CLI Komutları

| Komut | Açıklama |
|---|---|
| `ketsa run <file>` | Program çalıştır (varsayılan) |
| `ketsa build` | Derleyiciyi derle |
| `ketsa install` | Sisteme kur |
| `ketsa doctor` | Sistem tanılaması |
| `ketsa package` | Paket yönetimi |
| `ketsa fmt <file>` | Kaynak kodunu biçimlendir |
| `ketsa test` | Testleri çalıştır |
| `ketsa new <project>` | Yeni proje oluştur |
| `ketsa help` | Yardım göster |
| `ketsa version` | Sürüm bilgisi |

---

## VS Code Eklentisi

Ketsa, resmi VS Code desteği sunar.

Konum: `editors/vscode/`

**Özellikler:**
- `.ketsa` dosya desteği
- Sözdizimi vurgulama
- Ketsa dosya ikonları
- Run Ketsa butonu
- Ketsa Output paneli

**Kurulum:**
1. VS Code'da uzantıyı yükle
2. `.ketsa` dosyası aç
3. **Run Ketsa** butonuna bas
4. Çıktıyı **Ketsa Output** panelinde gör

---

## Tanı Sistemi

```bash
ketsa doctor
```

Denetlenenler:
- Derleyici sürümü
- Build sistemi (CMake)
- C++ derleyicisi
- C++ standardı
- Standart kütüphane
- Git deposu
- Mimari
- JIT motoru
- JIT arka ucu
- Native derleyici
- Ortam PATH'i
