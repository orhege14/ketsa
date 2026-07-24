# Compiler Architecture

## Genel Bakış

Ketsa derleyicisi, çok aşamalı bir pipeline ile çalışır. Her aşama bir öncekinin çıktısını işler ve bir sonrakine aktarır.

```
Kaynak Kod (.ketsa)
    │
    ▼
┌─────────────────────────────────────┐
│ 1. LEXER (Lexer.cpp/.h)            │
│    Karakter akışı → Token akışı    │
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│ 2. PARSER (Parser.cpp/.h)          │
│    Token akışı → AST (Soyut Söz    │
│    Dizimi Ağacı)                   │
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│ 3. TYPE CHECKER (TypeChecker.cpp/h) │
│    AST → Tip Denetlenmiş AST       │
│    Statik semantik analiz          │
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│ 4. BYTECODE COMPILER               │
│    (Compiler.cpp/.h)               │
│    AST → Bytecode talimatları      │
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│ 5. VIRTUAL MACHINE (VM.cpp/.h)     │
│    Bytecode → Çalıştırma           │
│    Stack tabanlı yürütme           │
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│ 6. JIT ENGINE (JIT.cpp/.h)        │
│    Sık kullanılan kodları native   │
│    x64 makine koduna derler       │
└─────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────┐
│ 7. INTERPRETER (Interpreter.cpp/h) │
│    Tree-walking interpreter        │
│    (VM kullanılmazsa fallback)     │
└─────────────────────────────────────┘
```

---

## 1. Lexer (Lexer.cpp/.h)

Lexer, kaynak kodu karakter karakter okur ve anlamlı token'lara dönüştürür.

### Sorumluluklar

- Anahtar kelimeleri tanıma (500+ token türü)
- Sayısal literal ayrıştırma (int, float)
- String literal ayrıştırma (kaçış karakterleri dahil)
- Char literal ayrıştırma
- Operatörleri tanıma (tekli, ikili, çok karakterli)
- Yorumları atlama (`//`, `/* */`)
- Beyaz boşluk yönetimi
- Hata konumu takibi (satır/sütun)

### Token Türleri

```cpp
enum class TokenType {
    // Özel
    UNKNOWN,

    // Anahtar Kelimeler
    PRINT, LET, CONST, IF, ELSE, WHILE, FOR,
    FUNC, RETURN, CLASS, IMPORT, MATCH, SWITCH,

    // Literaller
    NUMBER, FLOAT, STRING, CHARACTER, BOOLEAN,

    // Operatörler
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
    EQUAL, EQUAL_EQUAL, NOT_EQUAL, BIGGER, SMALLER,
    AND_AND, OR_OR, NOT,

    // Yapısal
    LPAREN, RPAREN, LBRACE, RBRACE,
    LBRACKET, RBRACKET, SEMICOLON, COLON, COMMA, DOT,

    // ...
    END
};
```

### Önemli Detaylar

- `<=>` (üç yönlü karşılaştırma) özel sıralama ile doğru tokenize edilir
- `...` (spread) operatörü `..`'den önce kontrol edilir
- String'lerde kaçış dizileri (`\n`, `\t`, `\"`) işlenir

---

## 2. Parser (Parser.cpp/.h)

Parser, token akışını alır ve Abstract Syntax Tree (AST) oluşturur.

### Parse Stratejisi

- **İfadeler**: Precedence climbing (öncelik tırmanma)
- **Deyimler**: Recursive descent (özyinelemeli iniş)
- **Hata Kurtarma**: `synchronize()` ile akıllı atlama

### Öncelik Seviyeleri

| Seviye | Operatörler |
|---|---|
| 1 (En düşük) | `=` atama |
| 2 | `\|\|` |
| 3 | `&&` |
| 4 | `==` `!=` |
| 5 | `<` `>` `<=` `>=` `<=>` |
| 6 | `+` `-` |
| 7 | `*` `/` `%` `**` |
| 8 (En yüksek) | Unary `!` `-` `++` `--` |

### AST Düğüm Türleri

- **Literal**: Number, Float, String, Boolean, Char, Null
- **Değişken**: VariableDeclaration, VariableAccess, Assignment
- **Operatör**: BinaryExpression, UnaryExpression
- **Kontrol**: If, While, For, Break, Continue, Switch
- **Fonksiyon**: FunctionDeclaration, FunctionCall, Return
- **Sınıf**: ClassDeclaration, NewExpression, MemberAccess
- **Dizi**: ArrayLiteral, ArrayAccess, IndexAssignment
- **Modül**: Import, FromImport
- **Pattern**: Match, MatchCase, MatchPattern

---

## 3. Type Checker (TypeChecker.cpp/.h)

Type Checker, AST üzerinde statik semantik analiz yapar.

### Görevleri

- **Tip çıkarımı**: Bildirilmeyen tipleri değerden çıkarır
- **Tip uyumluluğu**: Atama ve işlemlerde tip kontrolü
- **Değişken kapsamı**: Tanımlanmamış değişkenleri tespit
- **Fonksiyon imzaları**: Argüman sayısı ve tip kontrolü
- **Sabit kontrolü**: `const` değişkenlere atama denetimi
- **Döngü kontrolü**: `break`/`continue` döngü dışında kullanımı
- **Dönüş yolu**: void olmayan fonksiyonlarda return kontrolü

### Kapsam Yönetimi

```
Global Scope
  ├── Yerleşik fonksiyonlar (print, len, type...)
  ├── Kullanıcı tanımlı global değişkenler
  │
  └── Block Scope
      ├── Fonksiyon parametreleri
      └── Yerel değişkenler
          └── İç içe block'lar
```

### Tip Uyumluluk Kuralları

| Kaynak | Hedef | Geçerli mi? |
|---|---|---|
| int | float | Evet (otomatik) |
| float | int | Evet (uyarı) |
| any | herhangi | Evet |
| null | object | Evet |
| null | int/float | Hayır |

---

## 4. Bytecode Compiler (bytecode/Compiler.cpp/.h)

Bytecode compiler, tip denetlenmiş AST'yi stack tabanlı bytecode talimatlarına dönüştürür.

### Komut Seti (64+ talimat)

#### Sabitler
| OpCode | Açıklama |
|---|---|
| `LOAD_CONST` | Sabit havuzundan değer yükle |
| `LOAD_NULL` | null yükle |
| `LOAD_TRUE` / `LOAD_FALSE` | Boolean yükle |

#### Değişkenler
| OpCode | Açıklama |
|---|---|
| `LOAD_VAR` / `STORE_VAR` | Yerel değişken |
| `LOAD_GLOBAL` / `STORE_GLOBAL` | Global değişken |
| `LOAD_UPVALUE` / `STORE_UPVALUE` | Closure değişkeni |

#### Aritmetik
| OpCode | Açıklama |
|---|---|
| `ADD`, `SUB` | Toplama, çıkarma |
| `MUL`, `DIV` | Çarpma, bölme |
| `MOD`, `POW` | Mod, üs |
| `NEGATE` | Tekli eksi |

#### Kontrol
| OpCode | Açıklama |
|---|---|
| `JUMP` | Koşulsuz atlama |
| `JUMP_IF_FALSE` / `JUMP_IF_TRUE` | Koşullu atlama |
| `LOOP` | Geri atlama (döngü) |

#### Fonksiyonlar
| OpCode | Açıklama |
|---|---|
| `CALL` | Fonksiyon çağırma |
| `RETURN` | Geri dönüş |
| `MAKE_FUNCTION` | Fonksiyon oluşturma |
| `MAKE_CLOSURE` | Closure oluşturma |

#### Nesneler
| OpCode | Açıklama |
|---|---|
| `MAKE_OBJECT` | Nesne oluşturma |
| `LOAD_FIELD` / `STORE_FIELD` | Alan erişimi |
| `MAKE_CLASS` | Sınıf oluşturma |

#### Diziler
| OpCode | Açıklama |
|---|---|
| `MAKE_ARRAY` | Dizi oluşturma |
| `LOAD_INDEX` | İndex ile okuma |
| `STORE_INDEX` | İndex ile yazma |
| `ARRAY_LEN` | Dizi uzunluğu |

### Derleme Süreci

```cpp
std::unique_ptr<FunctionProto> Compiler::compile(AST* ast) {
    // 1. Ana fonksiyon prototipini oluştur
    auto mainFunc = createFunction("__main__", 0);
    pushFunction(mainFunc);

    // 2. Her AST düğümünü bytecode'a derle
    for (auto& node : ast) {
        compileStatement(node.get());
    }

    // 3. Implicit return ekle
    emitByte(OpCode::LOAD_NULL);
    emitByte(OpCode::RETURN);

    return mainFunc;
}
```

### Optimizasyon Pass'leri

```cpp
// 1. Constant Folding: 2 + 3 → 5
LOAD_CONST 2, LOAD_CONST 3, ADD → LOAD_CONST 5

// 2. Dead Code Elimination:
STORE_VAR x, POP → STORE_VAR x

// 3. Jump Optimization:
JUMP → (bir sonraki talimat) → sil

// 4. Peephole:
NOT + JUMP_IF_TRUE → JUMP_IF_FALSE
LOAD_CONST 0 + ADD → sil
LOAD_CONST 1 + MUL → sil
```

---

## 5. Virtual Machine (bytecode/VM.cpp/.h)

Stack tabanlı sanal makine, bytecode talimatlarını yorumlar.

### Mimari

```
┌─────────────────────────────┐
│         Stack               │
│  ┌───────────────────────┐  │
│  │ [değer] [değer] ...   │  │
│  └───────────────────────┘  │
│                             │
│  Call Frame Stack           │
│  ┌───────────────────────┐  │
│  │ Frame 0 (main)        │  │
│  │ Frame 1 (func)        │  │
│  │ ...                   │  │
│  └───────────────────────┘  │
│                             │
│  Global Environment         │
│  ┌───────────────────────┐  │
│  │ isim → Value           │  │
│  │ x    → Value           │  │
│  └───────────────────────┘  │
└─────────────────────────────┘
```

### Çalışma Döngüsü

```cpp
InterpretResult VM::interpret(FunctionProto* mainFunc) {
    // Ana çalışma döngüsü
    while (frameCount > 0) {
        CallFrame* cf = &frames[frameCount - 1];
        Instruction instr = *cf->ip;

        switch (instr.op) {
            case OpCode::LOAD_CONST: /* ... */ break;
            case OpCode::ADD:        /* ... */ break;
            case OpCode::CALL:       /* ... */ break;
            case OpCode::RETURN:     /* ... */ break;
            // ...
        }
    }
}
```

### Call Frame Yapısı

```cpp
struct CallFrame {
    FunctionProto* function;     // Çalıştırılan fonksiyon
    const Instruction* ip;       // Talimat pointer'ı
    Environment* closure;        // Closure ortamı
    int returnSlot;              // Dönüş slot'u
};
```

---

## 6. JIT Engine (jit/JIT.cpp/.h)

JIT motoru, sık kullanılan fonksiyonları native x64 makine koduna derler.

### Ne Zaman JIT Devreye Girer?

```cpp
constexpr int HOT_FUNCTION_THRESHOLD = 50;  // 50 çağrı → JIT
constexpr int HOT_LOOP_THRESHOLD = 100;      // 100 iterasyon → optimize
```

### JIT Pipeline

```
1. Profiling
   - Fonksiyon çağrı sayılarını izle
   - Sıcak fonksiyonları belirle

2. Bytecode Optimizasyon
   - Sabit katlama
   - Ölü kod temizleme
   - Jump optimizasyonu

3. Native Kod Üretimi
   - x64 assembly (X64Assembler)
   - Stack frame yönetimi
   - Register tahsisi

4. Kod Yürütme
   - Execute edilebilir sayfaya kopyala
   - Native fonksiyon olarak çağır
```

### x64 Assembler

```cpp
// x64 kod üretimi örneği
void X64Assembler::emitProlog(int frameSize) {
    emit8(0x55);                    // push rbp
    emit8(0x48); emit8(0x89); emit8(0xE5);  // mov rbp, rsp
    emit8(0x48); emit8(0x83); emit8(0xEC); emit8(alignedSize); // sub rsp, N
    // Non-volatile register'ları kaydet
}
```

Desteklenen talimatlar:
- `mov` (register/stack/immediate)
- `add`, `sub`, `imul`, `idiv`
- `cmp`, `setCC`
- `jmp`, `jcc` (koşullu dallanma)
- `call`, `ret`
- `push`, `pop`
- `cdq` (sign extension)

---

## 7. Interpreter (Interpreter.cpp/.h)

Tree-walking interpreter, AST'yi doğrudan yürütür. VM kullanılmadığında fallback olarak çalışır.

### Yürütme Modeli

```cpp
void Interpreter::executeStatement(ASTNode* node) {
    switch (node->type) {
        case NodeType::VARIABLE_DECLARATION:
            // Değişken tanımla
        case NodeType::ASSIGNMENT:
            // Atama yap
        case NodeType::IF:
            // Koşul kontrol et
        case NodeType::WHILE:
            // Döngü
        case NodeType::FUNCTION_CALL:
            // Fonksiyon çağır
        // ...
    }
}
```

### Environment (Kapsam) Yönetimi

```cpp
class Environment {
    std::unordered_map<std::string, VariableInfo> variables;
    std::shared_ptr<Environment> parent;  // Üst kapsam

    void define(name, value, isConst, type);  // Yeni değişken
    void set(name, value);                    // Değişken atama
    Value* get(name);                         // Değişken okuma
    Environment createChild();                 // Alt kapsam
};
```

### Built-in Fonksiyonlar

Interpreter başlangıçta şu yerleşik fonksiyonları kaydeder:

- `print`, `println` - Çıktı
- `input` - Kullanıcı girdisi
- `type` - Tip sorgulama
- `len` - Uzunluk
- `toInt`, `toFloat`, `toString` - Tip dönüşümü
- `range` - Aralık oluşturma

---

## Hata Sistemi (errors/Error.cpp/.h)

### Hata Seviyeleri

| Seviye | Açıklama |
|---|---|
| `WARNING` | Devam edilebilir, potansiyel sorun |
| `ERROR` | Derleme durur |
| `FATAL` | Sistem hatası |

### Hata Kodları

```
KETSA-E3001: Tip uyuşmazlığı
KETSA-W0103: Implicit dönüşüm
KETSA-E4001: Sıfıra bölme
```

### Hata Çıktısı Formatı

```
error[KETSA-E3001]: Cannot assign type 'string' to variable 'x' of type 'int'
  --> program.ketsa:5:3
   |
 5 | let x: int = "hello"
   |   ^^^^^^^^^^^^^^^^^
   = note: Type mismatch in variable declaration
```

---

## Runtime Değer Sistemi (runtime/values/)

Her runtime değeri `Value` soyut sınıfından türetilir:

```
Value (abstract)
├── NumberValue       (int64)
├── FloatValue        (double)
├── StringValue       (string)
├── BooleanValue      (bool)
├── CharValue         (char)
├── NullValue         (null)
├── ArrayValue        (vector<Value>)
├── ObjectValue       (map<string, Value>)
├── FunctionValue     (user-defined)
├── BuiltinFunctionValue (native C++)
├── ClassValue        (class blueprint)
└── ModuleValue       (module scope)
```

### Value Arayüzü

```cpp
class Value {
    virtual ValueType getType() const = 0;
    virtual std::string toString() const = 0;
    virtual std::unique_ptr<Value> clone() const = 0;
    virtual bool isTruthy() const;
    virtual TypeInfo getTypeInfo() const;
};
```

---

## Proje Dizini Yapısı

```
src/
├── main.cpp                 # Giriş noktası
├── CLI.h/.cpp              # Komut satırı arayüzü
├── Lexer.h/.cpp            # Lexer (tokenizer)
├── Parser.h/.cpp           # Parser (AST builder)
├── AST.h                   # AST düğüm tanımları
├── TypeChecker.h/.cpp      # Tip denetleyici
├── Interpreter.h/.cpp      # Tree-walking interpreter
├── errors/
│   ├── Error.h/.cpp        # Hata sistemi
├── runtime/
│   ├── Environment.h/.cpp  # Değişken kapsamı
│   └── values/             # Runtime değer türleri
├── bytecode/
│   ├── Bytecode.h          # Komut seti
│   ├── Compiler.h/.cpp     # Bytecode derleyici
│   └── VM.h/.cpp           # Sanal makine
├── jit/
│   ├── JIT.h/.cpp          # JIT motoru
│   ├── Optimizer.h/.cpp    # Bytecode optimizer
│   └── x64/
│       ├── Assembler.h/.cpp # x64 native kod üretici

std/
└── core.ketsa              # Standart kütüphane
```
