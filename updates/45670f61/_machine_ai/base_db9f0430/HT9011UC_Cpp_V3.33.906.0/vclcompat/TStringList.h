// ===========================================================================
//  vclcompat/TStringList.h
//  BCB6 TStringList compatibility shim (the subset actually used by V906).
//
//  Heap-allocated and accessed via -> in the source (520 new-sites). Backed by
//  a std::vector<AnsiString>. 0-BASED element access (Strings[i], Count,
//  Delete(i), Insert(i,...)) -- NOTE this is 0-based, unlike AnsiString which
//  is 1-based; matches BCB6.
//
//  Property emulation: BCB6 properties (Strings[i], Text, CommaText,
//  DelimitedText, Objects[i]) are exposed via small proxy objects so both
//  read (implicit conversion to AnsiString) and write (operator=) compile
//  the same way the original `slist->Text = x;` / `x = slist->Text;` code does.
//
//    Strings      -> StringsProxy ;  list->Strings[i]              (get/set)
//    Objects      -> ObjectsProxy ;  list->Objects[i]             (get/set)
//    Text         -> TextProxy    ;  list->Text                   (get/set)
//    CommaText    -> CommaTextProxy
//    DelimitedText-> DelimitedTextProxy
//    Count        -> int    (read-only function-call-free member via GetCount)
//    Delimiter    -> char   (plain public member)
// ===========================================================================
#ifndef VCLCOMPAT_TSTRINGLIST_H
#define VCLCOMPAT_TSTRINGLIST_H

#include "vclcompat/AnsiString.h"
#include <vector>

namespace vclcompat {

// Minimal TObject base so AddObject/Objects can carry user pointers.
class TObject {
public:
    virtual ~TObject() {}
};

// ---------------------------------------------------------------------------
//  TStrings -- abstract base class (BCB6 VCL).
//  TStringList inherits from TStrings so that functions declared as
//  TStrings* (e.g. SearchFile, SearchFileAll, SearchFolder in HTMD5.h) accept
//  a TStringList* argument without a cast.
//
//  Only the methods called through a TStrings* pointer in the V906 source are
//  listed here as pure virtuals.  TStringList provides the concrete impl.
//
// AI(W906-VCW1) 20260721: R1 -- added `: public TObject` (was base-less).
// Needed so `dynamic_cast<TStringList*>(someTObjectPtr)` is well-formed in
// SECSGEM's SetECValue/DataItemOutSV/DataItemOutEC/GetECDataValue cast-
// dispatch cascades (golden dynamic_casts a stored EC/SV "VCL pointer",
// declared TObject*, against TStringList among 6 other candidate widget
// types) -- per [expr.dynamic.cast], a dynamic_cast between pointer types is
// ill-formed at COMPILE TIME unless one of the two classes is a base of the
// other; TStrings had no relation to TObject before this change, which would
// make that cast a hard compile error, not a runtime nullptr. Purely
// additive: TObject is `{ virtual ~TObject(){} }` (no data members), and
// TStrings/TStringList were already polymorphic via their own pure virtuals,
// so this adds no new vtable pointer and changes no sizeof/layout for any of
// the 67 existing TStringList call sites (none of which cast a TStringList*
// to/from anything today -- confirmed by recon before this edit).
// ---------------------------------------------------------------------------
class TStrings : public TObject {
public:
    virtual ~TStrings() {}

    virtual void       Clear()                               = 0;
    virtual int        Add(const AnsiString& s)              = 0;
    virtual AnsiString GetString(int i)  const               = 0;
    virtual void       SetString(int i, const AnsiString& s) = 0;
    virtual int        GetCount() const                      = 0;

    // Expose Count as a virtual accessor so derived classes keep it in sync.
    // BCB6 code accesses slResult->Count; we expose it as a virtual getter
    // that is overridden by TStringList (which caches an int member for the
    // `list->Count` no-parens idiom).
    //
    // NOTE: TStringList re-declares Count as a plain int member (the dominant
    // BCB6 usage pattern is list->Count with no parens, not list->Count()).
    // When accessed through a TStrings* pointer the virtual count() is used.
    virtual int count() const { return GetCount(); }

    // Proxy-based Strings[i] cannot be surfaced through a pure virtual interface
    // without introducing a virtual accessor pair.  Code using TStrings* uses
    // GetString/SetString directly; the Strings accessor proxy is only available
    // on a TStringList*.
};

class TStringList; // fwd

// ---- index proxies --------------------------------------------------------
class StringsProxy {
public:
    StringsProxy(TStringList* o, int i) : owner_(o), idx_(i) {}
    operator AnsiString() const;                 // read
    StringsProxy& operator=(const AnsiString& v); // write
    StringsProxy& operator=(const char* v);
    // AI(W906-FW-BinSel-WA-integrate) 20260819: same proxy-to-proxy text-
    // semantics repair as CommaTextProxy below/above -- the defaulted copy-
    // assign was a silent no-op for the BCB6 property-to-property idiom.
    // (9 live `a->Strings[i]=b->Strings[j]` call sites measured 20260819.)
    StringsProxy& operator=(const StringsProxy& v) { return *this = AnsiString(v); }
    // AI(W906-FW-GEM-W8) 20260826: golden 到處寫 `List->Strings[i].c_str()`
    // （本波 uHGemEquipment.cpp:7285/7317 兩處，全樹還有更多）。BCB6 的
    // __property 讀出來就是 AnsiString，可以直接 .c_str()；本樹的 proxy 只有
    // `operator AnsiString()`，成員存取不會觸發使用者定義轉換，所以 .c_str()
    // 編不過。補這一個成員讓 golden 原文逐字成立。
    //
    // ⚠ 第一版寫成 `return AnsiString(*this).c_str();` 是錯的：那個暫存
    // AnsiString 在 return 這個完整運算式結束就析構，指標當場懸空。
    // 正確做法是存進 proxy 自己的 mutable 快取——proxy 本身就是
    // `Strings[i]` 產生的暫存物件，生命週期**正好等於呼叫端的完整運算式**，
    // 與 BCB6 的 __property 回傳暫存 AnsiString 完全同一個窗口。
    const char* c_str() const { c_str_cache_ = AnsiString(*this); return c_str_cache_.c_str(); }
private:
    TStringList* owner_;
    int idx_;
    mutable AnsiString c_str_cache_;   // 只服務上面的 c_str()
};

class ObjectsProxy {
public:
    ObjectsProxy(TStringList* o, int i) : owner_(o), idx_(i) {}
    operator TObject*() const;                    // read
    ObjectsProxy& operator=(TObject* v);          // write
private:
    TStringList* owner_;
    int idx_;
};

// ---- whole-buffer proxies (Text / CommaText / DelimitedText) --------------
class TextProxy {
public:
    explicit TextProxy(TStringList* o) : owner_(o) {}
    operator AnsiString() const;
    TextProxy& operator=(const AnsiString& v);
    TextProxy& operator=(const char* v);
    // AI(W906-FW-BinSel-WA-integrate) 20260819: same proxy-to-proxy text-
    // semantics repair as CommaTextProxy below/above -- the defaulted copy-
    // assign was a silent no-op for the BCB6 property-to-property idiom.
    TextProxy& operator=(const TextProxy& v) { return *this = AnsiString(v); }
private:
    TStringList* owner_;
};

class CommaTextProxy {
public:
    explicit CommaTextProxy(TStringList* o) : owner_(o) {}
    operator AnsiString() const;
    CommaTextProxy& operator=(const AnsiString& v);
    CommaTextProxy& operator=(const char* v);
    // AI(W906-FW-BinSel-WA-integrate) 20260819: proxy-to-proxy assignment
    // MUST mean TEXT semantics (serialize source list -> parse into target),
    // matching BCB6 property-to-property `a->CommaText = b->CommaText`.
    // Without this overload the implicitly-defaulted copy-assign wins the
    // overload resolution and copies the proxy's owner_ POINTER -- a silent
    // no-op that ate every such golden idiom in the tree (24 call sites in
    // 9 TUs, measured 20260819: PordRec production-log copies in
    // aHotPlateSubstrate/asortarm/aTester_Front/aTester_Rear/ainarm9045 and
    // TfBinSel::TransferBinTrayStrToName, where the BinSelCore gate exposed
    // it: target list stayed empty, loop ran zero times).
    CommaTextProxy& operator=(const CommaTextProxy& v) { return *this = AnsiString(v); }
private:
    TStringList* owner_;
};

class DelimitedTextProxy {
public:
    explicit DelimitedTextProxy(TStringList* o) : owner_(o) {}
    operator AnsiString() const;
    DelimitedTextProxy& operator=(const AnsiString& v);
    DelimitedTextProxy& operator=(const char* v);
    // AI(W906-FW-BinSel-WA-integrate) 20260819: same proxy-to-proxy text-
    // semantics repair as CommaTextProxy below/above -- the defaulted copy-
    // assign was a silent no-op for the BCB6 property-to-property idiom.
    DelimitedTextProxy& operator=(const DelimitedTextProxy& v) { return *this = AnsiString(v); }
private:
    TStringList* owner_;
};

// ===========================================================================
//  TStringList
// ===========================================================================
class TStringList : public TStrings {
public:
    TStringList()
        : Count(0),
          Text(this), CommaText(this), DelimitedText(this),
          Delimiter(','), QuoteChar('"') {
        bindAccessors();
    }

    // The property proxies hold a back-pointer to `this`; copying/moving a
    // TStringList would leave proxies pointing at the wrong object. BCB6 code
    // always uses TStringList* (heap) and copies content via Assign(), so we
    // forbid value copy/move to keep proxy owners correct.
    TStringList(const TStringList&) = delete;
    TStringList& operator=(const TStringList&) = delete;

    // ---- element access (0-based) -----------------------------------------
    int        Add(const AnsiString& s);                 // returns new index
    int        AddObject(const AnsiString& s, TObject* obj);
    void       Insert(int index, const AnsiString& s);   // 0-based
    void       Delete(int index);                        // 0-based
    void       Clear();
    int        IndexOf(const AnsiString& s) const;       // -1 if absent
    // AI(W906-W7-L1-Wave0) 20260801: VCL TStrings::Find -- BINARY SEARCH OVER AN
    // ALREADY-SORTED LIST, and IndexOf is NOT a drop-in for it.  Two differences
    // matter: Find returns bool (not the index) and it WRITES the out-parameter.
    // Golden asendic_Loader.cpp calls it at :1412, :1585 and :1606, in every case
    // immediately after Sort() (see :1408-1412 -- Clear / CommaText= / Sort /
    // Find), which is the precondition VCL documents.
    // Algorithm mirrors Delphi Classes.pas TStringList.Find verbatim, including
    // its two easily-missed properties: (a) Index is assigned even when the item
    // is NOT found, receiving the insertion point; (b) with the default
    // Duplicates (dupIgnore, i.e. anything other than dupAccept) the search keeps
    // narrowing left after a hit, so Index lands on the FIRST equal element.
    // Comparison uses AnsiString operator< / == so it matches Sort() exactly (see
    // Sort's own note: it orders by AnsiString operator<, i.e. byte order);
    // using a different comparator here would silently break the binary search.
    bool Find(const AnsiString& S, int& Index) const {
        bool found = false;
        int L = 0, H = static_cast<int>(items_.size()) - 1;
        while (L <= H) {
            int I = L + ((H - L) >> 1);
            if (items_[static_cast<size_t>(I)] < S) {
                L = I + 1;
            } else {
                H = I - 1;
                if (items_[static_cast<size_t>(I)] == S) { found = true; L = I; }
            }
        }
        Index = L;
        return found;
    }
    AnsiString First() const;                            // Strings[0]
    void       Sort();                                   // uses AnsiString<

    int GetCount() const { return static_cast<int>(items_.size()); }

    // direct (function) accessors -- handy and used internally by proxies
    AnsiString GetString(int i) const;
    void       SetString(int i, const AnsiString& s);
    TObject*   GetObject(int i) const;
    void       SetObject(int i, TObject* o);

    // ---- whole-buffer text -------------------------------------------------
    AnsiString GetText() const;                  // lines joined by '\n'
    void       SetText(const AnsiString& v);     // split on CR/LF
    AnsiString GetCommaText() const;
    void       SetCommaText(const AnsiString& v);
    AnsiString GetDelimitedText() const;
    void       SetDelimitedText(const AnsiString& v);

    // char* flavours seen in the source (SetText(char*)/GetText()->char*)
    void        SetTextStr(const char* p) { SetText(AnsiString(p)); }
    const char* GetTextStr();                    // valid until next mutation

    // ---- file I/O ----------------------------------------------------------
    void LoadFromFile(const AnsiString& path);
    void SaveToFile(const AnsiString& path) const;

    void Assign(const TStringList* src);

    // ---- BCB6-style properties (proxy objects) -----------------------------
    // The real source writes `list->Strings[i]`. We expose Strings as a tiny
    // indexable proxy-factory so that subscripting yields a StringsProxy.
    struct StringsAccessor {
        TStringList* owner;
        StringsProxy operator[](int i) const { return StringsProxy(owner, i); }
    };
    struct ObjectsAccessor {
        TStringList* owner;
        ObjectsProxy operator[](int i) const { return ObjectsProxy(owner, i); }
    };

    StringsAccessor    Strings;        // list->Strings[i]
    ObjectsAccessor    Objects;        // list->Objects[i]
    // Count is a value kept in sync on every mutation so `list->Count` reads
    // like a property (no parens), matching the 1030 `->Count` uses.
    int                Count;
    TextProxy          Text;
    CommaTextProxy     CommaText;
    DelimitedTextProxy DelimitedText;
    char               Delimiter;
    char               QuoteChar;

    // Late-bind the accessor owners + Count after move/copy is not supported;
    // we bind in every ctor below.
    void bindAccessors() {
        Strings.owner = this;
        Objects.owner = this;
        Count = GetCount();
    }

private:
    std::vector<AnsiString> items_;
    std::vector<TObject*>   objects_;
    std::string             textCache_;   // backing for GetTextStr()

    void syncCount() { Count = GetCount(); }
};

} // namespace vclcompat

#endif // VCLCOMPAT_TSTRINGLIST_H
