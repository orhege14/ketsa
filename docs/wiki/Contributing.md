# Contributing

## Geliştirme Ortamı

### Gereksinimler

- C++20 destekleyen derleyici (MSVC 2022+, GCC 11+, Clang 14+)
- CMake 3.20+
- Ninja (opsiyonel, hızlı derleme için)
- Git

### Kurulum

```bash
git clone https://github.com/orhege14/ketsa
cd ketsa
build.bat            # Windows
./build.sh           # Linux/macOS
```

### Doğrulama

```bash
ketsa --version
ketsa doctor
ketsa run examples/hello.ketsa
```

---

## Proje Yapısı

```
src/
├── main.cpp              # Giriş noktası
├── CLI.h/.cpp            # Komut satırı arayüzü
├── Lexer.h/.cpp          # Lexer (tokenizer)
├── Parser.h/.cpp         # Parser (AST builder)
├── AST.h                 # AST düğüm tanımları
├── TypeChecker.h/.cpp    # Tip denetleyici
├── Interpreter.h/.cpp    # Tree-walking interpreter
├── errors/
│   ├── Error.h/.cpp      # Hata sistemi
├── runtime/
│   ├── Environment.h/.cpp # Değişken kapsamı
│   └── values/           # Runtime değer türleri
├── bytecode/
│   ├── Bytecode.h        # Komut seti
│   ├── Compiler.h/.cpp   # Bytecode derleyici
│   └── VM.h/.cpp         # Sanal makine
├── jit/
│   ├── JIT.h/.cpp        # JIT motoru
│   ├── Optimizer.h/.cpp  # Bytecode optimizer
│   └── x64/
│       ├── Assembler.h/.cpp # x64 native kod
├── runtime/
│   ├── Environment.h/.cpp
│   └── values/
│       ├── Value.h/.cpp
│       ├── NumberValue.h/.cpp
│       ├── FloatValue.h/.cpp
│       ├── StringValue.h/.cpp
│       ├── BooleanValue.h/.cpp
│       ├── CharValue.h/.cpp
│       ├── NullValue.h/.cpp
│       ├── ArrayValue.h/.cpp
│       ├── ObjectValue.h/.cpp
│       ├── FunctionValue.h/.cpp
│       ├── BuiltinFunctionValue.h/.cpp
│       ├── ClassValue.h/.cpp
│       └── ModuleValue.h/.cpp

examples/                 # Örnek programlar
std/
└── core.ketsa           # Standart kütüphane
```

---

## Kod Stili

### İsimlendirme

| Öğe | Kural | Örnek |
|---|---|---|
| Sınıflar | PascalCase | `ClassDeclarationNode` |
| Fonksiyonlar | camelCase | `parseExpression()` |
| Değişkenler | camelCase | `scopeDepth` |
| Enum'lar | PascalCase | `TokenType` |
| Enum değerleri | UPPER_CASE | `TOKEN_TYPE` |
| Sabitler | UPPER_CASE | `HOT_THRESHOLD` |

### Biçimlendirme

- 4 boşluk girinti (tab değil)
- Açma parantezi aynı satırda
- İşleçlerden önce ve sonra boşluk
- Kontrol akışı parantezlerinden önce boşluk
- Tek satırlık bloklar için süslü parantez

```cpp
// İyi
if (condition) {
    doSomething();
}

// Kötü
if(condition){
    doSomething();
}
```

### Modern C++

- `std::unique_ptr` ile bellek yönetimi (new/delete yok)
- `std::optional` ile isteğe bağlı değerler
- `enum class` ile tip güvenli enumlar
- `auto` ile tip çıkarımı (okunabilirliği bozmadığı sürece)
- `override` anahtar kelimesini kullan
- `nullptr` kullan (NULL/makro değil)
- Range-based for döngülerini tercih et

---

## Derleme

```bash
# Debug (varsayılan)
./build.sh

# Release
./build.sh --release

# Temiz derleme
./build.sh --clean

# Derle ve test et
./build.sh --test

# Sadece test
build.bat --test
```

---

## Test

```bash
ketsa test                              # Tüm testleri çalıştır
ketsa run examples/comprehensive.ketsa  # Belirli test
ketsa run --check examples/test.ketsa   # Tip denetimi testi
```

### Test Dosyası Ekleme

1. `examples/` dizinine `.ketsa` dosyası ekle
2. `ketsa test` ile çalıştığından emin ol
3. Beklenen çıktıyı belge

---

## Özellik Ekleme

Her özellik şu aşamalardan geçmelidir:

1. **Lexer**: Yeni token türleri (gerekirse)
2. **Parser**: Dilbilgisi kuralları ve AST düğümleri
3. **AST.h**: Yeni düğüm türleri (gerekirse)
4. **TypeChecker**: Tip denetimi ve semantik analiz
5. **Interpreter**: Tree-walking yorumlama
6. **Bytecode Compiler**: Bytecode üretimi
7. **VM**: Bytecode yürütme
8. **Test**: `examples/` dizininde test dosyası

### Örnek: Yeni Bir Operatör Ekleme

1. `Lexer.h` → TokenType'e yeni değer ekle
2. `Lexer.cpp` → Operatörü tanı
3. `Parser.cpp` → Ayrıştırma kurallarını ekle
4. `TypeChecker.cpp` → Tip kontrolü ekle
5. `Interpreter.cpp` → Yorumlama mantığı ekle
6. `Compiler.cpp` → Bytecode üretimi ekle
7. `VM.cpp` → Bytecode yürütme ekle
8. `examples/` → Test ekle

---

## Hata Raporlama

### Hata Bildirirken

- Hatayı tetikleyen Ketsa kodunu ekle
- Beklenen davranışı açıkla
- Ortam bilgisini ver (OS, derleyici, sürüm)
- Mümkünse hata çıktısını ekle

### Hata Kodları

| Aralık | Bileşen |
|---|---|
| 1xxx | Lexer |
| 2xxx | Parser |
| 3xxx | Type Checker |
| 4xxx | Runtime |
| 5xxx | Modüller |

---

## Pull Request Süreci

1. Depoyu fork et
2. Feature branch oluştur (`git checkout -b feature/amazing-feature`)
3. Değişiklikleri yap
4. Testleri çalıştır (`ketsa test`)
5. Commit et (`git commit -m 'Add amazing feature'`)
6. Branch'i push et (`git push origin feature/amazing-feature`)
7. Pull Request aç

### PR Şablonu

```markdown
## Özet
Bu PR ne yapıyor?

## Değişiklikler
- Değişiklik 1
- Değişiklik 2

## Test
- [ ] Tüm testler geçiyor
- [ ] Yeni testler eklendi

## Kontrol Listesi
- [ ] Kod stili kurallarına uyuldu
- [ ] Bellek sızıntısı yok
- [ ] Derleme uyarısı yok
```

---

## Derleyici Mimarisi

Ketsa derleyicisi 7 aşamalı bir pipeline'dır:

```
Kaynak → Lexer → Parser → Type Checker → Bytecode Compiler → VM → Sonuç
                                                              → JIT → x64
                                                              → Interpreter (fallback)
```

Her aşama bir sonraki için gerekli veriyi üretir ve hata durumunda pipeline durur.

---

## Performans İpuçları

- Bytecode VM, tree-walking interpreter'dan ~5-10x daha hızlı
- JIT, sıcak fonksiyonlarda ~10-50x hızlanma sağlayabilir
- Constant folding ve dead code elimination otomatik
- `--jit --vm` ile JIT + VM modunda çalıştır

---

## Kaynaklar

- **Architecture**: `ARCHITECTURE.md`
- **Changelog**: `CHANGELOG.md`
- **Roadmap**: `ROADMAP.md`
- **Örnekler**: `examples/` dizini
